#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mathext.h"
#include "linxaudio.h"

#if defined(_MSC_VER) && _MSC_VER < 1200

float fminf(float a, float b) {
	return (((a)<(b))?(a):(b));
}

float fmaxf(float a, float b) {
	return (((a)>(b))?(a):(b));
}

#endif

void dsp_mix(float* dest, float* src, int sample_count) {
	int i;
	for (i = 0; i < sample_count; i++) {
		dest[i] += src[i];
	}
}

void dsp_mix_amp(float* dest, float* src, int sample_count, float amp) {
	int i;
	for (i = 0; i < sample_count; i++) {
		dest[i] += src[i] * amp;
	}
}

void dsp_copy_amp(float* dest, float* src, int sample_count, float amp) {
	int i;
	for (i = 0; i < sample_count; i++) {
		dest[i] = src[i] * amp;
	}
}

unsigned int midi_make(unsigned char channel, unsigned char command, unsigned char data1, unsigned char data2) {
	unsigned int message = ((unsigned int)data2 << 16) | ((unsigned int)data1 << 8) | (((unsigned int)command & 0x0f) << 4) | ((unsigned int)channel & 0x0f);
	return message;
}

void midi_parse(unsigned int message, unsigned short* status, unsigned char* channel, unsigned char* command, unsigned char* data1, unsigned char* data2) {
	*status = message & 0xff;
	*channel = message & 0x0f;
	*command = (message & 0xf0) >> 4;
	*data1 = (message >> 8) & 0xff;
	*data2 = (message >> 16) & 0xff;
}

int linx_pin_is_buffer(enum linx_pin_type type) {
	switch (type) {
		case linx_pin_type_in_buffer_float:
		case linx_pin_type_out_buffer_float:
			return 1;
		default:
			return 0;
	}
}

int linx_pin_is_in(enum linx_pin_type type) {
	switch (type) {
		case linx_pin_type_in_scalar_int:
		case linx_pin_type_in_scalar_float:
		case linx_pin_type_in_buffer_float:
		case linx_pin_type_in_midi:
			return 1;
		default:
			return 0;
	}
}

int linx_pin_is_out(enum linx_pin_type type) {
	return !linx_pin_is_in(type);
}

void linx_clear_buffers(struct linx_buffer* buffers, struct linx_pin* pins, int pin_count) {
	int i;
	for (i = 0; i < pin_count; i++) {
		struct linx_pin* p = &pins[i];
		switch (p->pin_type ) {
			case linx_pin_type_out_buffer_float:
			case linx_pin_type_in_buffer_float:
				buffers[i].write_count = 0;
				break;
			default: 
				break; // OK - ignore
		}
	}
}

void linx_allocate_buffers(struct linx_buffer* buffers, struct linx_pin* pins, int pin_count) {
	int i;
	for (i = 0; i < pin_count; i++) {
		struct linx_pin* p = &pins[i];
		switch (p->pin_type ) {
			case linx_pin_type_out_buffer_float:
			case linx_pin_type_in_buffer_float:
				buffers[i].float_buffer = (float*)malloc(sizeof(float) * linx_max_buffer_size);
				memset(buffers[i].float_buffer, 0, sizeof(float) * linx_max_buffer_size);
				break;
			default: 
				break; // OK - ignore
		}
	}
}

void linx_free_buffers(struct linx_buffer* buffers, struct linx_pin* pins, int pin_count) {
	int i;
	for (i = 0; i < pin_count; i++) {
		struct linx_pin* p = &pins[i];
		switch (p->pin_type ) {
			case linx_pin_type_out_buffer_float:
			case linx_pin_type_in_buffer_float:
				free(buffers[i].float_buffer);
				break;
			default: 
				break; // OK - ignore
		}
	}
}

int linxp_vertdeps_get_zero_count(int* vertdeps, int vertex_count) {
	int result = 0;
	int i;
	for (i = 0; i < vertex_count; i++) {
		if (vertdeps[i] == 0) result++;
	}
	return result;
}

int* linxp_vertdeps_create(struct linx_graph_definition* graph) {
	int i;
	int vertex_count = graph->vertex_count;
	int* vertdeps = (int*)malloc(sizeof(int) * vertex_count);

	memset(vertdeps, 0, sizeof(int) * vertex_count);
	for (i = 0; i < graph->edge_count; i++) {
		struct linx_edge_definition* e = &graph->edges[i];
		if (e->to_vertex == linx_parent_graph_vertex_id || e->from_vertex == linx_parent_graph_vertex_id) {
			continue;
		}
		vertdeps[e->to_vertex]++;
	}

	return vertdeps;
}

void linxp_vertdeps_adjust(struct linx_graph_definition* graph, int* vertdeps, int* zeros, int zero_count) {
	int i, j;
	for (i = 0; i < graph->edge_count; i++) {
		struct linx_edge_definition* e = &graph->edges[i];
		for (j = 0; j < zero_count; j++) {
			if (e->to_vertex == linx_parent_graph_vertex_id || e->from_vertex == linx_parent_graph_vertex_id) {
				continue;
			}
			if (e->from_vertex == zeros[j]) {
				vertdeps[e->to_vertex]--;
			}
		}
	}
}

int linxp_get_processing_layer_count(struct linx_graph_definition* graph) {
	int i;
	int zeros[max_vertices_per_layer];
	int done_count;
	int zero_index;
	int result;
	int vertex_count = graph->vertex_count;
	int* vertdeps = linxp_vertdeps_create(graph);

	result = 0;
	while (1) {
		done_count = 0;
		zero_index = 0;
		for (i = 0; i < vertex_count; i++) {
			if (vertdeps[i] == -1) {
				done_count++;
			} else if (vertdeps[i] == 0) {
				assert(zero_index < max_vertices_per_layer);
				vertdeps[i]--;
				zeros[zero_index] = i;
				zero_index++;
			}
		}

		linxp_vertdeps_adjust(graph, vertdeps, zeros, zero_index);
		if (done_count == vertex_count) {
			break;
		}
		result++;
	}

	free(vertdeps);
	return result;
}

void linx_processing_order_create(struct linx_graph_definition* graph, struct linx_processing_order* result) {
	int i;
	int* zeros;
	int zero_index, zero_count;
	int vertex_count = graph->vertex_count;
	int layer_count = linxp_get_processing_layer_count(graph);
	int layer_index = 0;
	int* vertdeps = linxp_vertdeps_create(graph);

	result->layer_count = layer_count;
	result->layers = (struct linx_processing_layer*)malloc(sizeof(struct linx_processing_layer) * layer_count);

	while (1) {
		zero_index = 0;
		zero_count = linxp_vertdeps_get_zero_count(vertdeps, vertex_count);
		if (zero_count == 0) {
			break;
		}

		zeros = (int*)malloc(sizeof(int) * zero_count);
		for (i = 0; i < vertex_count; i++) {
			if (vertdeps[i] == 0) {
				vertdeps[i]--;
				zeros[zero_index] = i;
				zero_index++;
			}
		}

		linxp_vertdeps_adjust(graph, vertdeps, zeros, zero_count);
		result->layers[layer_index].vertex_count = zero_count;
		result->layers[layer_index].vertices = zeros;
		layer_index++;
	}

	free(vertdeps);

}

