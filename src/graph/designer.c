#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "linxaudio.h"
#include "commonhost.h"

struct linx_vertex_definition* graph_set_vertex(struct linx_graph_definition* graph, int vertex_index, struct linx_factory* factory, struct linx_graph_definition* subgraph, int init_value_count) {
	struct linx_vertex_definition* vertex = &graph->vertices[vertex_index];
	vertex->factory = factory;
	vertex->subgraph = subgraph;
	vertex->init_value_count = init_value_count;
	vertex->init_values = (struct linx_value*)malloc(sizeof(struct linx_value) * init_value_count);
	memset(vertex->init_values, 0, sizeof(struct linx_value) * init_value_count);
	return vertex;
}

void graph_set_vertex_init_int(struct linx_vertex_definition* vertex, int value_index, int pin_index, int value) {
	vertex->init_values[value_index].pin_index = pin_index;
	vertex->init_values[value_index].intvalue = value;
}

void graph_set_vertex_init_float(struct linx_vertex_definition* vertex, int value_index, int pin_index, float value) {
	vertex->init_values[value_index].pin_index = pin_index;
	vertex->init_values[value_index].floatvalue = value;
}

struct linx_edge_definition* graph_set_edge(struct linx_graph_definition* graph, int edge_index, int to_vertex, int to_pin_index, enum linx_pin_group to_pin_group, int from_vertex, int from_pin_index, enum linx_pin_group from_pin_group) {

	struct linx_edge_definition* edge = &graph->edges[edge_index];
	edge->to_vertex = to_vertex;
	edge->to_pin_index = to_pin_index;
	edge->to_pin_group = to_pin_group;
	edge->from_vertex = from_vertex;
	edge->from_pin_index = from_pin_index;
	edge->from_pin_group = from_pin_group;

	//printf("%i %i %i %i %i %i\n", to_vertex, to_pin_index, to_pin_group, from_vertex, from_pin_index, from_pin_group);
	return edge;
}

struct linx_pin_ref* graph_set_propagated_pin(struct linx_graph_definition* graph, int pin_index, const char* name, int vertex, int prop_pin_index, enum linx_pin_group prop_pin_group) {
	struct linx_pin_ref* pin = &graph->propagated_pins[pin_index];
	pin->name = name;
	pin->vertex = vertex;
	pin->pin_index = prop_pin_index;
	pin->pin_group = prop_pin_group;
	return pin;
}

struct linx_graph_definition* linx_graph_definition_create(int vertex_count, int edge_count, int propagated_pins) {
	struct linx_graph_definition* graph = (struct linx_graph_definition*)malloc(sizeof(struct linx_graph_definition));
	graph->vertex_count = vertex_count;
	graph->vertices = (struct linx_vertex_definition*)malloc(sizeof(struct linx_vertex_definition) * vertex_count);
	memset(graph->vertices, 0, sizeof(struct linx_vertex_definition) * vertex_count);

	graph->edge_count = edge_count;
	graph->edges = (struct linx_edge_definition*)malloc(sizeof(struct linx_edge_definition) * edge_count);
	memset(graph->edges, 0, sizeof(struct linx_edge_definition) * edge_count);

	graph->propagated_pins = (struct linx_pin_ref*)malloc(sizeof(struct linx_pin_ref) * propagated_pins);
	graph->propagated_pin_count = propagated_pins;
	return graph;
}

void linx_graph_definition_destroy(struct linx_graph_definition* graph) {
	free(graph->vertices);
	free(graph->edges);
	free(graph->propagated_pins);
	free(graph);
}

// javascript access functions

struct linx_pin_ref* linx_graph_definition_get_pinref(struct linx_graph_definition* graph, int index) {
	return &graph->propagated_pins[index];
}

int linx_graph_definition_get_pinref_count(struct linx_graph_definition* graph) {
	return graph->propagated_pin_count;
}

const char* linx_pinref_get_name(struct linx_pin_ref* pinref) {
	return pinref->name;
}

int linx_pinref_get_vertex(struct linx_pin_ref* pinref) {
	return pinref->vertex;
}

int linx_pinref_get_index(struct linx_pin_ref* pinref) {
	return pinref->pin_index;
}

enum linx_pin_group linx_pinref_get_group(struct linx_pin_ref* pinref) {
	return pinref->pin_group;
}

linx_create_functype* linx_factory_get_create(struct linx_factory* factory) {
	return factory->create;
}

linx_destroy_functype* linx_factory_get_destroy(struct linx_factory* factory) {
	return factory->destroy;
}

int linx_factory_get_pin_count(struct linx_factory* factory) {
	return factory->pin_count;
}

struct linx_pin* linx_factory_get_pin(struct linx_factory* factory, int index) {
	return &factory->pins[index];
}

int linx_factory_is_subgraph_parent(struct linx_factory* factory) {
	return (factory->flags & linx_factory_flag_is_subgraph_parent) != 0;
}

int linx_factory_get_subgraph_pin_count(struct linx_factory* factory) {
	return factory->subgraph_pin_count;
}

struct linx_pin* linx_factory_get_subgraph_pin(struct linx_factory* factory, int index) {
	return &factory->subgraph_pins[index];
}

const char* linx_pin_get_name(struct linx_pin* pin) {
	return pin->name;
}

enum linx_pin_type linx_pin_get_type(struct linx_pin* pin) {
	return pin->pin_type;
}

float linx_pin_get_min(struct linx_pin* pin) {
	return pin->min_value;
}

float linx_pin_get_max(struct linx_pin* pin) {
	return pin->max_value;
}

float linx_pin_get_default(struct linx_pin* pin) {
	return pin->default_value;
}

float linx_pin_get_precision(struct linx_pin* pin) {
	return pin->precision;
}

