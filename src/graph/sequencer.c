#include <string.h>

struct pattern_event {
	int timestamp;
	union {
		int int_value;
		float float_value;
	};
};

struct pattern_column {
	int event_count;
	struct pattern_event* events;
	int module_index;
	int pin_group;
	int pin_index;
	int repeat_index;
};

struct pattern {
	int column_count;
	struct pattern_column* columns;
};

struct sequencer_event {
	int timestamp;
	int pattern_index;
};

struct sequencer_track {
	int event_count;
	struct sequencer_event* events;
};

struct sequencer {
	struct linx_graph_definition* graph;
	struct linx_graph_instance* graph_data;
	int track_count;
	struct sequencer_track* tracks;

	int pattern_count;
	struct pattern* patterns;
};

struct sequencer* sequencer_create(struct linx_graph_definition* graph, struct linx_graph_instance* graph_data) {
	struct sequencer* result = (struct sequencer*)malloc(sizeof(struct sequencer));
	result->graph = graph;
	result->graph_data = graph_data;
	result->track_count = 0;
	result->tracks = NULL;
	return result;
}

void sequencer_process(struct sequencer* self, int numsamples) {
	// float samplerate, ...
	// this should simply schedule pin events in the range we're processing!

	// prolem: recompile and rebuild graf for every change

	// have X tracks, and a pattern playing in each,
	// each pattern have interpolated columns
}


// save as exe: self playing thingy, native directsound/pulseaudio host or something

struct pattern* pattern_create(struct sequencer* self) {
	struct pattern* result = (struct pattern*)malloc(sizeof(struct pattern));
	result->column_count = 0;
	result->columns = NULL;
	return result;
}

void pattern_add_column(struct pattern* self) {

}

