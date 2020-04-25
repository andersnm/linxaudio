#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "linxaudio.h"

struct linx_pin midinotesplit_pins[] = {
	{ "MidiIn", linx_pin_type_in_midi },
	{ "MidiChannel", linx_pin_type_in_scalar_int, 0, 16, 0 },
	{ "OutFreq", linx_pin_type_out_scalar_float, 20, 440*5, 440 },
	{ "OutVelo", linx_pin_type_out_scalar_float, 0, 1, 1 },
	{ "OutTrigger", linx_pin_type_out_scalar_int, 0, 1, 1 },
	{ "OutNote", linx_pin_type_out_scalar_int, 0, 8*12, 0 },
};

struct midinotesplit {
	int ch;
	float samplerate;
};

void midinotesplit_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	struct midinotesplit* data = (struct midinotesplit*)self->data;
	unsigned short status;
	unsigned char channel;
	unsigned char command;
	unsigned char data1;
	unsigned char data2;

	float freq;
	float velo;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 0) {
			midi_parse(v->midimessage, &status, &channel, &command, &data1, &data2);
			if (command == 8 || (command == 9 && data2 == 0) || (command == 0xb && data1 == 0x7b)) {
				// note off
				linx_value_array_push_int(out_values, 4, linx_pin_group_module, 0, v->timestamp);
			} else if (command == 9) {
				// note on
				freq = 440.0f * powf(2.0f, ((float)data1 - 57.0f) / 12.0f);
				velo = (float)data2 / 127.0f;
				linx_value_array_push_float(out_values, 2, linx_pin_group_module, freq, v->timestamp);
				linx_value_array_push_float(out_values, 3, linx_pin_group_module, velo, v->timestamp);
				linx_value_array_push_int(out_values, 4, linx_pin_group_module, 1, v->timestamp);
				linx_value_array_push_int(out_values, 5, linx_pin_group_module, data1, v->timestamp);
			}
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 1) {
			data->ch = v->intvalue;
		}
	}
}

void midinotesplit_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct midinotesplit* midinotesplit = (struct midinotesplit*)malloc(sizeof(struct midinotesplit));
	memset(midinotesplit, 0, sizeof(struct midinotesplit));
	midinotesplit->ch = 0;
	midinotesplit->samplerate = samplerate;
	plugin->data = midinotesplit;
}

void midinotesplit_destroy(struct linx_vertex_instance* plugin) {
	struct midinotesplit* data = (struct midinotesplit*)plugin->data;
	free(data);
}

struct linx_factory midinotesplit_factory = {
	sizeof(struct linx_factory), "MidiNoteSplit", 0,
	(sizeof(midinotesplit_pins) / sizeof(struct linx_pin)), midinotesplit_pins,
	0, 0,
	midinotesplit_create, midinotesplit_destroy, midinotesplit_process
};

struct linx_factory* midinotesplit_get_factory() {
	return &midinotesplit_factory;
}
