#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "linxaudio.h"

struct linx_pin distortion_pins[] = {
	{ "In", linx_pin_type_in_buffer_float },
	{ "Out", linx_pin_type_out_buffer_float },
	{ "Type", linx_pin_type_in_scalar_int, 0.0f, 2.0f, 1.0f, 1.0f },
	{ "Amount", linx_pin_type_in_scalar_float, 1.0f, 8.0f, 1.0f, 0.01f }
};

struct distortion {
	int type;
	float amount;
};

float signf(float f) {
	return f >= 0 ? 1.0f : -1.0f;
}

void dsp_copy_tanh(float* dest, float* src, int sample_count, float amount) {
	int i;
	for (i = 0; i < sample_count; i++) {
		float x = src[i];
		dest[i] = tanhf(x * amount);
	}
}

void dsp_copy_atan(float* dest, float* src, int sample_count, float amount) {
	int i;
	for (i = 0; i < sample_count; i++) {
		float x = src[i];
		dest[i] = atanf(x * amount);
	}
}

void dsp_copy_pow(float* dest, float* src, int sample_count, float amount) {
	int i;
	for (i = 0; i < sample_count; i++) {
		float x = src[i];
		dest[i] = signf(x) * powf(x, amount);
	}
}

void distortion_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	struct linx_buffer* in_buffer = &pin_buffers[0];
	struct linx_buffer* out_buffer = &pin_buffers[1];
	struct distortion* data = (struct distortion*)self->data;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 2) {
			data->type = v->intvalue;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 3) {
			data->amount = v->floatvalue;
		}
	}

	if (in_buffer->write_count > 0) {
		switch (data->type) {
			case 0:
				dsp_copy_tanh(out_buffer->float_buffer, in_buffer->float_buffer, sample_count, data->amount);
				break;
			case 1:
				dsp_copy_atan(out_buffer->float_buffer, in_buffer->float_buffer, sample_count, data->amount);
				break;
			case 2:
				dsp_copy_pow(out_buffer->float_buffer, in_buffer->float_buffer, sample_count, data->amount);
				break;
		}
		out_buffer->write_count = 1;
	}
}

const char* distortion_type_names[] = {
	"tanh(x*a)", "atan(x*a)", "sign(x)*pow(x, a)"
};

void distortion_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
	switch (pin_index) {
		case 2:
			// type
			snprintf(result_name, result_name_size, "%s", distortion_type_names[(int)value]);
			break;
	}
}

void distortion_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct distortion* distortion = (struct distortion*)malloc(sizeof(struct distortion));
	memset(distortion, 0, sizeof(struct distortion));
	distortion->type = 1;
	plugin->data = distortion;
}

void distortion_destroy(struct linx_vertex_instance* plugin) {
	struct distortion* data = (struct distortion*)plugin->data;
	free(data);
}

struct linx_factory distortion_factory = {
	sizeof(struct linx_factory), "Distortion", 0,
	(sizeof(distortion_pins) / sizeof(struct linx_pin)), distortion_pins,
	0, 0,
	distortion_create, distortion_destroy, distortion_process, distortion_describe_value
};

struct linx_factory* distortion_get_factory() {
	return &distortion_factory;
}