void linx_processing_order_free(struct linx_processing_order* result) {
	int i;
	for (i = 0; i < result->layer_count; i++) {
		struct linx_processing_layer* layer = &result->layers[i];
		free(layer->vertices);
	}
	free(result->layers);
}

float linx_get_init_value_float(struct linx_value* init_values, int init_value_count, int pin_index, enum linx_pin_group pin_group, float default_value) {
	int i;
	for (i = 0; i < init_value_count; i++) {
		if (init_values[i].pin_index == pin_index && init_values[i].pin_group == pin_group) {
			return init_values[i].floatvalue;
		}
	}

	return default_value;
}

int linx_get_init_value_int(struct linx_value* init_values, int init_value_count, int pin_index, enum linx_pin_group pin_group, int default_value) {
	int i;
	for (i = 0; i < init_value_count; i++) {
		if (init_values[i].pin_index == pin_index && init_values[i].pin_group == pin_group) {
			return init_values[i].intvalue; 
		}
	}

	return default_value; 
}

void linxp_init_current_value(struct linx_pin* pin, int pin_index, enum linx_pin_group pin_group, struct linx_value* init_values, int init_value_count, struct linx_value_array* in_values) {
	float floatvalue;
	int intvalue;
	switch (pin->pin_type) {
		case linx_pin_type_in_scalar_float:
			floatvalue = linx_get_init_value_float(init_values, init_value_count, pin_index, linx_pin_group_module, pin->default_value);
			linx_value_array_push_float(in_values, pin_index, pin_group, floatvalue, 0);
			break;
		case linx_pin_type_in_scalar_int:
			intvalue = linx_get_init_value_int(init_values, init_value_count, pin_index, linx_pin_group_module, (int)pin->default_value);
			linx_value_array_push_int(in_values, pin_index, pin_group, intvalue, 0);
			break;
		default: 
			break; // OK - ignore
	}
}

void linx_buffer_write(struct linx_buffer* to_buffer, float* srcbuffer, int sample_count) {
	assert(srcbuffer != 0);
	assert(to_buffer->float_buffer != 0);
	if (to_buffer->write_count == 0) {
		memcpy(to_buffer->float_buffer, srcbuffer, sizeof(float) * sample_count);
	} else {
		dsp_mix(to_buffer->float_buffer, srcbuffer, sample_count);
	}
	to_buffer->write_count ++;
}

void linx_buffer_read(float* destbuffer, struct linx_buffer* srcbuffer, int sample_count) {
	assert(destbuffer != 0);
	assert(srcbuffer->float_buffer != 0);
	if (srcbuffer->write_count == 0) {
		memset(destbuffer, 0, sizeof(float) * sample_count);
	} else {
		memcpy(destbuffer, srcbuffer->float_buffer, sizeof(float) * sample_count);
	}
}

void linx_buffer_copy_chunk(struct linx_buffer* dest_buffer, struct linx_buffer* src_buffer, int dest_offset, int src_offset, int sample_count) {
	if (src_buffer->write_count == 0) {
		// if already wrote something, memset 0 for this chunk
		if (dest_buffer->write_count > 0) {
			memset(dest_buffer->float_buffer + dest_offset, 0, sizeof(float) * sample_count);
		}
	} else if (src_buffer->write_count > 0) {
		if (dest_buffer->write_count == 0 && dest_offset > 0) {
			// clear start of buffer if first chunk of samples comes after a chunk of silence
			memset(dest_buffer->float_buffer, 0, sizeof(float) * dest_offset);
		}

		memcpy(dest_buffer->float_buffer + dest_offset, src_buffer->float_buffer + src_offset, sizeof(float) * sample_count);
		dest_buffer->write_count = 1;
	}
}

struct linx_buffer* linx_graph_instance_get_propagated_pin_buffer(struct linx_graph_instance* instance, int pin_index) {
	struct linx_pin_ref* pinref = &instance->graph->propagated_pins[pin_index];
	switch (pinref->pin_group) {
		case linx_pin_group_module:
			return &instance->vertex_data[pinref->vertex].pin_buffers[pinref->pin_index];
			break;
		case linx_pin_group_propagated:
			return &instance->vertex_data[pinref->vertex].propagated_pin_buffers[pinref->pin_index];
		default:
			assert(0);
			return 0;
	}
}

float linx_vertex_definition_get_pin_default_init_value(struct linx_vertex_definition* vertdef, int pin_index) {
	// override propagated pin default value with vertex init value or pin default value
	struct linx_pin* pin = &vertdef->factory->pins[pin_index];
	switch (pin->pin_type) {
		case linx_pin_type_in_scalar_int:
			return linx_get_init_value_int(vertdef->init_values, vertdef->init_value_count, pin_index, linx_pin_group_module, pin->default_value);
		case linx_pin_type_in_scalar_float:
			return linx_get_init_value_float(vertdef->init_values, vertdef->init_value_count, pin_index, linx_pin_group_module, pin->default_value);
		default:
			return 0.0f;
	}
}

struct linx_graph_instance* linx_graph_definition_create_instance(struct linx_graph_definition* graph, int samplerate) {
	int i, j;
	struct linx_graph_instance* result;
	int in_value_count = 0;

	result = (struct linx_graph_instance*)malloc(sizeof(struct linx_graph_instance));
	result->graph = graph;
	result->vertex_data = (struct linx_vertex_instance*)malloc(sizeof(struct linx_vertex_instance) * graph->vertex_count);
	result->edge_data = (struct linx_edge_instance*)malloc(sizeof(struct linx_edge_instance) * graph->edge_count);
	result->propagated_pins = (struct linx_pin*)malloc(sizeof(struct linx_pin) * graph->propagated_pin_count);

	for (i = 0; i < graph->propagated_pin_count; i++) {
		struct linx_vertex_definition* resolved_vertdef;
		int resolved_pin_index;
		struct linx_pin* pin = linx_graph_definition_resolve_pin(graph, i, &resolved_vertdef, &resolved_pin_index);
		assert(pin != 0);
		result->propagated_pins[i] = *pin;

		// override propagated pin default value with vertex init value or pin default value
		result->propagated_pins[i].default_value = linx_vertex_definition_get_pin_default_init_value(resolved_vertdef, resolved_pin_index);
	}

	linx_processing_order_create(graph, &result->processing_order);

