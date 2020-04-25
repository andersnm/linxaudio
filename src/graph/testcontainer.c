#include <stdlib.h>
#include <string.h>
#include "linxaudio.h"

struct linx_pin container_pins[] = {
	{ "Left In", linx_pin_type_in_buffer_float },
	{ "Right In", linx_pin_type_in_buffer_float },
	{ "Left Out", linx_pin_type_out_buffer_float },
	{ "Right Out", linx_pin_type_out_buffer_float },
};

struct linx_pin container_subgraph_pins[] = {
	{ "SubLeft In", linx_pin_type_in_buffer_float },
	{ "SubRight In", linx_pin_type_in_buffer_float },
	{ "SubLeft Out", linx_pin_type_out_buffer_float },
	{ "SubRight Out", linx_pin_type_out_buffer_float },
};

struct container {
	//float out_buffer[2][linx_max_buffer_size];
	float _;
};

void container_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_value_array* out_values, int sample_count) {

	// as the container iplementer, we are responsible for setting up the invalues required by the subgraph
	// and mixing the subgraph outvalues with the container outvalues afterwards
	int i;

	struct container* data = (struct container*)self->data;
	struct linx_value subgraph_in_value_data[max_value_queue_per_vertex];
	struct linx_value subgraph_out_value_data[max_value_queue_per_vertex];
	struct linx_value_array subgraph_in_values = { subgraph_in_value_data, 0, max_value_queue_per_vertex };
	struct linx_value_array subgraph_out_values = { subgraph_out_value_data, 0, max_value_queue_per_vertex };
	struct linx_buffer* subgraph_buffers = self->subgraph_pin_buffers;

	// containers param_index 0 -> "Left In", send container input to the subgraph param_index 2 "SubLeft Out"
	// containers param_index 1 -> "Right In", send container input to the subgraph param_index 3 "SubRight Out"
	// also propagate extra param values from this graph targeted at any vertices in the subgraph
	
	// in the containers subgraph, it is allowed to create edges to and from a special "PARENT" module,
	// using a special value -1 for the vertex index. these edges are connected to or from any of the 
	// "subgraph_pins".

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_propagated) {
			// pass on to subgraph!
			linx_value_array_push_value(&subgraph_in_values, v->pin_index, v->pin_group, v, v->timestamp);
		}
	}

	// copy pin->subout
	// self->vertex_data->subgraph_pin_buffers ; // always proper to use as 0-based, wheras pin_buffers are not proper because of potential chunk-offset
	if (pin_buffers[0].write_count > 0) {
		linx_buffer_write(&subgraph_buffers[2], pin_buffers[0].float_buffer, sample_count);
	}

	if (pin_buffers[1].write_count > 0) {
		linx_buffer_write(&subgraph_buffers[3], pin_buffers[1].float_buffer, sample_count);
	}

	linx_vertex_instance_process_subgraph(self->subgraph, self, &subgraph_in_values, &subgraph_out_values, sample_count);

	// copy subin->pin
	if (subgraph_buffers[0].write_count > 0) {
		linx_buffer_write(&pin_buffers[2], subgraph_buffers[0].float_buffer, sample_count);
	}

	if (subgraph_buffers[1].write_count > 0) {
		linx_buffer_write(&pin_buffers[3], subgraph_buffers[1].float_buffer, sample_count);
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
}

void container_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct container* container = (struct container*)malloc(sizeof(struct container));
	memset(container, 0, sizeof(struct container));
	plugin->data = container;
}

void container_destroy(struct linx_vertex_instance* plugin) {
	free(plugin->data);
}

struct linx_factory container_factory = {
	sizeof(struct linx_factory), "Container", linx_factory_flag_is_subgraph_parent,
	(sizeof(container_pins) / sizeof(struct linx_pin)), container_pins,
	(sizeof(container_subgraph_pins) / sizeof(struct linx_pin)), container_subgraph_pins,
	container_create, container_destroy, container_process
};

struct linx_factory* container_get_factory() {
	return &container_factory;
}
