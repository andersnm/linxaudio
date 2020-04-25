#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "linxaudio.h"

struct linx_pin lfovalue_pins[] = {
	{ "Out", linx_pin_type_out_scalar_float, -1, 1, 0 },
	{ "Waveform", linx_pin_type_in_scalar_int, 0.0f, 6.0f, 0.0f },
	{ "Frequency", linx_pin_type_in_scalar_float, 0.001, 20, 0.1, 0.01 },
	{ "Phase", linx_pin_type_in_scalar_float, 0.0f, 1.0f, 0.0f, 0.01f },
	{ "Amp", linx_pin_type_in_scalar_float, 0.0f, 1.0f, 1.0f, 0.01 },
};

struct lfovalue {
	double phase;
	float phaseoffset;
	float amp;
	float freq;
	float samplerate;
	int wave;
	int t;
};

float osc_sample(float phi, int type);

void lfovalue_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	struct lfovalue* data = (struct lfovalue*)self->data;
	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 1) {
			// waveform changed
			data->wave = v->intvalue;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 2) {
			// freq changed
			data->freq = v->floatvalue;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 3) {
			data->phaseoffset = v->floatvalue;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 4) {
			data->amp = v->floatvalue;
		}
	}

	for (i = 0; i < sample_count; i++) {
		// generate out events pr N samples!
		if ((data->t % 16) == 0) {
			float value = osc_sample((float)data->phase + data->phaseoffset, data->wave) * data->amp;
			linx_value_array_push_float(out_values, 0, linx_pin_group_module, value, i);
		}
		data->t++;
		data->phase += (data->freq / data->samplerate);
		if (data->phase >= 1)
			data->phase -= 1;
	}
}

static const char* osc_names[] = {
	"Sine", "Square", "Saw", "InvSaw", "Triangle", "Gauss", "White"
};

void lfovalue_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
	switch (pin_index) {
		case 1:
			snprintf(result_name, result_name_size, "%s", osc_names[(int)value]);
			break;
		case 2:
			snprintf(result_name, result_name_size, "%.2f Hz", value);
			break;
	}

}

void lfovalue_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct lfovalue* data = (struct lfovalue*)malloc(sizeof(struct lfovalue));
	memset(data, 0, sizeof(struct lfovalue));
	data->phase = 0;
	data->phaseoffset = 0.0f;
	data->wave = 0;
	data->freq = 20.0f;
	data->t = 0;
	data->samplerate = (float)samplerate;
	plugin->data = data;
}

void lfovalue_destroy(struct linx_vertex_instance* plugin) {
	struct lfovalue* data = (struct lfovalue*)plugin->data;
	free(data);
}

struct linx_factory lfovalue_factory = {
	sizeof(struct linx_factory), "lfovalue", 0,
	(sizeof(lfovalue_pins) / sizeof(struct linx_pin)), lfovalue_pins,
	0, 0,
	lfovalue_create, lfovalue_destroy, lfovalue_process, lfovalue_describe_value
};

struct linx_factory* lfovalue_get_factory() {
	return &lfovalue_factory;
}
