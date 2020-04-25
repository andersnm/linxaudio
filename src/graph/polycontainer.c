#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "linxaudio.h"

#define max_voices 8

struct note_map {
	int note_voices[max_voices];
};

void note_map_clear(struct note_map* nm) {
	memset(nm->note_voices, 0, sizeof(nm->note_voices));
}

int note_map_press_note(struct note_map* nm, int note) {
	int i;
	for (i = 0; i < max_voices; i++) {
		if (nm->note_voices[i] == 0) {
			nm->note_voices[i] = note;
			return i;
		}
	}
	return -1;
}

int note_map_release_note(struct note_map* nm, int note) {
	int i;
	for (i = 0; i < max_voices; i++) {
		if (nm->note_voices[i] == note) {
			nm->note_voices[i] = 0;
			return i;
		}
	}
	return -1;
}

struct linx_pin polycontainer_pins[] = {
	{ "Midi In", linx_pin_type_in_midi },
	{ "Audio Out", linx_pin_type_out_buffer_float },
};

struct linx_pin polycontainer_subgraph_pins[] = {
	{ "From Midi In", linx_pin_type_out_midi },
	{ "To Audio Out", linx_pin_type_in_buffer_float },
};

struct voice {
	struct linx_graph_instance* subgraph;
	struct linx_value subgraph_in_value_data[max_value_queue_per_vertex];
	struct linx_value_array subgraph_in_values;
};

struct polycontainer {
	struct note_map voice_notes;
	struct voice voices[max_voices];
};

void polycontainer_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {

	int i, j;
	struct polycontainer* data = (struct polycontainer*)self->data;
	struct linx_value subgraph_out_value_data[max_value_queue_per_vertex];
	struct linx_value_array subgraph_out_values = { subgraph_out_value_data, 0, max_value_queue_per_vertex };
	struct linx_buffer* subgraph_buffers = self->subgraph_pin_buffers;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 0 && v->timestamp == 0) {

			unsigned short status;
			unsigned char channel, command, data1, data2;
			midi_parse(v->midimessage, &status, &channel, &command, &data1, &data2);

			// in MPE-mode, use midi channel first to designate voices ?
			// in MPE-mode, notes arrive on midi channel 1-15, and global sone messages in channel 0
			// in non-MPE mode, notes arrive on midi channel 0
			// dual mode and MPE mode?? dual mode = 

			// MPE mode = rpn 6 ? support MPE setup commands for different sones??

			if (command == 0xb && data1 == 0x7b) { // all notes off
				for (j = 0; j < max_voices; j++) {
					data->voice_notes.note_voices[j] = 0;
					linx_value_array_push_midi(&data->voices[j].subgraph_in_values, 0, linx_pin_group_module, v->midimessage, v->timestamp);
				}
			} else if ((command == 9 && data2 == 0) || command == 8) { // noteoff
				int voice = note_map_release_note(&data->voice_notes, data1);
				assert(data1 != 0);
				if (voice != -1) {
					linx_value_array_push_midi(&data->voices[voice].subgraph_in_values, 0, linx_pin_group_module, v->midimessage, v->timestamp);
				}
			} else if (command == 9) { // noteon
				int voice = note_map_press_note(&data->voice_notes, data1);
				if (voice != -1) {
					linx_value_array_push_midi(&data->voices[voice].subgraph_in_values, 0, linx_pin_group_module, v->midimessage, v->timestamp);
				}
			} else {
				; // discard non-note midi message
			}
		} else if (v->pin_group == linx_pin_group_propagated) {
			// pass on to subgraphs
			for (j = 0; j < max_voices; j++) {
				linx_value_array_push_value(&data->voices[j].subgraph_in_values, v->pin_index, v->pin_group, v, v->timestamp);
			}
		}
	}

	// process voices, append outvalues
	for (i = 0; i < max_voices; i++) {
		// NOTE: passing in self as the context, edges out of the subgraphs are mixed automatically in the vertex' subgraph buffers
		linx_vertex_instance_process_subgraph(data->voices[i].subgraph, self, &data->voices[i].subgraph_in_values, &subgraph_out_values, sample_count);
	}

	// copy subin->pin buffer
	if (subgraph_buffers[1].write_count > 0) {
		linx_buffer_write(&pin_buffers[1], subgraph_buffers[1].float_buffer, sample_count);
	}

	for (i = 0; i < subgraph_out_values.length; i++) {
		struct linx_value* v = &subgraph_out_values.items[i];
		if (v->pin_group == linx_pin_group_propagated) {
			linx_value_array_push_value(out_values, v->pin_index, v->pin_group, v, v->timestamp);
		}
	}

	self->subgraph_pin_buffers[1].write_count = 0;

	for (i = 0; i < max_voices; i++) {
		data->voices[i].subgraph_in_values.length = 0;
		linx_graph_instance_process_clear(data->voices[i].subgraph);
	}

}

void polycontainer_create(struct linx_vertex_instance* plugin, int samplerate) {
	int i;
	struct polycontainer* container = (struct polycontainer*)malloc(sizeof(struct polycontainer));
	memset(container, 0, sizeof(struct polycontainer));
	plugin->data = container;

	// setup a subgraph for each voice: use the default subgraph as the first
	// voice and its definition to create a standalone subgraph for each
	// remaining voice.

	note_map_clear(&container->voice_notes);
	container->voices[0].subgraph = plugin->subgraph;

	for (i = 0; i < max_voices; i++) {
		if (i != 0) {
			container->voices[i].subgraph = linx_graph_definition_create_instance(plugin->subgraph->graph, samplerate);
		}
		linx_value_array_init_from(&container->voices[i].subgraph_in_values, container->voices[i].subgraph_in_value_data, 0, max_value_queue_per_vertex);
	}
}

void polycontainer_destroy(struct linx_vertex_instance* plugin) {
	int i;
	struct polycontainer* data = (struct polycontainer*)plugin->data;
	for (i = 1; i < max_voices; i++) {
		linx_graph_instance_destroy(data->voices[i].subgraph);
	}
	free(plugin->data);
}

struct linx_factory polycontainer_factory = {
	sizeof(struct linx_factory), "PolyContainer", linx_factory_flag_is_subgraph_parent,
	(sizeof(polycontainer_pins) / sizeof(struct linx_pin)), polycontainer_pins,
	(sizeof(polycontainer_subgraph_pins) / sizeof(struct linx_pin)), polycontainer_subgraph_pins,
	polycontainer_create, polycontainer_destroy, polycontainer_process
};

struct linx_factory* polycontainer_get_factory() {
	return &polycontainer_factory;
}
