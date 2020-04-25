#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "mathext.h"
#include "linxaudio.h"

// State Variable Filter (Double Sampled, Stable)
// based on information from Andrew Simper, Laurent de Soras,
// and Steffan Diedrichsen

struct linx_pin svf_pins[] = {
	{ "In", linx_pin_type_in_buffer_float },
	{ "Out", linx_pin_type_out_buffer_float },
	{ "Type", linx_pin_type_in_scalar_int, 0.0f, 4.0f, 1.0f, 1.0f },
	{ "Cutoff", linx_pin_type_in_scalar_float, 10.0f, 20000.0f, 10000.0f, 1.0f },
	{ "Resonance", linx_pin_type_in_scalar_float, 0.0f, 1.0f,  0.0f, 0.1f  },
};

struct svf {
	float fs; // sampling frequency
	float fc; // cutoff frequency normally something like: 440.0*pow(2.0, (midi_note - 69.0)/12.0);
	float res; // resonance 0 to 1;
	float drive; // internal distortion 0 to 0.1
	float freq; // 2.0*sin(PI*MIN(0.25, fc/(fs*2)));  // the fs*2 is because it's double sampled
	float damp; // MIN(2.0*(1.0 - pow(res, 0.25)), MIN(2.0, 2.0/freq - freq*0.5));
	float v[5]; // 0=notch,1=low,2=high,3=band,4=peak
	int mode;
};

void svf_setup(struct svf* svf) {
	svf->freq = 2.0f * sinf(M_PI*fminf(0.25f, svf->fc/(svf->fs*2.0f)));
	svf->damp = fminf(2.0f*(1.0f - powf(svf->res, 0.25f)), fminf(2.0f, 2.0f/svf->freq - svf->freq*0.5f));
}

#pragma optimize("", off)
	static void normalize(float* y, int c, int s) {
#define ANTI_DENORMAL_FLT 1e-20f
		int i;
		for (i = 0; i < c; i += s) {
			y[i] += ANTI_DENORMAL_FLT;
		}
		for (i = 0; i < c; i += s) {
			y[i] -= ANTI_DENORMAL_FLT;
		}
	}
#pragma optimize("", on)

void svf_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	float in, out;
	struct linx_buffer* in_buffer = &pin_buffers[0];
	struct linx_buffer* out_buffer = &pin_buffers[1];
	float* inptr = in_buffer->float_buffer;
	float* outptr = out_buffer->float_buffer;

	struct svf* data = (struct svf*)self->data;
	int changed = 0;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 2) {
			data->mode = v->intvalue; // type
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 3) {
			data->fc = v->floatvalue; // cutoff
			changed = 1;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 4) {
			data->res = v->floatvalue;// resonance
			changed = 1;
		}
	}
	if (changed) {
		svf_setup(data);
	}

	while (sample_count--) {
		if (in_buffer->write_count > 0) {
			in = *inptr++;
		} else {
			in = 0;
		}
		data->v[0] = in - data->damp * data->v[3];
		data->v[1] = data->v[1] + data->freq * data->v[3];
		data->v[2] = data->v[0] - data->v[1];
		data->v[3] = data->freq * data->v[2] + data->v[3] - data->drive * data->v[3] * data->v[3] * data->v[3];
		out = 0.5f * data->v[data->mode];
		data->v[0] = in - data->damp * data->v[3];
		data->v[1] = data->v[1] + data->freq * data->v[3];
		data->v[2] = data->v[0] - data->v[1];
		data->v[3] = data->freq * data->v[2] + data->v[3] - data->drive * data->v[3] * data->v[3] * data->v[3];
		out += 0.5f * data->v[data->mode];
		*outptr++ = out;
	}
	normalize(data->v, 5, 1);

	out_buffer->write_count = 1;
}

const char* svf_type_names[] = {
	"Notch", "Low", "High", "Band", "Peak"
};

void svf_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
	switch (pin_index) {
		case 2:
			// type
			snprintf(result_name, result_name_size, "%s", svf_type_names[(int)value]);
			break;
		case 3:
			// cutoff
			snprintf(result_name, result_name_size, "%0.0f Hz", value);
			break;
	}
}

void svf_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct svf* svf = (struct svf*)malloc(sizeof(struct svf));
	memset(svf, 0, sizeof(struct svf));
	svf->mode = 1;
	svf->fs = (float)samplerate;
	svf->fc = 523;
	svf->res = 0;
	svf->drive = 0.0f;
	svf_setup(svf);
	memset(svf->v, 0, sizeof(svf->v));
	plugin->data = svf;
}

void svf_destroy(struct linx_vertex_instance* plugin) {
	struct svf* data = (struct svf*)plugin->data;
	free(data);
}

struct linx_factory svf_factory = {
	sizeof(struct linx_factory), "Svf", 0,
	(sizeof(svf_pins) / sizeof(struct linx_pin)), svf_pins,
	0, 0,
	svf_create, svf_destroy, svf_process, svf_describe_value
};

struct linx_factory* svf_get_factory() {
	return &svf_factory;
}