	for (i = 0; i < graph->vertex_count; i++) {
		struct linx_vertex_definition* v = &graph->vertices[i];
		struct linx_vertex_instance* pv = &result->vertex_data[i];

		if (v->subgraph != NULL) {
			pv->subgraph = linx_graph_definition_create_instance(v->subgraph, samplerate);
		} else {
			pv->subgraph = NULL;
		}

		pv->vertex = v;
		pv->in_values = linx_value_array_create(max_value_queue_per_vertex);
		pv->out_values = linx_value_array_create(max_value_queue_per_vertex);
		pv->subgraph_in_values = linx_value_array_create(max_value_queue_per_vertex);
		pv->subgraph_out_values = linx_value_array_create(max_value_queue_per_vertex);

		pv->last_in_values = (struct linx_value*)malloc(sizeof(struct linx_value) * v->factory->pin_count);

		// allocate buffers for buffer pins
		pv->pin_buffers = (struct linx_buffer*)malloc(sizeof(struct linx_buffer) * v->factory->pin_count);
		memset(pv->pin_buffers, 0, sizeof(struct linx_buffer) * v->factory->pin_count);

		linx_allocate_buffers(pv->pin_buffers, v->factory->pins, v->factory->pin_count);

		if (v->subgraph != NULL) {
			pv->propagated_pin_buffers = (struct linx_buffer*)malloc(sizeof(struct linx_buffer) * v->subgraph->propagated_pin_count);
			memset(pv->propagated_pin_buffers, 0, sizeof(struct linx_buffer) * v->subgraph->propagated_pin_count);

			linx_allocate_buffers(pv->propagated_pin_buffers, pv->subgraph->propagated_pins, v->subgraph->propagated_pin_count);
		}

		// queue default values
		in_value_count = 0;
		for (j = 0; j < v->factory->pin_count; j++) {
			struct linx_pin* p = &v->factory->pins[j];
			linxp_init_current_value(p, j, linx_pin_group_module, v->init_values, v->init_value_count, pv->in_values);
		}

		pv->subgraph_pin_buffers = (struct linx_buffer*)malloc(sizeof(struct linx_buffer) * v->factory->subgraph_pin_count);
		memset(pv->subgraph_pin_buffers, 0, sizeof(struct linx_buffer) * v->factory->subgraph_pin_count);
		linx_allocate_buffers(pv->subgraph_pin_buffers, v->factory->subgraph_pins, v->factory->subgraph_pin_count);

		// call module factory provided initialisation code
		v->factory->create(pv, samplerate);

	}

	result->snapshot = linx_graph_instance_create_snapshot(result, samplerate);

	return result;
}

void linx_graph_instance_destroy(struct linx_graph_instance* graph) {
	int i;
	for (i = 0; i < graph->graph->vertex_count; i++) {
		struct linx_vertex_instance* vertex_instance = &graph->vertex_data[i];
		struct linx_vertex_definition* vertex_definition = &graph->graph->vertices[i];

		if (vertex_instance->subgraph) {
			linx_graph_instance_destroy(vertex_instance->subgraph);
		}

		vertex_definition->factory->destroy(vertex_instance);

		linx_value_array_free(vertex_instance->in_values);
		linx_value_array_free(vertex_instance->out_values);
		linx_value_array_free(vertex_instance->subgraph_in_values);
		linx_value_array_free(vertex_instance->subgraph_out_values);
		free(vertex_instance->last_in_values);

		linx_free_buffers(vertex_instance->pin_buffers, vertex_definition->factory->pins, vertex_definition->factory->pin_count);
		linx_free_buffers(vertex_instance->subgraph_pin_buffers, vertex_definition->factory->subgraph_pins, vertex_definition->factory->subgraph_pin_count);
		free(vertex_instance->pin_buffers);
		free(vertex_instance->subgraph_pin_buffers);
	}
	
	linx_processing_order_free(&graph->processing_order);
	linx_graph_instance_free_snapshot(graph);

	free(graph->vertex_data);
	free(graph->edge_data);
	free(graph->propagated_pins);
	free(graph);
}

void get_edge_vertex_pin(struct linx_graph_instance* graph, struct linx_vertex_instance* context, int vertex, int pin_index, enum linx_pin_group pin_group, struct linx_vertex_instance** result_vertexdata, struct linx_pin** result_pin, struct linx_value_array** result_in_queue, struct linx_value_array** result_out_queue, struct linx_buffer** result_buffer) {
	struct linx_vertex_instance* to_vertexdata;
	struct linx_pin* to_pin;
	struct linx_value_array* to_in_queue;
	struct linx_value_array* to_out_queue;
	struct linx_buffer* buffer;

	if (vertex == linx_parent_graph_vertex_id) {
		assert(pin_group == linx_pin_group_module);
		to_vertexdata = context;
		assert(pin_index < to_vertexdata->vertex->factory->subgraph_pin_count);
		to_pin = &to_vertexdata->vertex->factory->subgraph_pins[pin_index];
		to_in_queue = to_vertexdata->subgraph_out_values; // note: subgraph in/ou are swapped, because they work opposite, in some way
		to_out_queue = to_vertexdata->subgraph_in_values;
		buffer = &to_vertexdata->subgraph_pin_buffers[pin_index];
	} else {
		to_vertexdata = &graph->vertex_data[vertex];
		if (pin_group == linx_pin_group_module) {
			assert(pin_index < to_vertexdata->vertex->factory->pin_count);
			to_pin = &to_vertexdata->vertex->factory->pins[pin_index];
			buffer = &to_vertexdata->pin_buffers[pin_index];
		} else if (pin_group == linx_pin_group_propagated) {
			assert(pin_index < to_vertexdata->subgraph->graph->propagated_pin_count);
			to_pin = &to_vertexdata->subgraph->propagated_pins[pin_index];
			buffer = &to_vertexdata->propagated_pin_buffers[pin_index];
		} else {
			assert(0);
		}
		to_in_queue = to_vertexdata->in_values;
		to_out_queue = to_vertexdata->out_values;
	}

	*result_vertexdata = to_vertexdata;
	*result_pin = to_pin;
	*result_in_queue = to_in_queue;
	*result_out_queue = to_out_queue;
	*result_buffer = buffer;
	assert((buffer == 0 || buffer->float_buffer == 0) || linx_pin_is_buffer(to_pin->pin_type));
}

// edge processors: one function for each kind of connection
// void process_edge_XX_YY(...)
//     XX = from pin type
//     YY = to pin type
// XX/YY: 
//     fb = float-buffer (removed)
//     ib = int-buffer
//     fs = float-scalar
//     is = int-scalar
//     m = midi

