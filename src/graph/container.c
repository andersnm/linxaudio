#include <stdlib.h>
#include <string.h>
#include "linxaudio.h"

void container_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	for (i = 0; i < self->subgraph->graph->propagated_pin_count; i++) {
		if (self->subgraph->propagated_pins[i].pin_type == linx_pin_type_in_buffer_float) {
			struct linx_buffer* buffer = linx_graph_instance_get_propagated_pin_buffer(self->subgraph, i);
			linx_buffer_copy_chunk(buffer, &propagated_pin_buffers[i], 0, 0, sample_count);
		}
	}

	linx_vertex_instance_process_subgraph(self->subgraph, self, in_values, out_values, sample_count);

	for (i = 0; i < self->subgraph->graph->propagated_pin_count; i++) {
		if (self->subgraph->propagated_pins[i].pin_type == linx_pin_type_out_buffer_float) {
			struct linx_buffer* buffer = linx_graph_instance_get_propagated_pin_buffer(self->subgraph, i);
			linx_buffer_copy_chunk(&propagated_pin_buffers[i], buffer, 0, 0, sample_count);
		}
	}

	linx_graph_instance_process_clear(self->subgraph);
}

void container_create(struct linx_vertex_instance* plugin, int samplerate) {
	plugin->data = 0;
}

void container_destroy(struct linx_vertex_instance* plugin) {
}

struct linx_factory container_factory = {
	sizeof(struct linx_factory), "Container", linx_factory_flag_is_subgraph_parent,
	0, 0,
	0, 0,
	container_create, container_destroy, container_process
};

struct linx_factory* container_get_factory() {
	return &container_factory;
}
