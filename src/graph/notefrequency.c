#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include "linxaudio.h"

struct linx_pin notefrequency_pins[] = {
	{ "In", linx_pin_type_in_scalar_float, 0.0f, 96.0f, 0.0f, 0.01f },
	{ "Detune", linx_pin_type_in_scalar_float, -12.0f, 12.0f, 0.0f, 0.01f },
	{ "OutFreq", linx_pin_type_out_scalar_float, 20, 440*5, 440 },
};

struct notefrequency {
	float note;
	float detune;
};

void notefrequency_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	struct notefrequency* data = (struct notefrequency*)self->data;
	int has_note = 0;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 0 && v->timestamp == 0) {
			data->note = v->floatvalue;
			has_note = 1;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 1 && v->timestamp == 0) {
			data->detune = v->floatvalue;
			has_note = 2;
		}
	}

	if (has_note && data->note > 0) {
		float freq = 440.0f * powf(2.0f, ((data->note + data->detune) - 57.0f) / 12.0f); 
		linx_value_array_push_float(out_values, 2, linx_pin_group_module, freq, 0);
	}
}

void notefrequency_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
}

void notefrequency_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct notefrequency* data = (struct notefrequency*)malloc(sizeof(struct notefrequency));
	memset(data, 0, sizeof(struct notefrequency));
	plugin->data = data;
}

void notefrequency_destroy(struct linx_vertex_instance* plugin) {
	struct notefrequency* data = (struct notefrequency*)plugin->data;
	free(data);
}

struct linx_factory notefrequency_factory = {
	sizeof(struct linx_factory), "NoteFrequency", 0,
	(sizeof(notefrequency_pins) / sizeof(struct linx_pin)), notefrequency_pins,
	0, 0,
	notefrequency_create, notefrequency_destroy, notefrequency_process, notefrequency_describe_value
};

struct linx_factory* notefrequency_get_factory() {
	return &notefrequency_factory;
}