// to=from: float-scalar = float-scalar
void process_edge_fs_fs(struct linx_graph_instance* graph, struct linx_vertex_instance* context, int to_pin_index, enum linx_pin_group to_pin_group, struct linx_value_array* to_values, int from_pin_index, enum linx_pin_group from_pin_group, struct linx_value_array* from_values, int sample_count) {
	int i;
	for (i = 0; i < from_values->length; i++) {
		if (from_values->items[i].pin_index == from_pin_index && from_values->items[i].pin_group == from_pin_group && from_values->items[i].timestamp >= 0) {
			linx_value_array_push_float(to_values, to_pin_index, to_pin_group, from_values->items[i].floatvalue, from_values->items[i].timestamp);
		}
	}
}

// to=from: int-scalar = float-scalar
void process_edge_is_fs(struct linx_graph_instance* graph, struct linx_vertex_instance* context, int to_pin_index, enum linx_pin_group to_pin_group, struct linx_value_array* to_values, int from_pin_index, enum linx_pin_group from_pin_group, struct linx_value_array* from_values, int sample_count) {
	int i;
	for (i = 0; i < from_values->length; i++) {
		if (from_values->items[i].pin_index == from_pin_index && from_values->items[i].pin_group == from_pin_group && from_values->items[i].timestamp >= 0) {
			linx_value_array_push_int(to_values, to_pin_index, to_pin_group, (int)from_values->items[i].floatvalue, from_values->items[i].timestamp);
		}
	}
}
// to=from: float-scalar = int-scalar
void process_edge_fs_is(struct linx_graph_instance* graph, struct linx_vertex_instance* context, int to_pin_index, enum linx_pin_group to_pin_group, struct linx_value_array* to_values, int from_pin_index, enum linx_pin_group from_pin_group, struct linx_value_array* from_values, int sample_count) {
	int i;
	for (i = 0; i < from_values->length; i++) {
		if (from_values->items[i].pin_index == from_pin_index && from_values->items[i].pin_group == from_pin_group && from_values->items[i].timestamp >= 0) {
			linx_value_array_push_float(to_values, to_pin_index, to_pin_group, from_values->items[i].intvalue, from_values->items[i].timestamp);
		}
	}
}

// to=from: int-scalar = int-scalar
void process_edge_is_is(struct linx_graph_instance* graph, struct linx_vertex_instance* context, int to_pin_index, enum linx_pin_group to_pin_group, struct linx_value_array* to_values, int from_pin_index, enum linx_pin_group from_pin_group, struct linx_value_array* from_values, int sample_count) {
	int i;
	for (i = 0; i < from_values->length; i++) {
		if (from_values->items[i].pin_index == from_pin_index && from_values->items[i].pin_group == from_pin_group && from_values->items[i].timestamp >= 0) {
			linx_value_array_push_int(to_values, to_pin_index, to_pin_group, from_values->items[i].intvalue, from_values->items[i].timestamp);
		}
	}
}

// to=from: midi = midi
void process_edge_midi_midi(struct linx_graph_instance* graph, struct linx_vertex_instance* context, int to_pin_index, enum linx_pin_group to_pin_group, struct linx_value_array* to_values, int from_pin_index, enum linx_pin_group from_pin_group, struct linx_value_array* from_values, int sample_count) {
	int i;
	for (i = 0; i < from_values->length; i++) {
		if (from_values->items[i].pin_index == from_pin_index && from_values->items[i].pin_group == from_pin_group && from_values->items[i].timestamp >= 0) {
			linx_value_array_push_midi(to_values, to_pin_index, to_pin_group, from_values->items[i].midimessage, from_values->items[i].timestamp);
		}
	}
}

void linxp_graph_process_edge(struct linx_graph_instance* graph, struct linx_vertex_instance* context, struct linx_edge_definition* edge, int sample_count) {
	struct linx_vertex_instance* to_vertexdata;
	struct linx_pin* to_pin;
	struct linx_vertex_definition* from_vertex;
	struct linx_vertex_instance* from_vertexdata;
	struct linx_pin* from_pin;
	struct linx_value_array* from_in_queue;
	struct linx_value_array* from_out_queue;
	struct linx_value_array* to_in_queue;
	struct linx_value_array* to_out_queue;
	struct linx_buffer* to_buffer;
	struct linx_buffer* from_buffer;

	get_edge_vertex_pin(graph, context, edge->to_vertex, edge->to_pin_index, edge->to_pin_group, &to_vertexdata, &to_pin, &to_in_queue, &to_out_queue, &to_buffer);
	get_edge_vertex_pin(graph, context, edge->from_vertex, edge->from_pin_index, edge->from_pin_group, &from_vertexdata, &from_pin, &from_in_queue, &from_out_queue, &from_buffer);

	switch (from_pin->pin_type) {
		case linx_pin_type_out_buffer_float:
			switch (to_pin->pin_type) {
				case linx_pin_type_in_buffer_float:
					if (from_buffer->write_count > 0) {
						linx_buffer_write(to_buffer, from_buffer->float_buffer, sample_count);
					}
					break;
				default:
					assert(0);
					break;
			}
			break;
		case linx_pin_type_out_scalar_float:
			switch (to_pin->pin_type) {
				case linx_pin_type_in_scalar_float:
					process_edge_fs_fs(graph, context, edge->to_pin_index, edge->to_pin_group, to_in_queue, edge->from_pin_index, edge->from_pin_group, from_out_queue, sample_count);
					break;
				case linx_pin_type_in_scalar_int:
					process_edge_is_fs(graph, context, edge->to_pin_index, edge->to_pin_group, to_in_queue, edge->from_pin_index, edge->from_pin_group, from_out_queue, sample_count);
					break;
				default:
					assert(0);
					break;
			}
			break;
		case linx_pin_type_out_scalar_int:
			switch (to_pin->pin_type) {
				case linx_pin_type_in_scalar_float:
					process_edge_fs_is(graph, context, edge->to_pin_index, edge->to_pin_group, to_in_queue, edge->from_pin_index, edge->from_pin_group, from_out_queue, sample_count);
					break;
				case linx_pin_type_in_scalar_int:
					process_edge_is_is(graph, context, edge->to_pin_index, edge->to_pin_group, to_in_queue, edge->from_pin_index, edge->from_pin_group, from_out_queue, sample_count);
					break;
				default:
					assert(0);
					break;
			}
			break;
		case linx_pin_type_out_midi:
			switch (to_pin->pin_type) {
				case linx_pin_type_in_midi:
					process_edge_midi_midi(graph, context, edge->to_pin_index, edge->to_pin_group, to_in_queue, edge->from_pin_index, edge->from_pin_group, from_out_queue, sample_count);
					break;
				default:
					assert(0);
					break;
			}
			break;
		default:
			assert(0);
			break;
	}
}

void linxp_graph_process_to_edges(struct linx_graph_instance* graph, struct linx_vertex_instance* context, int vertex_index, int sample_count) {
	int i;
	for (i = 0; i < graph->graph->edge_count; i++) {
		struct linx_edge_definition* edge = &graph->graph->edges[i];
		struct linx_edge_instance* edgedata = &graph->edge_data[i];
		if (edge->to_vertex == vertex_index) {
			linxp_graph_process_edge(graph, context, edge, sample_count);
		}
	}
}

