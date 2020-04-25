#pragma once

typedef struct inertia_value {
	float current;
	float target;
	float delta;
	int frames;
} inertia_value_t;

void inertia_set_ms(inertia_value_t* value, int ms, float samplerate, float floatvalue);
void inertia_update(inertia_value_t* value);
