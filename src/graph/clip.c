#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mathext.h"
#include "linxaudio.h"
#include "inertia.h"

struct linx_pin clip_pins[] = {
	{ "In", linx_pin_type_in_buffer_float },
	{ "Out", linx_pin_type_out_buffer_float },
	{ "PreGain", linx_pin_type_in_scalar_float, 0.0f, 4.0f, 1.0f, 0.01f },
	{ "Clip", linx_pin_type_in_scalar_float, 0.0f, 1.0f, 1.0f, 0.01f },
	{ "PostGain", linx_pin_type_in_scalar_float, 0.0f, 4.0f, 1.0, 0.01f }
};

struct clip {
	inertia_value_t pregain;
	inertia_value_t a;
	inertia_value_t postgain;
	float samplerate;
};

void clip_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	float value;
	struct clip* data = (struct clip*)self->data;
	struct linx_buffer* in_buffer = &pin_buffers[0];
	struct linx_buffer* out_buffer = &pin_buffers[1];

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->timestamp == 0 && v->pin_index == 2) {
			inertia_set_ms(&data->pregain, 1, data->samplerate, v->floatvalue);
		} else if (v->pin_group == linx_pin_group_module && v->timestamp == 0 && v->pin_index == 3) {
			inertia_set_ms(&data->a, 1, data->samplerate, v->floatvalue);
		} else if (v->pin_group == linx_pin_group_module && v->timestamp == 0 && v->pin_index == 4) {
			inertia_set_ms(&data->postgain, 1, data->samplerate, v->floatvalue);
		}
	}

	if (in_buffer->write_count > 0) {

		for (i = 0; i < sample_count; i++) {
			inertia_update(&data->a);
			inertia_update(&data->pregain);
			inertia_update(&data->postgain);
			value = in_buffer->float_buffer[i] * data->pregain.current;
			value = fminf(fmaxf(value, -data->a.current), data->a.current) * data->postgain.current;
			out_buffer->write_count |= (value != 0.0f);
			out_buffer->float_buffer[i] = value;
		}

	} else {
		data->a.frames = 0;
	}
}

void clip_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
}

void clip_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct clip* data = (struct clip*)malloc(sizeof(struct clip));
	memset(data, 0, sizeof(struct clip));
	data->a.current = 1.0f;
	data->samplerate = (float)samplerate;
	inertia_set_ms(&data->a, 1, data->samplerate, 1);
	plugin->data = data;
}

void clip_destroy(struct linx_vertex_instance* plugin) {
	struct clip* data = (struct clip*)plugin->data;
	free(data);
}

struct linx_factory clip_factory = {
	sizeof(struct linx_factory), "Clip", 0,
	(sizeof(clip_pins) / sizeof(struct linx_pin)), clip_pins,
	0, 0,
	clip_create, clip_destroy, clip_process, clip_describe_value
};

struct linx_factory* clip_get_factory() {
	return &clip_factory;
}