void copy_pin_buffers(struct linx_buffer* dest_buffers, struct linx_buffer* src_buffers, struct linx_pin* pins, int pin_count, int chunk_offset) {
	int i;
	for (i = 0; i < pin_count; i++) {
		struct linx_pin* pin = &pins[i];
		switch (pin->pin_type) {
			case linx_pin_type_in_buffer_float:
				if (src_buffers[i].write_count > 0) {
					dest_buffers[i].float_buffer = src_buffers[i].float_buffer + chunk_offset;
					dest_buffers[i].write_count = 1;
				} else {
					dest_buffers[i].float_buffer = 0;
					dest_buffers[i].write_count = 0;
				}
				break;
			case linx_pin_type_out_buffer_float:
				dest_buffers[i].float_buffer = src_buffers[i].float_buffer + chunk_offset;
				dest_buffers[i].write_count = 0;
				break;
			default:
				dest_buffers[i].float_buffer = 0;
				dest_buffers[i].write_count = 0;
				break;
		}
	}
}

void update_pin_buffers(struct linx_buffer* dest_buffers, struct linx_buffer* src_buffers, struct linx_pin* pins, int pin_count, int chunk_offset, int sample_count) {
	int i;
	for (i = 0; i < pin_count; i++) {
		struct linx_pin* pin = &pins[i];
		switch (pin->pin_type) {
			case linx_pin_type_out_buffer_float:
				linx_buffer_copy_chunk(&dest_buffers[i], &src_buffers[i], chunk_offset, 0, sample_count);
				break;
			default:
				break;
		}
	}
}

void linxp_graph_process_vertex(struct linx_graph_instance* graph, struct linx_vertex_instance* context, int vertex, int sample_count) {
	int i, j;
	struct linx_vertex_definition* vert;
	struct linx_vertex_instance* vertdata;
	struct linx_pin* param;
	struct linx_value chunk_in_value_data[max_value_queue_per_vertex];
	struct linx_value chunk_out_value_data[max_value_queue_per_vertex];
	struct linx_value_array chunk_in_values = { chunk_in_value_data, 0, max_value_queue_per_vertex };
	struct linx_value_array chunk_out_values = { chunk_out_value_data, 0, max_value_queue_per_vertex };
	
	vert = &graph->graph->vertices[vertex];
	vertdata = &graph->vertex_data[vertex];

	// enumerate incoming edges: in audio, in midi, in values
	linxp_graph_process_to_edges(graph, context, vertex, sample_count);

	// now the input pin buffers on this plugin are mixed!
	// also incoming events fom previous layer are queued in the plugin

	// set last_in_values from in_values, only where timestamps are less than sample_count
	linx_value_array_set_last_values(vertdata->last_in_values, vertdata->in_values, vert->factory->pin_count, sample_count);

	if ((vert->factory->flags & linx_factory_flag_is_timestamp_aware) != 0) {
		// if the plugin supports timestamps, process the whole chunk at once
		linx_value_array_copy_in_values(&chunk_in_values, vertdata->in_values, sample_count);

		vert->factory->process(vertdata, &chunk_in_values, vertdata->pin_buffers, vertdata->propagated_pin_buffers, vertdata->out_values, sample_count);

		linx_value_array_shift_values(vertdata->in_values, sample_count);
	} else {
		// if the plugin doesnt support timestamps, process each interval between timestamps as a chunk.
		// timestamps and buffers are adjusted such that the plugin always processes relative to the chunk.
		// in_values to propagated pins do not affect the chunk size, only scalars and midi on this module.
		int chunk_offset = 0;
		int chunk_sample_count;
		struct linx_buffer pin_buffers[1024]; // 1024 = anticipated max number of pins
		struct linx_buffer propagated_pin_buffers[1024];

		while (1) {
			chunk_in_values.length = 0;
			chunk_out_values.length = 0;

			// NOTE: vertdata->in_values' timestamps are relative to the chunk, because linxp_shift_values() after each chunk
			
			chunk_sample_count = linx_value_array_get_next_timestamp(vertdata->in_values, sample_count - chunk_offset);

			linx_value_array_copy_in_values(&chunk_in_values, vertdata->in_values, chunk_sample_count);

			copy_pin_buffers(pin_buffers, vertdata->pin_buffers, vert->factory->pins, vert->factory->pin_count, chunk_offset);
			if (vert->subgraph != NULL) {
				copy_pin_buffers(propagated_pin_buffers, vertdata->propagated_pin_buffers, vertdata->subgraph->propagated_pins, vert->subgraph->propagated_pin_count, chunk_offset);
			}

			vert->factory->process(vertdata, &chunk_in_values, pin_buffers, propagated_pin_buffers, &chunk_out_values, chunk_sample_count);

			update_pin_buffers(vertdata->pin_buffers, pin_buffers, vert->factory->pins, vert->factory->pin_count, chunk_offset, chunk_sample_count);
			if (vert->subgraph != NULL) {
				update_pin_buffers(vertdata->propagated_pin_buffers, propagated_pin_buffers, vertdata->subgraph->propagated_pins, vert->subgraph->propagated_pin_count, chunk_offset, chunk_sample_count);
			}

			linx_value_array_shift_values(vertdata->in_values, chunk_sample_count);

			// copy chunk_out_values to out_values with adjusted tmestamps
			for (i = 0; i < chunk_out_values.length; i++) {
				struct linx_value* value = &chunk_out_values.items[i];
				linx_value_array_push_value(vertdata->out_values, value->pin_index, value->pin_group, value, value->timestamp + chunk_offset);
			}

			chunk_offset += chunk_sample_count;
			if (chunk_offset == sample_count) {
				break;
			}
		}
	}
}

void linx_vertex_instance_process_subgraph(struct linx_graph_instance* subgraphinst, struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_value_array* out_values, int sample_count) {
	int i;
	struct linx_value process_in_value_data[max_value_queue_per_vertex];
	struct linx_value_array process_in_values = { process_in_value_data, 0, max_value_queue_per_vertex };

	if (subgraphinst == 0) {
		return ;
	}

	// process_subgraph() is invoked by a module instance with a subgraph inside it.
	// process_subgraph() expects "subgraph in_pins" and extra pin values in in_values
	// process_subgraph() returns "subgraph out_pins" and extra pin values in out_values

	// process_subgraph() calls process_graph(), after some pin transformations:
	//   1. copy extra in_values into process_in_values[]
	//   2. copy subgraph in_values into self->subgraph_in_values
	//   3. call process_graph(process_in_values, out_values);
	//   4. (extra out_values were added to out_values in process_graph)
	//   5. copy self->subgraph_out_values to out_values
	//   6. -> the module instance can extract subgraph results from the out_values

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* value = &in_values->items[i];
		if (value->pin_group == linx_pin_group_propagated) {
			linx_value_array_push_value(&process_in_values, value->pin_index, value->pin_group, value, value->timestamp);
		} else {
			linx_value_array_push_value(self->subgraph_in_values, value->pin_index, value->pin_group, value, value->timestamp);
		}
	}
	
	// graph_process() expects only extra pin values in in_values
	// graph_process() returns only extra pin values in out_values
	linx_graph_instance_process(subgraphinst, self, &process_in_values, out_values, sample_count);

	// add values from subgraph connections to the out_values, extra out_values are already added
	for (i = 0; i < self->subgraph_out_values->length; i++) {
		struct linx_value* value = &self->subgraph_out_values->items[i];
		if (value->timestamp >= 0) {
			linx_value_array_push_value(out_values, value->pin_index, value->pin_group, value, value->timestamp);
		}
	}
	// clear subgraph_out_values so modules can call process_subgraph again without reusing old outputs
	self->subgraph_out_values->length = 0;
	self->subgraph_in_values->length = 0;
}

