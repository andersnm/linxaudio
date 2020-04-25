#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mathext.h"
#include "linxaudio.h"

struct linx_pin envscale_pins[] = {
	{ "InValue", linx_pin_type_in_scalar_float, -FLT_MAX, FLT_MAX, 0.0f, 0.0f },
	{ "InMin", linx_pin_type_in_scalar_float, -FLT_MAX, FLT_MAX, 0.0f, 0.0f },
	{ "InMax", linx_pin_type_in_scalar_float, -FLT_MAX, FLT_MAX, 1.0f, 0.0f },
	{ "OutValue", linx_pin_type_out_scalar_float, -FLT_MAX, FLT_MAX, 0.0f, 0.0f },
	{ "OutMin", linx_pin_type_in_scalar_float, -FLT_MAX, FLT_MAX, 0.0f, 0.0f },
	{ "OutMax", linx_pin_type_in_scalar_float, -FLT_MAX, FLT_MAX, 1.0f, 0.0f },
	{ "EnvMod", linx_pin_type_in_scalar_float, 0.0f, 1.0f, 1.0f, 0.01f },
};

struct envscale {
	float in_value;
	float in_min;
	float in_max;
	float out_min;
	float out_max;
	float env_mod;
};

void envscale_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	int emit = 0;
	struct envscale* data = (struct envscale*)self->data;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->timestamp == 0 && v->pin_index == 0) {
			data->in_value = v->floatvalue;
			emit = 1;
		} else if (v->pin_group == linx_pin_group_module && v->timestamp == 0 && v->pin_index == 1) {
			data->in_min = v->floatvalue;
			emit = 1;
		} else if (v->pin_group == linx_pin_group_module && v->timestamp == 0 && v->pin_index == 2) {
			data->in_max = v->floatvalue;
			emit = 1;
		} else if (v->pin_group == linx_pin_group_module && v->timestamp == 0 && v->pin_index == 3) {
			assert(0); // the out pin
		} else if (v->pin_group == linx_pin_group_module && v->timestamp == 0 && v->pin_index == 4) {
			data->out_min = v->floatvalue;
			emit = 1;
		} else if (v->pin_group == linx_pin_group_module && v->timestamp == 0 && v->pin_index == 5) {
			data->out_max = v->floatvalue;
			emit = 1;
		} else if (v->pin_group == linx_pin_group_module && v->timestamp == 0 && v->pin_index == 6) {
			data->env_mod = v->floatvalue;
			emit = 1;
		}
	}

	if (emit) {
		float in_diff = (data->in_max - data->in_min);
		float out_diff = (data->out_max - data->out_min);
		
		float out_value = (data->in_value - data->in_min) / in_diff;
		out_value = (out_value * data->env_mod * out_diff) + data->out_min;

		if (data->out_max >= data->out_min) {
			out_value = fminf(fmaxf(out_value, data->out_min), data->out_max);
		} else {
			out_value = fmaxf(fminf(out_value, data->out_min), data->out_max);
		}
		linx_value_array_push_float(out_values, 3, linx_pin_group_module, out_value, 0);
	}

}

void envscale_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct envscale* data = (struct envscale*)malloc(sizeof(struct envscale));
	memset(data, 0, sizeof(struct envscale));
	//data->user_max = 1.0f;
	data->env_mod = 1.0f;
	plugin->data = data;
}

void envscale_destroy(struct linx_vertex_instance* plugin) {
	struct envscale* data = (struct envscale*)plugin->data;
	free(data);
}

struct linx_factory envscale_factory = {
	sizeof(struct linx_factory), "EnvScale", 0,
	(sizeof(envscale_pins) / sizeof(struct linx_pin)), envscale_pins,
	0, 0,
	envscale_create, envscale_destroy, envscale_process, 0
};

struct linx_factory* envscale_get_factory() {
	return &envscale_factory;
}
