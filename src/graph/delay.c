#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include "mathext.h"
#include "linxaudio.h"

struct linx_pin delay_pins[] = {
	{ "In", linx_pin_type_in_buffer_float },
	{ "Out", linx_pin_type_out_buffer_float },
	{ "DelayMs", linx_pin_type_in_scalar_float, 0.001f, 2000.0f, 750.0f, 1.0f },
	{ "Feedback", linx_pin_type_in_scalar_float, 0.0f, 2.0f, 0.6f, 0.01f },
	{ "WetGain", linx_pin_type_in_scalar_float, 0.0f, 2.0f, 0.7f, 0.01f },
	{ "DryGain", linx_pin_type_in_scalar_float, 0.0f, 2.0f, 1.0f, 0.01f },
};

struct linx_pin delay_subgraph_pins[] = {
	{ "SubIn", linx_pin_type_in_buffer_float },
	{ "SubOut", linx_pin_type_out_buffer_float },
};

#define MAX_DELAY_LENGTH 192000 // in samples

float dbtoamp(float db, float limit) {
	if (db <= limit)
		return 0.0f;
	return powf(10.0f, db / 20.0f);
}

struct ringbuffer_t {
	float buffer[MAX_DELAY_LENGTH]; // ringbuffer
	float *eob; // end of buffer
	float *pos; // buffer position
};

struct delay {
	struct ringbuffer_t rb;
	float delay_ms;
	float wet;
	float dry;
	float fb;
	float samplerate;
};

void rb_init(struct ringbuffer_t *rb) {
	memset(rb->buffer, 0, sizeof(float) * MAX_DELAY_LENGTH);
	rb->eob = rb->buffer + 1;
	rb->pos = rb->buffer;
}

void rb_setup(struct ringbuffer_t *rb, int size) {
	rb->eob = rb->buffer + size;
	/*while (rb->pos >= rb->eob)
		rb->pos -= size;*/
	if (rb->pos >= rb->eob)
		rb->pos = rb->buffer;
}

void dsp_mix(float* dest, float* src, int sample_count);
void dsp_mix_amp(float* dest, float* src, int sample_count, float amp);
void dsp_copy_amp(float* dest, float* src, int sample_count, float amp);

void delay_process_subgraph(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_value_array* out_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, int sample_offset, int sample_count) {
	int i;
	struct delay* data = (struct delay*)self->data;
	struct linx_value subgraph_in_value_data[max_value_queue_per_vertex];
	struct linx_value subgraph_out_value_data[max_value_queue_per_vertex];
	struct linx_value_array subgraph_in_values = { subgraph_in_value_data, 0, max_value_queue_per_vertex };
	struct linx_value_array subgraph_out_values = { subgraph_out_value_data, 0, max_value_queue_per_vertex };

	struct linx_buffer* in_buffer = &pin_buffers[0];
	struct linx_buffer* out_buffer = &pin_buffers[1];
	struct linx_buffer* subgraph_in_buffer = &self->subgraph_pin_buffers[0];
	struct linx_buffer* subgraph_out_buffer = &self->subgraph_pin_buffers[1];

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_propagated && v->timestamp >= sample_offset && v->timestamp < (sample_offset + sample_count)) {
			linx_value_array_push_value(&subgraph_in_values, v->pin_index, v->pin_group, v, v->timestamp - sample_offset);
		}
	}

	for (i = 0; i < self->subgraph->graph->propagated_pin_count; i++) {
		if (self->subgraph->propagated_pins[i].pin_type == linx_pin_type_in_buffer_float) {
			struct linx_buffer* buffer = linx_graph_instance_get_propagated_pin_buffer(self->subgraph, i);
			linx_buffer_copy_chunk(buffer, &propagated_pin_buffers[i], 0, sample_offset, sample_count);
		}
	}

	// copy data.rb into the start of subgraph_out_buffer

	linx_buffer_write(subgraph_out_buffer, data->rb.pos, sample_count);

	linx_vertex_instance_process_subgraph(self->subgraph, self, &subgraph_in_values, &subgraph_out_values, sample_count);

	if (in_buffer->write_count > 0) {
		dsp_copy_amp(data->rb.pos, &in_buffer->float_buffer[sample_offset], sample_count, data->wet);
	} else {
		memset(data->rb.pos, 0, sample_count * sizeof(float));
	}

	if (subgraph_in_buffer->write_count > 0) {
		dsp_mix_amp(data->rb.pos, subgraph_in_buffer->float_buffer, sample_count, data->fb);
	}

	if (in_buffer->write_count > 0) {
		dsp_copy_amp(&out_buffer->float_buffer[sample_offset], &in_buffer->float_buffer[sample_offset], sample_count, data->dry);
	} else {
		memset(&out_buffer->float_buffer[sample_offset], 0, sample_count * sizeof(float));
	}
	if (subgraph_in_buffer->write_count > 0) {
		dsp_mix_amp(&out_buffer->float_buffer[sample_offset], subgraph_in_buffer->float_buffer, sample_count, 1.0);
	}

	for (i = 0; i < subgraph_out_values.length; i++) {
		struct linx_value* v = &subgraph_out_values.items[i];
		if (v->pin_group == linx_pin_group_propagated) {
			linx_value_array_push_value(out_values, v->pin_index, v->pin_group, v, v->timestamp + sample_offset);
		}
	}

	for (i = 0; i < self->subgraph->graph->propagated_pin_count; i++) {
		if (self->subgraph->propagated_pins[i].pin_type == linx_pin_type_out_buffer_float) {
			struct linx_buffer* buffer = linx_graph_instance_get_propagated_pin_buffer(self->subgraph, i);
			linx_buffer_copy_chunk(&propagated_pin_buffers[i], buffer, sample_offset, 0, sample_count);
		}
	}

	data->rb.pos += sample_count;
	if (data->rb.pos == data->rb.eob) {
		data->rb.pos -= (data->rb.eob - data->rb.buffer);
		assert(data->rb.pos == data->rb.buffer);
	}

	assert(data->rb.pos < data->rb.eob);

	subgraph_out_buffer->write_count = 0;
	subgraph_in_buffer->write_count = 0;
	linx_graph_instance_process_clear(self->subgraph);
}