void linx_graph_instance_process_clear(struct linx_graph_instance* graph) {
	int i, j;
		
	for (i = 0; i < graph->graph->vertex_count; i++) {
		struct linx_vertex_definition* vert;
		struct linx_vertex_instance* vertdata;

		vert = &graph->graph->vertices[i];
		vertdata = &graph->vertex_data[i];
		linx_clear_buffers(vertdata->pin_buffers, vert->factory->pins, vert->factory->pin_count);
		linx_clear_buffers(vertdata->subgraph_pin_buffers, vert->factory->subgraph_pins, vert->factory->subgraph_pin_count);

		if (vert->subgraph != NULL) {
			linx_clear_buffers(vertdata->propagated_pin_buffers, vertdata->subgraph->propagated_pins, vert->subgraph->propagated_pin_count);
		}

		// at this point, 
		// - invalues are shifted as they are parsed
		// - outvalues have been handed off to all target vertices
		// - subgraph in/out values have been processed and done with in their respective vertex implementations
		vertdata->out_values->length = 0;
		vertdata->subgraph_in_values->length = 0;
		vertdata->subgraph_out_values->length = 0;
	}
}

void linx_graph_instance_process(struct linx_graph_instance* graph, struct linx_vertex_instance* context, struct linx_value_array* in_values, struct linx_value_array* out_values, int sample_count) {
	int i, j;
	int vertex_index;

	// graph_process() expects only extra pin values in in_values
	// graph_process() returns only extra pin values in out_values

	// resolve extra pin values relative to this graph and set pin
	// value in the module. extra pins pointing at extra pins in 
	// the subgraph will be resolved later, recursively.
	for (i = 0; i < in_values->length; i++) {
		struct linx_pin* pin;
		struct linx_pin_ref* pref;
		struct linx_vertex_instance* vdata;
		struct linx_value* value = &in_values->items[i];
		assert(value->pin_group == linx_pin_group_propagated);
		assert(value->timestamp >= 0);

		pin = &graph->propagated_pins[value->pin_index];
		assert(!linx_pin_is_buffer(pin->pin_type));

		pref = &graph->graph->propagated_pins[value->pin_index];
		vdata = &graph->vertex_data[pref->vertex];
		linx_value_array_push_value(vdata->in_values, pref->pin_index, pref->pin_group, value, value->timestamp);
	}

	// the processing order is layered: 
	//     the first layer has all leaf machines such as generators, etc,
	//     the second layer has machines that depend on the first layer, 
	//     the third layer has machines that depend on the second layer, etc.

	for (i = 0; i < graph->processing_order.layer_count; i++) {
		// each layer is processed for all samples in the frame before
		// processing the next layer etc. as long as the previous layer is
		// completely processed, with all timestamped events and buffers
		// queued up, the current layer can be chunked freely per vertex, 
		// with chunk sizes depending on plugins flags like quantization, 
		// emission interrupt rate, and ability to handle timestamps
		// (the nodes on each layer can also run in parallell)
		for (j = 0; j < graph->processing_order.layers[i].vertex_count; j++) {
			vertex_index = graph->processing_order.layers[i].vertices[j];
			linxp_graph_process_vertex(graph, context, vertex_index, sample_count);
		}
	}

	// enumerate edges to parent subgraph = those with -1 for target
	linxp_graph_process_to_edges(graph, context, -1, sample_count);

	//out_values->length = 0;

	// propagate extra pins from this to parent graph through out_values
	for (i = 0; i < graph->graph->propagated_pin_count; i++) {
		struct linx_pin* pin = &graph->propagated_pins[i];
		struct linx_pin_ref* pref = &graph->graph->propagated_pins[i];
		struct linx_vertex_instance* vdata = &graph->vertex_data[pref->vertex];

		for(j = 0; j < vdata->out_values->length; j++) {
			if (vdata->out_values->items[j].timestamp >= 0 && vdata->out_values->items[j].pin_index == pref->pin_index && vdata->out_values->items[j].pin_group == pref->pin_group) {
				linx_value_array_push_value(out_values, i, linx_pin_group_propagated, &vdata->out_values->items[j], vdata->out_values->items[j].timestamp);
			}
		}
	}

	linx_graph_instance_update_snapshot(graph, context, sample_count);
}

struct linx_pin* linx_vertex_definition_resolve_pin(struct linx_vertex_definition* v, int pin_index, int pin_group, struct linx_vertex_definition** result_vertdef, int* result_pin_index) {
	struct linx_pin_ref* subpref;

	if (pin_group == linx_pin_group_module) {
		*result_vertdef = v;
		*result_pin_index = pin_index;
		return &v->factory->pins[pin_index];
	} else if (pin_group == linx_pin_group_propagated) {
		subpref = &v->subgraph->propagated_pins[pin_index];
		return linx_vertex_definition_resolve_pin(&v->subgraph->vertices[subpref->vertex], subpref->pin_index, subpref->pin_group, result_vertdef, result_pin_index);
	}
	assert(0);
	return 0;
}

struct linx_pin* linx_graph_definition_resolve_pin(struct linx_graph_definition* graph, int pin_index, struct linx_vertex_definition** result_vertdef, int* result_pin_index) {
	struct linx_pin_ref* pinref;
	struct linx_vertex_definition* vert;
	pinref = &graph->propagated_pins[pin_index];
	vert = &graph->vertices[pinref->vertex];
	return linx_vertex_definition_resolve_pin(vert, pinref->pin_index, pinref->pin_group, result_vertdef, result_pin_index);
}

struct linx_pin* linx_vertex_instance_resolve_pin(struct linx_vertex_instance* vertdata, int pin_index, enum linx_pin_group pin_group, struct linx_vertex_instance** result_vertdata, int* result_pin_index) {
	struct linx_pin_ref* subpref;

