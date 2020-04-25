#include <stdlib.h>
#include <string.h>
#include "linxaudio.h"

/*
	stereo container: clone subgraph and process two input buffer pins separately. provide means for stereo spread
*/

struct linx_pin stereocontainer_pins[] = {
	{ "Left In", linx_pin_type_in_buffer_float },
	{ "Right In", linx_pin_type_in_buffer_float },
	{ "Left Out", linx_pin_type_out_buffer_float },
	{ "Right Out", linx_pin_type_out_buffer_float },
};

struct linx_pin stereocontainer_subgraph_pins[] = {
	{ "Sub In", linx_pin_type_in_buffer_float },
	{ "Sub Out", linx_pin_type_out_buffer_float },
	{ "Is Right", linx_pin_type_out_scalar_float, 0.0f, 1.0f, 0.0f, 1.0f }
};

struct channel {
	struct linx_graph_instance* subgraph;
	struct linx_value subgraph_in_value_data[max_value_queue_per_vertex];
	struct linx_value_array subgraph_in_values;
};

struct stereocontainer {
	struct channel channels[2];
};

void stereocontainer_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {

	// as the container iplementer, we are responsible for setting up the invalues required by the subgraph
	// and mixing the subgraph outvalues with the container outvalues afterwards
	int i;

	struct stereocontainer* data = (struct stereocontainer*)self->data;
	struct linx_value subgraph_out_value_data[max_value_queue_per_vertex];
	struct linx_value_array subgraph_out_values = { subgraph_out_value_data, 0, max_value_queue_per_vertex };
	struct linx_buffer* subgraph_buffers = self->subgraph_pin_buffers; 

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_propagated) {
			// pass on to subgraphs!
			linx_value_array_push_value(&data->channels[0].subgraph_in_values, v->pin_index, v->pin_group, v, v->timestamp);
			linx_value_array_push_value(&data->channels[1].subgraph_in_values, v->pin_index, v->pin_group, v, v->timestamp);
		}
	}

	// LEFT SIDE: copy LeftInPin->SubOut
	if (pin_buffers[0].write_count > 0) {
		linx_buffer_write(&subgraph_buffers[1], pin_buffers[0].float_buffer, sample_count);
	}

	// process left channel
	linx_vertex_instance_process_subgraph(data->channels[0].subgraph, self, &data->channels[0].subgraph_in_values, &subgraph_out_values, sample_count);

	// copy SubIn to LeftOutPin
	if (subgraph_buffers[0].write_count > 0) {
		linx_buffer_write(&pin_buffers[2], subgraph_buffers[0].float_buffer, sample_count);
	}
	
	// reset
	subgraph_buffers[0].write_count = 0;
	subgraph_buffers[1].write_count = 0;

	// RIGHT SIDE: copy RightInPin->SubOut
	if (pin_buffers[1].write_count > 0) {
		linx_buffer_write(&subgraph_buffers[1], pin_buffers[1].float_buffer, sample_count);
	}

	// process right channel
	linx_vertex_instance_process_subgraph(data->channels[1].subgraph, self, &data->channels[1].subgraph_in_values, &subgraph_out_values, sample_count);

	// copy SubIn to RightOutPin
	if (subgraph_buffers[0].write_count > 0) {
		linx_buffer_write(&pin_buffers[3], subgraph_buffers[0].float_buffer, sample_count);
	}

	// subgraph param_index 0 -> "SubLeftIn", send subgraph output to the containers param_index 2 "Left Out"
	// subgraph param_index 1 -> "SubRightIn", send subgraph output to the containers param_index 3 "Right Out"
	// also propagate extra param values from the subgraph vertices to this graph
	for (i = 0; i < subgraph_out_values.length; i++) {
		struct linx_value* v = &subgraph_out_values.items[i];
		if (v->pin_group == linx_pin_group_propagated) {
			linx_value_array_push_value(out_values, v->pin_index, v->pin_group, v, v->timestamp);
		}
	}

	self->subgraph_pin_buffers[1].write_count = 0;

	data->channels[0].subgraph_in_values.length = 0;
	linx_graph_instance_process_clear(data->channels[0].subgraph);
	data->channels[1].subgraph_in_values.length = 0;
	linx_graph_instance_process_clear(data->channels[1].subgraph);

}

void stereocontainer_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct stereocontainer* container = (struct stereocontainer*)malloc(sizeof(struct stereocontainer));
	memset(container, 0, sizeof(struct stereocontainer));
	plugin->data = container;

	container->channels[0].subgraph = plugin->subgraph;
	container->channels[1].subgraph = linx_graph_definition_create_instance(plugin->subgraph->graph, samplerate);
	linx_value_array_init_from(&container->channels[0].subgraph_in_values, container->channels[0].subgraph_in_value_data, 0, max_value_queue_per_vertex);
	linx_value_array_init_from(&container->channels[1].subgraph_in_values, container->channels[1].subgraph_in_value_data, 0, max_value_queue_per_vertex);

	// set "Is Right" to 1.0 on the second channel, and 0.0 on the first channel (TODO: only once during create)
	linx_value_array_push_float(&container->channels[0].subgraph_in_values, 2, linx_pin_group_module, 0.0f, 0);
	linx_value_array_push_float(&container->channels[1].subgraph_in_values, 2, linx_pin_group_module, 1.0f, 0);

}

void stereocontainer_destroy(struct linx_vertex_instance* plugin) {
	struct stereocontainer* data = (struct stereocontainer*)plugin->data;
	linx_graph_instance_destroy(data->channels[1].subgraph);
	free(plugin->data);
}

struct linx_factory stereocontainer_factory = {
	sizeof(struct linx_factory), "StereoContainer", linx_factory_flag_is_subgraph_parent,
	(sizeof(stereocontainer_pins) / sizeof(struct linx_pin)), stereocontainer_pins,
	(sizeof(stereocontainer_subgraph_pins) / sizeof(struct linx_pin)), stereocontainer_subgraph_pins,
	stereocontainer_create, stereocontainer_destroy, stereocontainer_process
};

struct linx_factory* stereocontainer_get_factory() {
	return &stereocontainer_factory;
}
