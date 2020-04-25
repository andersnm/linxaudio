struct linx_host_parameter {
	int extra_index;
	const char* name;
};

void linx_host_get_pins(struct linx_graph_definition* subgraph, int* result_param_in_count, struct linx_host_parameter** result_in_parameters, int* result_audio_in_count, struct linx_host_parameter** result_in_audios, int* result_audio_out_count, struct linx_host_parameter** result_out_audios, int* result_midi_in_count, struct linx_host_parameter** result_in_midis);
struct linx_graph_definition* linx_host_get_graph();