	if (pin_group == linx_pin_group_module) {
		*result_vertdata = vertdata;
		*result_pin_index = pin_index;
		return &vertdata->vertex->factory->pins[pin_index];
	} else if (pin_group == linx_pin_group_propagated) {
		subpref = &vertdata->vertex->subgraph->propagated_pins[pin_index];
		return linx_vertex_instance_resolve_pin(&vertdata->subgraph->vertex_data[subpref->vertex], subpref->pin_index, subpref->pin_group, result_vertdata, result_pin_index);
	}
	assert(0);
	return 0;
}

struct linx_pin* linx_graph_instance_resolve_pin(struct linx_graph_instance* graphdata, int pin_index, struct linx_vertex_instance** result_vertdata, int* result_pin_index) {
	struct linx_pin_ref* pinref = &graphdata->graph->propagated_pins[pin_index];
	struct linx_vertex_instance* vertdata = &graphdata->vertex_data[pinref->vertex];
	return linx_vertex_instance_resolve_pin(vertdata, pinref->pin_index, pinref->pin_group, result_vertdata, result_pin_index);
}

/*
void verify_x87_stack_empty() {
	unsigned i;
    unsigned z[8];
    __asm {
        fldz
        fldz
        fldz
        fldz
        fldz
        fldz
        fldz
        fldz
        fstp dword ptr [z+0x00]
        fstp dword ptr [z+0x04]
        fstp dword ptr [z+0x08]
        fstp dword ptr [z+0x0c]
        fstp dword ptr [z+0x10]
        fstp dword ptr [z+0x14]
        fstp dword ptr [z+0x18]
        fstp dword ptr [z+0x1c]
    }

    // Verify bit patterns. 0 = 0.0
    for (i = 0; i < 8; ++i) {
        assert(z[i] == 0);
    }
}*/


void linx_value_array_init_from(struct linx_value_array* values, struct linx_value* ptr, int length, int capacity) {
	values->items = ptr;
	values->length = length;
	values->capacity = capacity;
}

struct linx_value_array* linx_value_array_create(int capacity) {
	struct linx_value_array* result = (struct linx_value_array*)malloc(sizeof(struct linx_value_array) + sizeof(struct linx_value) * capacity);
	result->items = (struct linx_value*)(result + 1);
	result->length = 0;
	result->capacity = capacity;
	return result;
}

struct linx_value* linx_value_array_get(struct linx_value_array* values, int index) {
	assert(index < values->length);
	return &values->items[index];
}

void linx_value_array_push_value(struct linx_value_array* values, int pin_index, enum linx_pin_group pin_group, struct linx_value* value, int timestamp) {
	struct linx_value* item = &values->items[values->length];
	assert(values->length >= 0 && values->length < values->capacity);
	*item = *value;
	item->pin_group = pin_group;
	item->pin_index = pin_index;
	item->timestamp = timestamp;
	values->length++;
}

void linx_value_array_push_float(struct linx_value_array* values, int pin_index, enum linx_pin_group pin_group, float value, int timestamp) {
	struct linx_value* item = &values->items[values->length];
	assert(values->length >= 0 && values->length < values->capacity);
	item->floatvalue = value;
	item->pin_group = pin_group;
	item->pin_index = pin_index;
	item->timestamp = timestamp;
	values->length++;
}

void linx_value_array_push_int(struct linx_value_array* values, int pin_index, enum linx_pin_group pin_group, int value, int timestamp) {
	struct linx_value* item = &values->items[values->length];
	assert(values->length >= 0 && values->length < values->capacity);
	item->intvalue = value;
	item->pin_group = pin_group;
	item->pin_index = pin_index;
	item->timestamp = timestamp;
	values->length++;
}

void linx_value_array_push_midi(struct linx_value_array* values, int pin_index, enum linx_pin_group pin_group, unsigned int value, int timestamp) {
	struct linx_value* item = &values->items[values->length];
	assert(values->length >= 0 && values->length < values->capacity);
	item->midimessage = value;
	item->pin_group = pin_group;
	item->pin_index = pin_index;
	item->timestamp = timestamp;
	values->length++;
}


void linx_value_array_copy_in_values(struct linx_value_array* dest_values, struct linx_value_array* src_values, int sample_count) {
	int i;
	int chunk_next_offset = sample_count;
	struct linx_value* value;
	for (i = 0; i < src_values->length; i++) {
		value = &src_values->items[i];

		// NOTE: including module pins AT chunk, extra pins IN chunk:
		if (value->timestamp == 0 || (value->pin_group == linx_pin_group_propagated && value->timestamp >= 0 && value->timestamp < chunk_next_offset)) {
			linx_value_array_push_value(dest_values, value->pin_index, value->pin_group, value, value->timestamp);
		}
	}
}

void linx_value_array_set_last_values(struct linx_value* last_values, struct linx_value_array* new_values, int pin_count, int sample_count) {
	int i;

	// 1. set all last_timestamp to 0, so we can compare last timestamps with new timestamps in next step
	for (i = 0; i < pin_count; i++) {
		if (last_values[i].timestamp >= 0) {
			last_values[i].timestamp = 0;
		}
	}

	// 2. copy new_value to last_value if new_timestamp >= last_timestamp && new_timestamp < sample_count
	for (i = 0; i < new_values->length; i++) {
		struct linx_value* value = &new_values->items[i];
		if (value->timestamp != -1 && value->timestamp < sample_count) {
			if (value->pin_group == linx_pin_group_module) {
				struct linx_value* last_value = &last_values[value->pin_index];
				assert(value->pin_index < pin_count);
				if (value->timestamp >= last_value->timestamp) {
					*last_value = *value;
				}
			}
		}
	}
}

int linx_value_array_get_next_timestamp(struct linx_value_array* values, int sample_count) {
	int i;
	int chunk_next_offset = sample_count;
	struct linx_value* value;

	for (i = 0; i < values->length; i++) {
		value = &values->items[i];
		if (value->timestamp == -1) {
			continue;
		}
		if (value->timestamp > 0 && value->timestamp < chunk_next_offset && value->pin_group == linx_pin_group_module) {
			chunk_next_offset = value->timestamp;
		}
	}
	return chunk_next_offset;
}

void linx_value_array_shift_values(struct linx_value_array* values, int sample_count) {
	int i;
	int pos = 0;
	int deleted = 0;
	for (i = 0; i < values->length; i++) {
		struct linx_value* value = &values->items[i];
			
		if (value->timestamp >= 0) {
			if (value->timestamp < sample_count) {
				memset(value, 0, sizeof(struct linx_value));
				value->timestamp = -1;
				deleted ++;
			} else {
				if (pos != i) {
					values->items[pos] = *value;
					values->items[pos].timestamp -= sample_count;
					memset(value, 0, sizeof(struct linx_value));
					value->timestamp = -1;
				} else {
					value->timestamp -= sample_count;
				}
				pos++;
			}
		}
	}
	values->length -= deleted;
}

