#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "linxaudio.h"
#include "inertia.h"

struct linx_pin gain_pins[] = {
	{ "In", linx_pin_type_in_buffer_float },
	{ "Out", linx_pin_type_out_buffer_float },
	{ "Amp", linx_pin_type_in_scalar_float, 0.0f, 4.0f, 1.0f, 0.01f }
};

struct gain {
	inertia_value_t a;
	float samplerate;
};

void gain_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	struct gain* data = (struct gain*)self->data;
	struct linx_buffer* in_buffer = &pin_buffers[0];
	struct linx_buffer* out_buffer = &pin_buffers[1];

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 2) {
			inertia_set_ms(&data->a, 1, data->samplerate, v->floatvalue);
		}
	}

	if (in_buffer->write_count > 0) {
		if (data->a.current != 0) {
			for (i = 0; i < sample_count; i++) {
				inertia_update(&data->a);
				out_buffer->float_buffer[i] = in_buffer->float_buffer[i] * data->a.current;
			}

			out_buffer->write_count = 1;
		} else {
			for (i = 0; i < sample_count; i++) {
				inertia_update(&data->a);
			}
		}
	} else {
		data->a.frames = 0;
	}
}

float amptodb (float amp) { 
	return log10f(amp) * 20.f; 
}

void gain_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
	switch (pin_index) {
		case 2:
			snprintf(result_name, result_name_size, "%i%% (%.2f dB)", (int)(value * 100.0f), amptodb(value));
			break;
	}
}

void gain_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct gain* gain = (struct gain*)malloc(sizeof(struct gain));
	memset(gain, 0, sizeof(struct gain));
	gain->a.current = 1.0f;
	gain->samplerate = (float)samplerate;
	inertia_set_ms(&gain->a, 1, gain->samplerate, 1);
	plugin->data = gain;
}

void gain_destroy(struct linx_vertex_instance* plugin) {
	struct gain* data = (struct gain*)plugin->data;
	free(data);
}

struct linx_factory gain_factory = {
	sizeof(struct linx_factory), "Gain", 0,
	(sizeof(gain_pins) / sizeof(struct linx_pin)), gain_pins,
	0, 0,
	gain_create, gain_destroy, gain_process, gain_describe_value
};

struct linx_factory* gain_get_factory() {
	return &gain_factory;
}
