 #define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include "linxaudio.h"

struct linx_pin uservalue_f0_1_pins[] = {
	{ "In", linx_pin_type_in_scalar_float, 0.0f, 1.0f, 0.0f, 0.01f },
	{ "Out", linx_pin_type_out_scalar_float, 0.0f, 1.0f, 0.0f, 0.01f },
};

struct linx_pin uservalue_fn1_1_pins[] = {
	{ "In", linx_pin_type_in_scalar_float, -1.0f, 1.0f, 0.0f, 0.01f },
	{ "Out", linx_pin_type_out_scalar_float, -1.0f, 1.0f, 0.0f, 0.01f },
};

struct linx_pin uservalue_u8_pins[] = {
	{ "In", linx_pin_type_in_scalar_int, 0.0f, 255.0f, 0.0f },
	{ "Out", linx_pin_type_out_scalar_int, 0.0f, 255.0f, 0.0f },
};

struct linx_pin uservalue_u7_pins[] = {
	{ "In", linx_pin_type_in_scalar_int, 0.0f, 127.0f, 0.0f },
	{ "Out", linx_pin_type_out_scalar_int, 0.0f, 127.0f, 0.0f },
};

// user pin for propagating a cutoff parameter
struct linx_pin uservalue_hertz_pins[] = {
	{ "In", linx_pin_type_in_scalar_float, 33.0f, 22050.0f, 10000.0f, 1.0f },
	{ "Out", linx_pin_type_out_scalar_float, 33.0f, 22050.0f, 10000.0f, 1.0f  },
};

// user pin for propagating a ms parameter
struct linx_pin uservalue_ms_pins[] = {
	{ "In", linx_pin_type_in_scalar_float, 0.001f, 2000.0f, 750.0f, 1.0f },
	{ "Out", linx_pin_type_out_scalar_float, 0.001f, 2000.0f, 750.0f, 1.0f },
};

void uservalue_float_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	int out_index = 0;
	float value;
	int has_value = 0;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 0 && v->timestamp == 0) {
			value = v->floatvalue;
			has_value = 1;
		}
	}

	if (has_value) {
		linx_value_array_push_float(out_values, 1, linx_pin_group_module, value, 0);
	}
}

void uservalue_int_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	int out_index = 0;
	int value;
	int has_value = 0;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 0 && v->timestamp == 0) {
			value = v->intvalue;
			has_value = 1;
		}
	}

	if (has_value) {
		linx_value_array_push_int(out_values, 1, linx_pin_group_module, value, 0);
	}
}


void uservalue_float_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
}

void uservalue_int_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
}

void uservalue_hertz_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
	switch (pin_index) {
		case 0:
			snprintf(result_name, result_name_size, "%.2f Hz", value);
			break;
		default: 
			break;
	}
}

void uservalue_ms_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
	switch (pin_index) {
	case 0:
		snprintf(result_name, result_name_size, "%.2f ms", value);
		break;
	default:
		break;
	}
}

void uservalue_float_create(struct linx_vertex_instance* plugin, int samplerate) {
	plugin->data = 0;
}

void uservalue_int_create(struct linx_vertex_instance* plugin, int samplerate) {
	plugin->data = 0;
}

void uservalue_destroy(struct linx_vertex_instance* plugin) {
	struct uservalue* data = (struct uservalue*)plugin->data;
	free(data);
}

struct linx_factory uservalue_f0_1_factory = {
	sizeof(struct linx_factory), "uservalue_f0_1", 0,
	(sizeof(uservalue_f0_1_pins) / sizeof(struct linx_pin)), uservalue_f0_1_pins,
	0, 0,
	uservalue_float_create, uservalue_destroy, uservalue_float_process, uservalue_float_describe_value
};

struct linx_factory* uservalue_f0_1_get_factory() {
	return &uservalue_f0_1_factory;
}

struct linx_factory uservalue_fn1_1_factory = {
	sizeof(struct linx_factory), "uservalue_fn1_1", 0,
	(sizeof(uservalue_fn1_1_pins) / sizeof(struct linx_pin)), uservalue_fn1_1_pins,
	0, 0,
	uservalue_float_create, uservalue_destroy, uservalue_float_process, uservalue_float_describe_value
};

struct linx_factory* uservalue_fn1_1_get_factory() {
	return &uservalue_fn1_1_factory;
}


struct linx_factory uservalue_u7_factory = {
	sizeof(struct linx_factory), "uservalue_u7", 0,
	(sizeof(uservalue_u7_pins) / sizeof(struct linx_pin)), uservalue_u7_pins,
	0, 0,
	uservalue_int_create, uservalue_destroy, uservalue_int_process, uservalue_int_describe_value
};

struct linx_factory* uservalue_u7_get_factory() {
	return &uservalue_u7_factory;
}


struct linx_factory uservalue_u8_factory = {
	sizeof(struct linx_factory), "uservalue_u8", 0,
	(sizeof(uservalue_u8_pins) / sizeof(struct linx_pin)), uservalue_u8_pins,
	0, 0,
	uservalue_int_create, uservalue_destroy, uservalue_int_process, uservalue_int_describe_value
};

struct linx_factory* uservalue_u8_get_factory() {
	return &uservalue_u8_factory;
}


struct linx_factory uservalue_hertz_factory = {
	sizeof(struct linx_factory), "uservalue_hertz", 0,
	(sizeof(uservalue_hertz_pins) / sizeof(struct linx_pin)), uservalue_hertz_pins,
	0, 0,
	uservalue_float_create, uservalue_destroy, uservalue_float_process, uservalue_hertz_describe_value
};

struct linx_factory* uservalue_hertz_get_factory() {
	return &uservalue_hertz_factory;
}


struct linx_factory uservalue_ms_factory = {
	sizeof(struct linx_factory), "uservalue_ms", 0,
	(sizeof(uservalue_ms_pins) / sizeof(struct linx_pin)), uservalue_ms_pins,
	0, 0,
	uservalue_float_create, uservalue_destroy, uservalue_float_process, uservalue_ms_describe_value
};

struct linx_factory* uservalue_ms_get_factory() {
	return &uservalue_ms_factory;
}