void linx_value_array_free(struct linx_value_array* values) {
	free(values);
}


// describe arbitrary module pin in a graf or subgraf
void linx_graph_instance_describe_vertex_value(struct linx_graph_instance* instance, int vertex_index, int pin_index, enum linx_pin_group pin_group, float value, char* name_result, int name_length) {
	struct linx_vertex_instance* vertinst;

	assert(name_length > 0);
	strcpy(name_result, "");

	vertinst = &instance->vertex_data[vertex_index];
	if (pin_group == linx_pin_group_module) {
		if (vertinst->vertex->factory->describe_value) {
			vertinst->vertex->factory->describe_value(vertinst, pin_index, value, name_result, name_length);
		}
	} else if (pin_group == linx_pin_group_propagated) {
		struct linx_pin_ref* pinref = &vertinst->subgraph->graph->propagated_pins[pin_index];
		linx_graph_instance_describe_vertex_value(vertinst->subgraph, pinref->vertex, pinref->pin_index, pinref->pin_group, value, name_result, name_length);
	} else {
		assert(0);
		name_result[0] = 0;
	}
}

// describe propagated parameter on a graf
void linx_graph_instance_describe_value(struct linx_graph_instance* instance, int pin_index, float value, char* name_result, int name_length) {
	// assume prop pins on the graf instance
	struct linx_pin_ref* pinref = &instance->graph->propagated_pins[pin_index];
	linx_graph_instance_describe_vertex_value(instance, pinref->vertex, pinref->pin_index, pinref->pin_group, value, name_result, name_length);
}


struct linx_snapshot* linx_graph_instance_create_snapshot(struct linx_graph_instance* instance, int samplerate) {
	struct linx_snapshot* result = (struct linx_snapshot*)malloc(sizeof(struct linx_snapshot));
	result->samplerate = samplerate;
	result->position = 0;
	result->buffer = (float*)malloc( linx_edge_digest_size * instance->graph->edge_count * sizeof(float) );
	memset(result->buffer, 0, linx_edge_digest_size * instance->graph->edge_count * sizeof(float));
	return result;
}

void linx_graph_instance_free_snapshot(struct linx_graph_instance* instance) {
	free(instance->snapshot->buffer);
	free(instance->snapshot);
}

struct linx_value* get_nearest_value(struct linx_value_array* values, int pin_index, enum linx_pin_group pin_group, int timestamp, int sample_count) {
	int i;
	struct linx_value* result = 0;
	for (i = 0; i < values->length; i++) {
		struct linx_value* value = &values->items[i];
		if (value->pin_index == pin_index && value->pin_group == pin_group && 
				value->timestamp >= timestamp && value->timestamp < (timestamp + sample_count) &&
				(result == 0 || value->timestamp > result->timestamp) ) {
			result = value;
		}
	}
	return result;
}

float get_nearest_float(struct linx_value_array* values, int pin_index, enum linx_pin_group pin_group, int timestamp, int sample_count, float default_value) {
	struct linx_value* value = get_nearest_value(values, pin_index, pin_group, timestamp, sample_count);
	if (value) {
		return value->floatvalue;
	} else {
		return default_value;
	}
}

float get_nearest_int(struct linx_value_array* values, int pin_index, enum linx_pin_group pin_group, int timestamp, int sample_count, float default_value) {
	struct linx_value* value = get_nearest_value(values, pin_index, pin_group, timestamp, sample_count);
	if (value) {
		return value->intvalue;
	} else {
		return default_value;
	}
}

void linx_graph_instance_update_snapshot(struct linx_graph_instance* instance, struct linx_vertex_instance* context, int sample_count) {
	// sample values in output wires - call this at end of process (before clearing)

	// PROBLEM/TODO: the out value queues are always copied entirely into the 
	// input value queue, and can have events beyond this timestamp (f.ex midi
	// echo or delay). these values won't be polled unless the out values are 
	// also shifted, similar to the in values.

	int i;
	int update_samples;
	int position;
	int skip_samples = (int)(instance->snapshot->samplerate / linx_edge_digest_rate);

	for (i = 0; i < instance->graph->edge_count; i++) {
		struct linx_edge_definition* edge = &instance->graph->edges[i];
		struct linx_vertex_instance* from_vertexdata;
		struct linx_pin* from_pin;
		struct linx_value_array* from_in_queue;
		struct linx_value_array* from_out_queue;
		struct linx_buffer* from_buffer;

		get_edge_vertex_pin(instance, context, edge->from_vertex, edge->from_pin_index, edge->from_pin_group, &from_vertexdata, &from_pin, &from_in_queue, &from_out_queue, &from_buffer);
		
		update_samples = 0;
		position = instance->snapshot->position;
		while (update_samples < sample_count) {

			int skip_position = position % skip_samples;
			int chunk_length = (int)fminf((float)(skip_samples - skip_position), (float)(sample_count - update_samples));
			float value;
			int digest_offset;
			int prev_digest_offset;
			if (skip_position == 0) {
				digest_offset = (position / skip_samples) % linx_edge_digest_size;
				prev_digest_offset = (digest_offset + linx_edge_digest_size - 1) % linx_edge_digest_size;
			} else {
				digest_offset = (position / skip_samples) % linx_edge_digest_size;
				prev_digest_offset = digest_offset;
			}

			switch (from_pin->pin_type) {
				case linx_pin_type_out_buffer_float:
					if (skip_position == 0) {
						if (from_buffer->write_count > 0) {
							value = from_buffer->float_buffer[update_samples];
						} else {
							value = 0;
						}
						instance->snapshot->buffer[i * linx_edge_digest_size + digest_offset] = value;
					}
					break;
				case linx_pin_type_out_scalar_float:
					// reuse get last digested sample if thres no nearest value
					value = instance->snapshot->buffer[i * linx_edge_digest_size + prev_digest_offset];
					value = get_nearest_float(from_out_queue, edge->from_pin_index, edge->from_pin_group, update_samples, chunk_length, value);
					instance->snapshot->buffer[i * linx_edge_digest_size + digest_offset] = value;
					break;
				case linx_pin_type_out_scalar_int:
					value = instance->snapshot->buffer[i * linx_edge_digest_size + prev_digest_offset];
					value = get_nearest_int(from_out_queue, edge->from_pin_index, edge->from_pin_group, update_samples, chunk_length, value);
					instance->snapshot->buffer[i * linx_edge_digest_size + digest_offset] = value;
					break;
				default:
					value = 0;
					instance->snapshot->buffer[i * linx_edge_digest_size + digest_offset] = value;
					break;
			}

			update_samples += chunk_length;
			position += chunk_length;
		}
	}
	instance->snapshot->position += sample_count;
}

float* linx_graph_definition_get_snapshot(struct linx_graph_instance* instance) {
	return instance->snapshot->buffer;
}