void delay_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	int processed_count = 0;
	struct delay* data = (struct delay*)self->data;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 2) {
			int rbsize;
			data->delay_ms = v->floatvalue;
			rbsize = (int)fmaxf(fminf(data->delay_ms / 1000.0f * data->samplerate, (float)MAX_DELAY_LENGTH), 1.0f);
			rb_setup(&data->rb, rbsize);
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 3) {
			data->fb = v->floatvalue; // feedback
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 4) {
			data->wet = v->floatvalue; // wet
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 5) {
			data->dry = v->floatvalue; // dry
		}
	}
		
	while (processed_count < sample_count) {
		int buffer_size = (data->rb.eob - data->rb.buffer);
		int buffer_pos = (data->rb.pos - data->rb.buffer);
		int chunk_count = fminf(buffer_size - buffer_pos, sample_count - processed_count);

		delay_process_subgraph(self, in_values, out_values, pin_buffers, propagated_pin_buffers, processed_count, chunk_count);

		processed_count += chunk_count;
		pin_buffers[1].write_count = 1;
	}
}

extern float amptodb (float amp);

void delay_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
	switch (pin_index) {
		case 2:
			snprintf(result_name, result_name_size, "%i ms", (int)value);
			break;
		case 3:
		case 4:
		case 5:
			snprintf(result_name, result_name_size, "%i%% (%.2f dB)", (int)(value * 100.0f), amptodb(value));
			break;
	}
}

void delay_create(struct linx_vertex_instance* plugin, int samplerate) {
	int rbsize;
	struct delay* data = (struct delay*)malloc(sizeof(struct delay));
	data->delay_ms = 750;
	data->samplerate = (float)samplerate;
	rb_init(&data->rb);
	data->wet = 0.0f;
	data->dry = 0.0f;
	data->fb = 0.0f;

	rbsize = (int)fminf(data->delay_ms / 1000.0f * data->samplerate, (float)MAX_DELAY_LENGTH);
	rb_setup(&data->rb, rbsize);

	plugin->data = data;
}

void delay_destroy(struct linx_vertex_instance* plugin) {
	free(plugin->data);
}

struct linx_factory delay_factory = {
	sizeof(struct linx_factory), "Delay", linx_factory_flag_is_subgraph_parent, 
	(sizeof(delay_pins) / sizeof(struct linx_pin)), delay_pins,
	(sizeof(delay_subgraph_pins) / sizeof(struct linx_pin)), delay_subgraph_pins,
	delay_create, delay_destroy, delay_process, delay_describe_value
};

struct linx_factory* delay_get_factory() {
	return &delay_factory;
}
