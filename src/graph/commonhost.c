#include <stdlib.h>
#include "linxaudio.h"
#include "commonhost.h"

struct linx_graph_definition* linx_host_get_graph() {
	return linx_graph;
}

void linx_host_get_pin_count(struct linx_graph_definition* subgraph, int* result_param_in_count, int* result_param_out_count, int* result_audio_in_count, int* result_audio_out_count, int* result_midi_in_count, int* result_midi_out_count) {
	int i;
	int param_in_count = 0;
	int param_out_count = 0;
	int midi_in_count = 0;
	int midi_out_count = 0;
	int audio_in_count = 0;
	int audio_out_count = 0;
	struct linx_vertex_definition* resolved_vertdef;
	int resolved_pin_index;

	for (i = 0 ; i < subgraph->propagated_pin_count; i++) {
		struct linx_pin* p = linx_graph_definition_resolve_pin(subgraph, i, &resolved_vertdef, &resolved_pin_index);

		if (p->pin_type == linx_pin_type_in_scalar_float || p->pin_type == linx_pin_type_in_scalar_int) {
			param_in_count++;
		} else if (p->pin_type == linx_pin_type_out_scalar_float || p->pin_type == linx_pin_type_out_scalar_int) {
			param_out_count++;
		} else if (p->pin_type == linx_pin_type_in_midi) {
			midi_in_count++;
		} else if (p->pin_type == linx_pin_type_out_midi) {
			midi_out_count++;
		} else if (p->pin_type == linx_pin_type_in_buffer_float) {
			audio_in_count++;
		} else if (p->pin_type == linx_pin_type_out_buffer_float) {
			audio_out_count++;
		}
	}
	*result_param_in_count = param_in_count;
	*result_param_out_count = param_out_count;
	*result_audio_in_count = audio_in_count;
	*result_audio_out_count = audio_out_count;
	*result_midi_in_count = midi_in_count;
	*result_midi_out_count = midi_out_count;
}

void linx_host_get_pins(struct linx_graph_definition* subgraph, int* result_param_in_count, struct linx_host_parameter** result_in_pins, int* result_audio_in_count, struct linx_host_parameter** result_in_audios, int* result_audio_out_count, struct linx_host_parameter** result_out_audios, int* result_midi_in_count, struct linx_host_parameter** result_in_midis) {
	int i;
	int param_in_index = 0;
	int param_in_count = 0;
	int param_out_count = 0;
	int audio_in_index = 0;
	int audio_in_count = 0;
	int audio_out_index = 0;
	int audio_out_count = 0;
	int midi_in_index = 0;
	int midi_in_count = 0;
	int midi_out_count = 0;
	struct linx_host_parameter* in_parameters;
	struct linx_host_parameter* in_midis;
	struct linx_host_parameter* in_audios;
	struct linx_host_parameter* out_audios;
	struct linx_vertex_definition* resolved_vertdef;
	int resolved_pin_index;

	linx_host_get_pin_count(subgraph, &param_in_count, &param_out_count, &audio_in_count, &audio_out_count, &midi_in_count, &midi_out_count);

	in_parameters = (struct linx_host_parameter*)malloc(sizeof(struct linx_host_parameter) * param_in_count);
	in_midis = (struct linx_host_parameter*)malloc(sizeof(struct linx_host_parameter) * midi_in_count);
	in_audios = (struct linx_host_parameter*)malloc(sizeof(struct linx_host_parameter) * audio_in_count);
	out_audios = (struct linx_host_parameter*)malloc(sizeof(struct linx_host_parameter) * audio_out_count);

	for (i = 0 ; i < subgraph->propagated_pin_count; i++) {
		struct linx_pin_ref* pref = &subgraph->propagated_pins[i];
		struct linx_pin* p = linx_graph_definition_resolve_pin(subgraph, i, &resolved_vertdef, &resolved_pin_index);

		if (p->pin_type == linx_pin_type_in_scalar_float || p->pin_type == linx_pin_type_in_scalar_int) {
			in_parameters[param_in_index].extra_index = i;
			in_parameters[param_in_index].name = pref->name;
			param_in_index++;
		} else if (p->pin_type == linx_pin_type_in_midi) {
			in_midis[midi_in_index].extra_index = i;
			in_midis[midi_in_index].name = pref->name;
			midi_in_index++;
		} else if (p->pin_type == linx_pin_type_in_buffer_float) {
			in_audios[audio_in_index].extra_index = i;
			in_audios[audio_in_index].name = pref->name;
			audio_in_index++;
		} else if (p->pin_type == linx_pin_type_out_buffer_float) {
			out_audios[audio_out_index].extra_index = i;
			out_audios[audio_out_index].name = pref->name;
			audio_out_index++;
		}
	}

	*result_param_in_count = param_in_count;
	*result_in_pins = in_parameters;
	*result_audio_in_count = audio_in_count;
	*result_in_audios = in_audios;
	*result_audio_out_count = audio_out_count;
	*result_out_audios = out_audios;
	*result_midi_in_count = midi_in_count;
	*result_in_midis = in_midis;
}