int linx_value_get_pin_index(struct linx_value* value) {
	return value->pin_index;
}

enum linx_pin_group linx_value_get_pin_group(struct linx_value* value) {
	return value->pin_group;
}

/*float* linx_value_get_floatbuffer(struct linx_value* value) {
	return value->floatbuffer;
}*/

struct linx_value* linx_valuearray_item(struct linx_value* values, int index) {
	return &values[index];
}

struct linx_host_parameter* linx_host_parameters_item(struct linx_host_parameter* values, int index) {
	return &values[index];
}

const char* linx_host_parameter_get_name(struct linx_host_parameter* value) {
	return value->name;
}

int linx_host_parameter_get_pin_index(struct linx_host_parameter* value) {
	return value->extra_index;
}

struct linx_pin* linx_instance_get_pin(struct linx_graph_instance* instance, int index) {
	return &instance->propagated_pins[index];
}

void linx_graph_instance_process_vertex_value(struct linx_graph_instance* instance, int vertex_index, struct linx_value* value) {
	// quick access to preview init value

	struct linx_vertex_instance* vertex = &instance->vertex_data[vertex_index];
	linx_value_array_push_value(vertex->in_values, value->pin_index, value->pin_group, value, 0);
	
	// not sure why it would try to deliver directly as below.. performance?
	// if we send it like above, the propagateds will be processed by e.g stereocontainer

/*	if (value->pin_group == linx_pin_group_module) {
		int outcount = 0;
		linx_value_array_push_value(vertex->in_values, value->pin_index, value->pin_group, value, 0);
	} else if (value->pin_group == linx_pin_group_propagated) {
		struct linx_pin_ref* pinref = &vertex->subgraph->graph->propagated_pins[value->pin_index];
		value->pin_group = pinref->pin_group;
		value->pin_index = pinref->pin_index;
		linx_graph_instance_process_vertex_value(vertex->subgraph, pinref->vertex, value);
	} else {
		assert(0);
	}*/
}

void linx_graph_instance_process_vertex_float(struct linx_graph_instance* instance, int vertex_index, int pin_index, enum linx_pin_group pin_group, float floatvalue) {
	struct linx_value value;
	value.floatvalue = floatvalue;
	value.pin_index = pin_index;
	value.pin_group = pin_group;
	value.timestamp = 0;
	linx_graph_instance_process_vertex_value(instance, vertex_index, &value);
}

void linx_graph_instance_process_vertex_int(struct linx_graph_instance* instance, int vertex_index, int pin_index, enum linx_pin_group pin_group, int intvalue) {
	struct linx_value value;
	value.intvalue = intvalue;
	value.pin_index = pin_index;
	value.pin_group = pin_group;
	value.timestamp = 0;
	linx_graph_instance_process_vertex_value(instance, vertex_index, &value);
}

void linx_graph_instance_process_vertex_midi(struct linx_graph_instance* instance, int vertex_index, int pin_index, enum linx_pin_group pin_group, unsigned int midi) {
	struct linx_value value;
	value.midimessage = midi;
	value.pin_index = pin_index;
	value.pin_group = pin_group;
	value.timestamp = 0;
	linx_graph_instance_process_vertex_value(instance, vertex_index, &value);
}

struct linx_vertex_instance* linx_graph_instance_get_vertex_instance(struct linx_graph_instance* instance, int vertex_index) {
	return &instance->vertex_data[vertex_index];
}

struct linx_graph_instance* linx_vertex_instance_get_subgraph(struct linx_vertex_instance* vertex) {
	return vertex->subgraph ;
}

struct linx_vertex_definition* linx_graph_definition_get_vertex_definition(struct linx_graph_definition* graph, int vertex_index) {
	return &graph->vertices[vertex_index];
}

void linx_graph_set_propagated_buffer(struct linx_graph_instance* instance, int pin_index, float* buffer, int sample_count) {
	struct linx_buffer* propagated_buffer = linx_graph_instance_get_propagated_pin_buffer(instance, pin_index);
	assert(propagated_buffer != 0);
	linx_buffer_write(propagated_buffer, buffer, sample_count);
}

void linx_graph_get_propagated_buffer(struct linx_graph_instance* instance, int pin_index, float* buffer, int sample_count) {
	struct linx_buffer* propagated_buffer = linx_graph_instance_get_propagated_pin_buffer(instance, pin_index);
	assert(propagated_buffer != 0);
	linx_buffer_read(buffer, propagated_buffer, sample_count);
}

int linx_graph_has_propagated_buffer(struct linx_graph_instance* instance, int pin_index) {
	struct linx_buffer* propagated_buffer = linx_graph_instance_get_propagated_pin_buffer(instance, pin_index);
	assert(propagated_buffer != 0);

	return propagated_buffer->write_count > 0;
}

struct linx_value* linx_graph_instance_get_current_value(struct linx_graph_instance* instance, int pin_index) {

	struct linx_vertex_instance* resolved_vertdata;
	int resolved_pin_index;
		
	struct linx_pin* resolved_pin = linx_graph_instance_resolve_pin(instance, pin_index, &resolved_vertdata, &resolved_pin_index);
	assert(resolved_pin != 0);

	return &resolved_vertdata->last_in_values[resolved_pin_index];
}

float linx_graph_instance_get_current_value_float(struct linx_graph_instance* instance, int pin_index) {
	struct linx_value* value = linx_graph_instance_get_current_value(instance, pin_index);
	if (!value) {
		return 0;
	}

	return value->floatvalue;
}

int linx_graph_instance_get_current_value_int(struct linx_graph_instance* instance, int pin_index) {
	struct linx_value* value = linx_graph_instance_get_current_value(instance, pin_index);
	if (!value) {
		return 0;
	}

	return value->intvalue;
}
