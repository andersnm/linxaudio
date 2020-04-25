#define _USE_MATH_DEFINES
#include <assert.h>
#include <math.h>
#include "mathext.h"
#include "inertia.h"

void inertia_set_ms(inertia_value_t* value, int ms, float samplerate, float floatvalue) {
	value->frames = (int)((float)ms * 0.001f * samplerate);
	value->frames = fmaxf(1, value->frames);

	assert(value->frames > 0);
	value->target = floatvalue;
	value->delta = (value->target - value->current) / (float)value->frames; // 1ms
}

void inertia_update(inertia_value_t* value) {
	if (value->frames == 0) {
		value->current = value->target;
		return;
	}

	assert(value->frames > 0);
	value->frames--;

	if ((value->target > value->current && value->delta > 0) || (value->target < value->current && value->delta < 0)) {
		value->current += value->delta;
	}
}
