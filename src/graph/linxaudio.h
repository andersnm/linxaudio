#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#define inline __inline
#endif

/* internal audio graph definitions */

#define linx_parent_graph_vertex_id -1
#define max_vertices_per_layer 1024
#define max_value_queue_per_vertex 256
#define linx_max_buffer_size 256
#define linx_edge_digest_size 32	// length of edge digest
#define linx_edge_digest_rate 1024.0f	// edge samples per sec

enum linx_pin_group {
	linx_pin_group_module,
	linx_pin_group_propagated,
};

enum linx_pin_type {
	linx_pin_type_in_scalar_float,
	linx_pin_type_out_scalar_float,
	linx_pin_type_in_scalar_int,
	linx_pin_type_out_scalar_int,
	linx_pin_type_in_midi,
	linx_pin_type_out_midi,
	linx_pin_type_in_buffer_float,
	linx_pin_type_out_buffer_float,
};

struct linx_vertex_definition {
	struct linx_factory* factory;
	struct linx_graph_definition* subgraph;
	int init_value_count;
	struct linx_value* init_values;
};

struct linx_edge_definition {
	int from_vertex;
	enum linx_pin_group from_pin_group;
	int from_pin_index;
	int to_vertex;
	enum linx_pin_group to_pin_group;
	int to_pin_index;
};

struct linx_pin_ref {
	int vertex;
	enum linx_pin_group pin_group;
	int pin_index;
	const char* name;
};

struct linx_graph_definition {
	int vertex_count;
	struct linx_vertex_definition* vertices;
	int edge_count;
	struct linx_edge_definition* edges;
	int propagated_pin_count;
	struct linx_pin_ref* propagated_pins;
};

struct linx_buffer {
	float* float_buffer;
	int write_count;
};

struct linx_value;

struct linx_value_array {
	struct linx_value* items;
	int length;
	int capacity;
};

/* module definitions */

struct linx_vertex_instance;

typedef void (linx_create_functype)(struct linx_vertex_instance*, int samplerate);
typedef void (linx_destroy_functype)(struct linx_vertex_instance*);

typedef void(linx_process_functype)(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count);
typedef void(linx_describe_value_functype)(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size);

/*
	the union must be interpreted by the pin type of the referenced pin.

	if pin_type is in_scalar or out_scalar:
		if type is int, then intvalue is valid
		if type is float, then floatvalue is valid
		multiple scalar messages on the same pin+timestamp are undefined

	if pin_type is in_buffer or out_buffer:
		timestamp must be 0 - buffers are always sample-accurate
		if type is int, then intbuffer is valid
		if type is float, then floatbuffer is valid

	if pin_type is in_midi or out_midi:
		then midimessage is valid
		multiple midi messages on the same pin+timestamp are allowed
*/
struct linx_value {
	union {
		int intvalue;
		float floatvalue;
		unsigned int midimessage;
	};
	enum linx_pin_group pin_group;
	int pin_index;
	int timestamp;
};

struct linx_pin {
	const char* name;
	enum linx_pin_type pin_type;
	float min_value;
	float max_value;
	float default_value;
	float precision;
};

enum linx_factory_flags {
	linx_factory_flag_is_timestamp_aware = 0x00000001,
	linx_factory_flag_is_subgraph_parent = 0x00000002,
};

struct linx_factory {
	unsigned int version_size;
	const char* name;
	unsigned int flags;
	unsigned int pin_count;
	struct linx_pin* pins;
	unsigned int subgraph_pin_count;
	struct linx_pin* subgraph_pins;
	linx_create_functype* create;
	linx_destroy_functype* destroy;
	linx_process_functype* process;
	linx_describe_value_functype* describe_value;
};

/* internal graph instance definitions */

struct linx_vertex_instance {
	void* data;
	struct linx_vertex_definition* vertex;
	struct linx_graph_instance* subgraph;
	struct linx_buffer* pin_buffers;
	struct linx_buffer* propagated_pin_buffers;
	struct linx_buffer* subgraph_pin_buffers;
	struct linx_value_array* in_values;
	struct linx_value_array* out_values;
	struct linx_value_array* subgraph_in_values;
	struct linx_value_array* subgraph_out_values;
	struct linx_value* last_in_values;
};

struct linx_edge_instance {
	float* digest;
};

struct linx_processing_layer {
	int vertex_count;
	int* vertices;
};

struct linx_processing_order {
	int layer_count;
	struct linx_processing_layer* layers;
};

struct linx_snapshot {
	float samplerate;
	unsigned int position;
	float* buffer;
};

