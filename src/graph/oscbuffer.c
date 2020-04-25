#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mathext.h"
#include "linxaudio.h"
#include "inertia.h"

struct linx_pin oscbuffer_pins[] = {
	{ "Out", linx_pin_type_out_buffer_float },
	{ "Waveform", linx_pin_type_in_scalar_int, 0.0f, 6.0f, 0.0f },
	{ "Frequency", linx_pin_type_in_scalar_float, 220.0f, 440.0f * 5.0f,  440.0f, 0.1f },
	{ "Phase", linx_pin_type_in_scalar_float, 0.0f, 1.0f, 0.0f, 0.01f },
	{ "Amp", linx_pin_type_in_scalar_float, 0.0f, 1.0f, 1.0f, 0.01f },
};

struct lfo {
	double phase;
	float phaseoffset;
	inertia_value_t amp;
	inertia_value_t freq;
	float samplerate;
	int wave;
};

float osc_rnd() {
	// http://stackoverflow.com/questions/521295/javascript-random-seeds
	static int w = 123456789;
	static int z = 987654321;
	float result;
	assert(w != 0);
	z = (36969 * (z & 65535) + (z >> 16));
	w = (18000 * (w & 65535) + (w >> 16));
	result = (float)(int)((z << 16) + w);
	result /= 4294967296.0f;
	return result + 0.5f;
}

float osc_gauss_rnd() {
	// http://www.musicdsp.org/archive.php?classid=0#129
	const int q = 15;
    const float c1 = (float)((1 << q) - 1);
    const float c2 = (c1 / 3) + 1;
    const float c3 = 1 / c1;
	float r1 = osc_rnd() * c2;
	float r2 = osc_rnd() * c2;
	float r3 = osc_rnd() * c2;
	return (2 * (r1+r2+r3) - 3 * (c2 - 1)) * c3;
}

float osc_sample(float phi, int type) {
	switch(type) {
		case 0:
			// Sine wave. ~~~
			return sinf(phi * 2.0f * (float)M_PI);
		case 1:
			// Square wave. -_-_
			if (sinf(phi * 2.0f * (float)M_PI) > 0.0f)
				return 1.0f;
			else
				return -1.0f;
		case 2:
			// Saw wave. /|/|
			phi = fmodf(phi, 1.0f);
			return phi * 2.0f - 1.0f;
		case 3:
			// Inverted saw wave. \|\|
			phi = fmodf(phi, 1.0f);
			return -(phi * 2.0f - 1.0f);
		case 4:
			// Triangle. /\/\/
			phi = fmodf(phi, 1.0f);
			if (phi < 0.5) {
				return phi * 4.0f - 1.0f; // 0..0.5 -> -1..1
			} else {
				return 1.0f - ((phi - 0.5f) * 4.0f); // 0.5..1 -> 1..-1
			}
		case 5:
			return osc_gauss_rnd();// * 2.0f - 1.0f;
		case 6:
			return osc_rnd() * 2.0f - 1.0f;
		default:
			// Silence.
			return 0.0;
	}
}

void oscbuffer_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	struct linx_buffer* out_buffer = &pin_buffers[0];
	struct lfo* data = (struct lfo*)self->data;
	float frame_amp;
	float value;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 1) {
			// waveform changed
			data->wave = v->intvalue;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 2) {
			// freq changed
			inertia_set_ms(&data->freq, 1, data->samplerate, v->floatvalue);
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 3) {
			data->phaseoffset = v->floatvalue;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 4) {
			inertia_set_ms(&data->amp, 1, data->samplerate, v->floatvalue);
		}
	}

	frame_amp = fmaxf(data->amp.current, data->amp.target);

	if (frame_amp != 0) {

		for (i = 0; i < sample_count; i++) {
			inertia_update(&data->freq);
			inertia_update(&data->amp);

			value = osc_sample((float)data->phase + data->phaseoffset, data->wave) * data->amp.current;
			out_buffer->float_buffer[i] = value;

			data->phase += (data->freq.current / data->samplerate);
			if (data->phase >= 1)
				data->phase -= 1;
		}

		out_buffer->write_count = 1;
	}
}

const char* osc_names[] = {
	"Sine", "Square", "Saw", "InvSaw", "Triangle", "Gauss", "White"
};

void oscbuffer_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
	switch (pin_index) {
		case 1:
			snprintf(result_name, result_name_size, "%s", osc_names[(int)value]);
			break;
		case 2:
			snprintf(result_name, result_name_size, "%.2f Hz", value);
			break;
	}

}

void oscbuffer_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct lfo* lfo = (struct lfo*)malloc(sizeof(struct lfo));
	memset(lfo, 0, sizeof(struct lfo));
	lfo->phase = 0.0f;
	lfo->phaseoffset = 0.0f;
	lfo->wave = 0;
	lfo->freq.current = 440.0f;
	lfo->amp.current = 0.0f;
	lfo->samplerate = (float)samplerate;
	inertia_set_ms(&lfo->freq, 1, lfo->samplerate, 440.0f);
	inertia_set_ms(&lfo->amp, 1, lfo->samplerate, 1);
	plugin->data = lfo;
}

void oscbuffer_destroy(struct linx_vertex_instance* plugin) {
	struct lfo* data = (struct lfo*)plugin->data;
	free(data);
}

struct linx_factory oscbuffer_factory = {
	sizeof(struct linx_factory),  "OscBuffer", 0,
	(sizeof(oscbuffer_pins) / sizeof(struct linx_pin)), oscbuffer_pins,
	0, 0,
	oscbuffer_create, oscbuffer_destroy, oscbuffer_process, oscbuffer_describe_value
};

struct linx_factory* oscbuffer_get_factory() {
	return &oscbuffer_factory;
}
