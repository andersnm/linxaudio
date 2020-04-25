#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "linxaudio.h"

struct linx_pin adsrvalue_pins[] = {
	{ "Trigger", linx_pin_type_in_scalar_int, 0, 1, 0 },
	{ "Out", linx_pin_type_out_scalar_float, 0, 1, 0 },
	{ "Attack", linx_pin_type_in_scalar_int, 0, 1000, 100 },
	{ "Decay", linx_pin_type_in_scalar_int, 0, 1000, 100 },
	{ "Sustain", linx_pin_type_in_scalar_float, 0, 1, 0.7, 0.01 },
	{ "Release", linx_pin_type_in_scalar_int, 0, 1000, 300 },
};

enum adsrstate {
	adsrstate_off,
	adsrstate_attack,
	adsrstate_decay,
	adsrstate_sustain,
	adsrstate_release,
};

struct adsr {
	int a, d, r;
	float s;
	int t;
	enum adsrstate state;
	float samplerate;
};

float adsr_get_value(struct adsr* data) {
	if (data->state == adsrstate_attack) {
		return (float)data->t / (float)data->a;
	} else if (data->state == adsrstate_decay) {
		return 1.0f - (1.0f - data->s) * ((float)data->t / (float)data->d);
	} else if (data->state == adsrstate_sustain) {
		return data->s;
	} else if (data->state == adsrstate_release) {
		return data->s*((float)(data->r - data->t) / (float)data->r);
	} else {
		return 0.0f;
	}
}

void adsr_update(struct adsr* data) {
	if (data->state == adsrstate_attack) {
		if (data->t >= data->a) {
			data->t -= data->a;
			data->state = adsrstate_decay;
		}
	}

	if (data->state == adsrstate_decay) {
		if (data->t >= data->d) {
			data->t -= data->d;
			data->state = adsrstate_sustain;
		}
	}

	if (data->state == adsrstate_release) {
		if (data->t >= data->r) {
			data->t = -1;
			data->state = adsrstate_off;
		}
	}

}

void adsrvalue_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	struct adsr* data = (struct adsr*)self->data;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 0) {
			// trigger 
			if (v->intvalue == 1) {
				data->t = 0;
				data->state = adsrstate_attack;
			} else if (v->intvalue == 0 && data->state != adsrstate_off) {
				data->t = 0;
				data->state = adsrstate_release;
			}
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 1) {
			// out value set below
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 2) {
			data->a = (float)v->intvalue / 1000.0f * data->samplerate;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 3) {
			data->d = (float)v->intvalue / 1000.0f * data->samplerate;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 4) {
			data->s = v->floatvalue;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 5) {
			data->r = (float)v->intvalue / 1000.0f * data->samplerate;
		}
	}

	if (data->t >= 0) {
		for (i = 0; i < sample_count; i++) {
			// generate out events pr N samples!
			if ((data->t % 16) == 0) {
				float value;

				adsr_update(data);
				value = adsr_get_value(data);
				linx_value_array_push_float(out_values, 1, linx_pin_group_module, value, i);

				if (data->t == -1) {
					break;
				}
			}
			data->t++;
		}
	}
}

void adsrvalue_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct adsr* adsr = (struct adsr*)malloc(sizeof(struct adsr));
	memset(adsr, 0, sizeof(struct adsr));
	adsr->a = 100;
	adsr->d = 100;
	adsr->s = 0.7;
	adsr->r = 300;
	adsr->t = -1;
	adsr->state = adsrstate_off;
	adsr->samplerate = (float)samplerate;
	plugin->data = adsr;
}

void adsrvalue_destroy(struct linx_vertex_instance* plugin) {
	struct adsr* data = (struct adsr*)plugin->data;
	free(data);
}

struct linx_factory adsrvalue_factory = {
	sizeof(struct linx_factory), "AdsrValue", 0,
	(sizeof(adsrvalue_pins) / sizeof(struct linx_pin)), adsrvalue_pins,
	0, 0,
	adsrvalue_create, adsrvalue_destroy, adsrvalue_process, 0
};

struct linx_factory* adsrvalue_get_factory() {
	return &adsrvalue_factory;
}
