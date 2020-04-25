#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include "linxaudio.h"

struct linx_pin arithmeticvalue_pins[] = {
	{ "Lhs", linx_pin_type_in_scalar_float, -FLT_MAX, FLT_MAX, 0.0f },
	{ "Rhs", linx_pin_type_in_scalar_float, -FLT_MAX, FLT_MAX, 0.0f },
	{ "Type", linx_pin_type_in_scalar_int, 0, 4, 0 },
	{ "Out", linx_pin_type_out_scalar_float, -FLT_MAX, FLT_MAX, 0.0f }
};

struct arithmeticvalue {
	int type;
	float lhs, rhs;
};

void arithmeticvalue_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	struct arithmeticvalue* data = (struct arithmeticvalue*)self->data;
	float value;
	int has_value = 0;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 0 && v->timestamp == 0) {
			data->lhs = v->floatvalue;
			has_value = 1;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 1 && v->timestamp == 0) {
			data->rhs = v->floatvalue;
			has_value = 1;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 2 && v->timestamp == 0) {
			data->type = v->intvalue;
		}
	}

	if (has_value) {
		switch (data->type) {
			case 0:
				value = data->lhs + data->rhs;
				break;
			case 1:
				value = data->lhs - data->rhs;
				break;
			case 2:
				value = data->lhs * data->rhs;
				break;
			case 3:
				if (data->rhs != 0) {
					value = data->lhs / data->rhs;
				} else {
					value = 0;
				}
				break;
			case 4:
				if (data->rhs != 0) {
					value = fmodf(data->lhs, data->rhs);
				} else {
					value = 0;
				}
				break;
			default:
				value = 0;
				break;
		}
		linx_value_array_push_float(out_values, 3, linx_pin_group_module, value, 0);
	}
}


static const char* arithmetic_types[] = {
	"Add", "Subtract", "Multiply", "Divide", "Modulus"
};

void arithmeticvalue_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
	switch (pin_index) {
		case 2:
			snprintf(result_name, result_name_size, "%s", arithmetic_types[(int)value]);
			break;
	}

}

void arithmeticvalue_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct arithmeticvalue* data = (struct arithmeticvalue*)malloc(sizeof(struct arithmeticvalue));
	memset(data, 0, sizeof(struct arithmeticvalue));
	plugin->data = data;
}

void arithmeticvalue_destroy(struct linx_vertex_instance* plugin) {
	struct arithmeticvalue* data = (struct arithmeticvalue*)plugin->data;
	free(data);
}

struct linx_factory arithmeticvalue_factory = {
	sizeof(struct linx_factory), "arithmeticvalue", 0,
	(sizeof(arithmeticvalue_pins) / sizeof(struct linx_pin)), arithmeticvalue_pins,
	0, 0,
	arithmeticvalue_create, arithmeticvalue_destroy, arithmeticvalue_process, arithmeticvalue_describe_value
};

struct linx_factory* arithmeticvalue_get_factory() {
	return &arithmeticvalue_factory;
}