struct linx_graph_instance {
	struct linx_graph_definition* graph;
	struct linx_processing_order processing_order;
	struct linx_vertex_instance* vertex_data;
	struct linx_edge_instance* edge_data;
	struct linx_pin* propagated_pins;
	struct linx_snapshot* snapshot;
};

extern struct linx_graph_definition* linx_graph;
extern const char linx_graph_author[];
extern const char linx_graph_product[];
extern int linx_graph_uniqueid;

struct linx_graph_instance* linx_graph_definition_create_instance(struct linx_graph_definition* graph, int samplerate);
void linx_graph_instance_destroy(struct linx_graph_instance* graph);

void linx_graph_instance_process(struct linx_graph_instance* graph, struct linx_vertex_instance* context, struct linx_value_array* in_values, struct linx_value_array* out_values, int sample_count);
void linx_graph_instance_process_clear(struct linx_graph_instance* graph);

void linx_vertex_instance_process_subgraph(struct linx_graph_instance* subgraphinst, struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_value_array* out_values, int sample_count);

// resolve propagated pin on a graf
struct linx_pin* linx_graph_definition_resolve_pin(struct linx_graph_definition* graph, int pin_index, struct linx_vertex_definition** result_vertdef, int* result_pin_index);

// resolve propagated pin and vertex instance in a graf instance
struct linx_pin* linx_graph_instance_resolve_pin(struct linx_graph_instance* graphdata, int pin_index, struct linx_vertex_instance** result_vertdata, int* result_pin_index);

// describe arbitrary module pin in a graf or subgraf
void linx_graph_instance_describe_vertex_value(struct linx_graph_instance* instance, int vertex_index, int pin_index, enum linx_pin_group pin_group, float value, char* name_result, int name_length);

// describe propagated parameter on a graf
void linx_graph_instance_describe_value(struct linx_graph_instance* instance, int pin_index, float value, char* name_result, int name_length);

// resolve buffer for a propagated pin 
struct linx_buffer* linx_graph_instance_get_propagated_pin_buffer(struct linx_graph_instance* instance, int pin_index);

// get the init value for an int or float scalar input pin from the vertex definition, or default value if there is no init value
float linx_vertex_definition_get_pin_default_init_value(struct linx_vertex_definition* vertdef, int pin_index);

int linx_pin_is_in(enum linx_pin_type type);
int linx_pin_is_out(enum linx_pin_type type);

unsigned int midi_make(unsigned char channel, unsigned char command, unsigned char data1, unsigned char data2);
void midi_parse(unsigned int message, unsigned short* status, unsigned char* channel, unsigned char* command, unsigned char* data1, unsigned char* data2);

void linx_buffer_write(struct linx_buffer* to_buffer, float* srcbuffer, int sample_count);
void linx_buffer_read(float* destbuffer, struct linx_buffer* srcbuffer, int sample_count);
void linx_buffer_copy_chunk(struct linx_buffer* dest_buffer, struct linx_buffer* src_buffer, int dest_offset, int src_offset, int sample_count);

void linx_value_array_init_from(struct linx_value_array* values, struct linx_value* ptr, int length, int capacity);
struct linx_value_array* linx_value_array_create(int capacity);
struct linx_value* linx_value_array_get(struct linx_value_array* values, int index);
void linx_value_array_push_value(struct linx_value_array* values, int pin_index, enum linx_pin_group pin_group, struct linx_value* value, int timestamp);
void linx_value_array_push_float(struct linx_value_array* values, int pin_index, enum linx_pin_group pin_group, float value, int timestamp);
void linx_value_array_push_int(struct linx_value_array* values, int pin_index, enum linx_pin_group pin_group, int value, int timestamp);
void linx_value_array_push_midi(struct linx_value_array* values, int pin_index, enum linx_pin_group pin_group, unsigned int message, int timestamp);
int linx_value_array_get_next_timestamp(struct linx_value_array* values, int sample_count);
void linx_value_array_copy_in_values(struct linx_value_array* dest_values, struct linx_value_array* src_values, int sample_count);
void linx_value_array_shift_values(struct linx_value_array* values, int sample_count);
void linx_value_array_set_last_values(struct linx_value* last_values, struct linx_value_array* new_values, int pin_count, int sample_count);
void linx_value_array_free(struct linx_value_array* values);

struct linx_snapshot* linx_graph_instance_create_snapshot(struct linx_graph_instance* instance, int samplerate);
void linx_graph_instance_update_snapshot(struct linx_graph_instance* instance, struct linx_vertex_instance* context, int sample_count);
void linx_graph_instance_free_snapshot(struct linx_graph_instance* instance);

#ifdef __cplusplus
}
#endif
