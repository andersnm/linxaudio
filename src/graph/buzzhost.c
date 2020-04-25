#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include "thiscall.h"
#include "linxaudio.h"
#include "commonhost.h"
#include "buzzhost.h"

void dsp_i2s(float ** ssamples, float* isamples, float amp, int sample_count) {
	int i;
	for (i = 0; i < sample_count; i++) {
		ssamples[0][i] = isamples[i * 2 + 0] * amp;
		ssamples[1][i] = isamples[i * 2 + 1] * amp;
	}
}

void dsp_s2i(float* isamples, float ** ssamples, float amp, int sample_count) {
	int i;
	for (i = 0; i < sample_count; i++) {
		isamples[i * 2 + 0] = ssamples[0][i] * amp;
		isamples[i * 2 + 1] = ssamples[1][i] * amp;
	}
}

void dsp_amp(float* samples, float amp, int sample_count) {
	int i;
	for (i = 0; i < sample_count; i++) {
		samples[i] *= amp;
	}
}

void dsp_copy(float* out, float* in, float amp, int sample_count) {
	int i;
	for (i = 0; i < sample_count; i++) {
		out[i] = in[i] * amp;
	}
}

int linx_buzz_value_from_scalar(float value, struct linx_pin* pin) {
	float param_scale = pin->max_value - pin->min_value;
	float result = (value - pin->min_value) / param_scale;
	return (int)(result * 127.0f);
}

float linx_scalar_from_buzz_value(int value, struct linx_pin* pin) {
	float param_scale = pin->max_value - pin->min_value;
	float result = (float)value / 127.0f * param_scale + pin->min_value;
	if (result > pin->max_value) {
		result = pin->max_value;
	} else if (result < pin->min_value) {
		result = pin->min_value;
	}
	return result;
}

struct buzz_userdata {
	void* CMachineInterface_trampolines;
	void* CMachineInterfaceEx_trampolines;
	struct CMachineInfo* info;
	struct CMachineInterface* mi;
	struct CMachineInterfaceEx* miex;
	struct linx_graph_definition* graph;
	struct linx_graph_instance* graph_data;
	struct linx_value_array* in_values;
	struct linx_value_array* out_values;
};

struct thiscall_wrap CMICallbacks_vtbl[];

typedef const void*(cvp_cb_i)(struct CMICallbacks*, int);
typedef const void*(cvp_cb_i_i)(struct CMICallbacks*, int, int);
typedef void*(vp_cb)(struct CMICallbacks*);
typedef void*(vp_cb_ccp)(struct CMICallbacks*, char const*);

typedef void (v_cb_i)(struct CMICallbacks*, int);
typedef void (v_cb_vp_i)(struct CMICallbacks*, void*, int);
typedef void (v_cb_miex)(struct CMICallbacks*, struct CMachineInterfaceEx* miex);

typedef int (i_cb)(struct CMICallbacks*);


//
// CMICallbacks implementation
//

void const *CMICallbacks_GetNearestWaveLevel(struct CMICallbacks* self, int i, int note) {
	// entry to MDK heaven, call with -1, -1 or -2, -2 for various magic!
	cvp_cb_i_i* GetNearestWaveLevel = (cvp_cb_i_i*)CMICallbacks_vtbl[16].trampoline_function_ptr ;
	return GetNearestWaveLevel(self, i, note);
}

void CMICallbacks_SetMachineInterfaceEx(struct CMICallbacks* self, struct CMachineInterfaceEx *pex) {
	v_cb_miex* SetMachineInterfaceEx = (v_cb_miex*)CMICallbacks_vtbl[29].trampoline_function_ptr ;
	SetMachineInterfaceEx(self, pex);
}

void* CMICallbacks_GetThisMachine(struct CMICallbacks* self) {
	vp_cb* GetThisMachine = (vp_cb*)CMICallbacks_vtbl[34].trampoline_function_ptr ;
	return GetThisMachine(self);
}

int CMICallbacks_GetStateFlags(struct CMICallbacks* self) {
	i_cb* GetStateFlags = (i_cb*)CMICallbacks_vtbl[38].trampoline_function_ptr ;
	return GetStateFlags(self);
}

void CMICallbacks_SetnumOutputChannels(struct CMICallbacks* self, void* pmac, int n) {
	v_cb_vp_i* SetnumOutputChannels = (v_cb_vp_i*)CMICallbacks_vtbl[39].trampoline_function_ptr ;
	SetnumOutputChannels(self, pmac, n);
}

void* CMICallbacks_GetMachine(struct CMICallbacks* self, char const *name) {
	vp_cb_ccp* SetnumOutputChannels = (vp_cb_ccp*)CMICallbacks_vtbl[44].trampoline_function_ptr ;
	return SetnumOutputChannels(self, name);
}

int CMICallbacks_GetHostVersion(struct CMICallbacks* self) {
	i_cb* GetHostVersion = (i_cb*)CMICallbacks_vtbl[48].trampoline_function_ptr ;
	return GetHostVersion(self);
}

int CMICallbacks_GetSongPosition(struct CMICallbacks* self) {
	i_cb* GetSongPosition = (i_cb*)CMICallbacks_vtbl[49].trampoline_function_ptr ;
	return GetSongPosition(self);
}

int CMICallbacks_GetTempo(struct CMICallbacks* self) {
	i_cb* GetTempo = (i_cb*)CMICallbacks_vtbl[51].trampoline_function_ptr ;
	return GetTempo(self);
}

int CMICallbacks_GetTPB(struct CMICallbacks* self) {
	i_cb* GetTPB = (i_cb*)CMICallbacks_vtbl[53].trampoline_function_ptr ;
	return GetTPB(self);
}

int CMICallbacks_HostMIDIFiltering(struct CMICallbacks* self) {
	i_cb* HostMIDIFiltering = (i_cb*)CMICallbacks_vtbl[51].trampoline_function_ptr ;
	return HostMIDIFiltering(self);
}

int CMICallbacks_GetBuildNumber(struct CMICallbacks* self) {
	i_cb* GetBuildNumber = (i_cb*)CMICallbacks_vtbl[107].trampoline_function_ptr ;
	return GetBuildNumber(self);
}

void CMICallbacks_SetInputChannelCount(struct CMICallbacks* self, int count) {
	v_cb_i* SetInputChannelCount = (v_cb_i*)CMICallbacks_vtbl[114].trampoline_function_ptr ;
	SetInputChannelCount(self, count);
}

void CMICallbacks_SetOutputChannelCount(struct CMICallbacks* self, int count) {
	v_cb_i* SetOutputChannelCount = (v_cb_i*)CMICallbacks_vtbl[115].trampoline_function_ptr ;
	SetOutputChannelCount(self, count);
}

//
// CMachineInterface implementation
//

void CMachineInterface_destructor(struct CMachineInterface* self, int should_delete) {
	// http://stackoverflow.com/questions/7750280/how-does-virtual-destructor-work-in-c
	// The argument is used by the destructor to know if it should call delete at the end.
	if (should_delete == 1) {
		free(self);
	}
}

void CMachineInterface_Init(struct CMachineInterface* self, void* ptr) {
	struct buzz_userdata* data = (struct buzz_userdata*)self->userdata;
	assert(self->userdata != 0);
	assert(self->pCB);

	CMICallbacks_SetMachineInterfaceEx(self->pCB, data->miex);

	if ((data->info->Flags & MIF_MULTI_IO) != 0) {
		// NOTE: buzz channel count is number of stereo channels
		CMICallbacks_SetInputChannelCount(self->pCB, (data->info->audio_in_count + 1) / 2);
		CMICallbacks_SetOutputChannelCount(self->pCB, (data->info->audio_out_count + 1) / 2);
	}

	data->graph_data = linx_graph_definition_create_instance(linx_graph, self->pMasterInfo->SamplesPerSec);
}

void CMachineInterface_Tick(struct CMachineInterface* self) {
	struct buzz_userdata* data = (struct buzz_userdata*)self->userdata;

	int i;
	unsigned char* global_values = (unsigned char*)self->GlobalVals;
	for (i = 0; i < data->info->param_in_count; i++) {
		struct CMachineParameter* param = data->info->Parameters[i];
		if (global_values[i] != param->NoValue) {
			int pin_index = data->info->in_parameters[i].extra_index;
			struct linx_pin* pin = &data->graph_data->propagated_pins[pin_index];
			float value = linx_scalar_from_buzz_value(global_values[i], pin);

			if (pin->pin_type == linx_pin_type_in_scalar_int) {
				linx_value_array_push_int(data->in_values, pin_index, linx_pin_group_propagated, (int)(value), 0);
			} else if (pin->pin_type == linx_pin_type_in_scalar_float) {
				linx_value_array_push_float(data->in_values, pin_index, linx_pin_group_propagated, value, 0);
			} else {
				assert(0);
			}
		}
	}
}

int CMachineInterface_Work(struct CMachineInterface* self, float *psamples, int numsamples, int const mode) { 
	// used by 0->0, 0->1, 1->0, 1->1, 2->0, 2->2, 
	int pin_index0;
	int pin_index1;
	float temp_buffer[2][MAX_BUFFER_LENGTH] = {0};
	float silent_buffer[MAX_BUFFER_LENGTH] = {0};
	float* temp_buffer_ptr[] = { temp_buffer[0], temp_buffer[1] };
	struct buzz_userdata* data = (struct buzz_userdata*)self->userdata;
	struct linx_buffer* pin_buffer0;
	struct linx_buffer* pin_buffer1;

	if ((data->info->Flags & MIF_STEREO_EFFECT) != 0) {
		assert(data->info->audio_in_count == 2);
		pin_index0 = data->info->in_audios[0].extra_index;
		pin_index1 = data->info->in_audios[1].extra_index;

		// psamples is always interleaved stereo input
		dsp_i2s(temp_buffer_ptr, psamples, 1.0f/32768.0f, numsamples);

		pin_buffer0 = linx_graph_instance_get_propagated_pin_buffer(data->graph_data, pin_index0);
		pin_buffer1 = linx_graph_instance_get_propagated_pin_buffer(data->graph_data, pin_index1);
		linx_buffer_write(pin_buffer0, temp_buffer[0], numsamples);
		linx_buffer_write(pin_buffer0, temp_buffer[1], numsamples);
	} else {
		// psamples is mono buffer
		if (data->info->Type == MT_EFFECT) {
			assert(data->info->audio_in_count == 1);
			pin_index0 = data->info->in_audios[0].extra_index;

			dsp_copy(temp_buffer[0], psamples, 1.0f / 32768.0f, numsamples);
			pin_buffer0 = linx_graph_instance_get_propagated_pin_buffer(data->graph_data, pin_index0);
			linx_buffer_write(pin_buffer0, temp_buffer[0], numsamples);
		} else {
			assert(data->info->audio_in_count == 0);
		}
	}

	linx_graph_instance_process(data->graph_data, 0, data->in_values, data->out_values, numsamples);

	if ((data->info->Flags & MIF_STEREO_EFFECT) != 0) {
		if ((data->info->Flags & MIF_NO_OUTPUT) == 0) {
			// returns interleaved stereo
			assert(data->info->audio_out_count == 2);
			pin_index0 = data->info->out_audios[0].extra_index;
			pin_index1 = data->info->out_audios[1].extra_index;
			
			pin_buffer0 = linx_graph_instance_get_propagated_pin_buffer(data->graph_data, pin_index0);
			if (pin_buffer0->write_count > 0) {
				temp_buffer_ptr[0] = pin_buffer0->float_buffer;
			} else {
				temp_buffer_ptr[0] = silent_buffer;
			}

			pin_buffer1 = linx_graph_instance_get_propagated_pin_buffer(data->graph_data, pin_index1);
			if (pin_buffer1->write_count > 0) {
				temp_buffer_ptr[1] = pin_buffer1->float_buffer;
			} else {
				temp_buffer_ptr[1] = silent_buffer;
			}

			dsp_s2i(psamples, temp_buffer_ptr, 32768.0f, numsamples);
		} else {
			assert(data->info->audio_out_count == 0);
		}
	} else {
		if ((data->info->Flags & MIF_NO_OUTPUT) == 0) {
			// returns mono
			assert(data->info->audio_out_count == 1);
			pin_index0 = data->info->out_audios[0].extra_index;

			pin_buffer0 = linx_graph_instance_get_propagated_pin_buffer(data->graph_data, pin_index0);
			if (pin_buffer0->write_count > 0) {
				temp_buffer_ptr[0] = pin_buffer0->float_buffer;
				dsp_copy(psamples, temp_buffer_ptr[0], 32768.0f, numsamples);
			} else {
				memset(psamples, 0, sizeof(float) * numsamples);
			}
		} else {
			assert(data->info->audio_out_count == 0);
		}
	}

	linx_graph_instance_process_clear(data->graph_data);

	data->in_values->length = 0;
	data->out_values->length = 0;
	return 1;
}

int CMachineInterface_WorkMonoToStereo(struct CMachineInterface* self, float *pin, float *pout, int numsamples, int const mode) { 
	// used by 0->2, 1->2
	int pin_index0;
	int pin_index1;
	float temp_buffer[2][MAX_BUFFER_LENGTH] = {0};
	float silent_buffer[MAX_BUFFER_LENGTH] = {0};
	float* temp_buffer_ptr[] = { temp_buffer[0], temp_buffer[1] };
	struct buzz_userdata* data = (struct buzz_userdata*)self->userdata;
	struct linx_buffer* pin_buffer0;
	struct linx_buffer* pin_buffer1;

	assert((data->info->Flags & MIF_MONO_TO_STEREO) != 0);

	if (data->info->Type == MT_GENERATOR) {
		assert(data->info->audio_in_count == 0);
		assert(data->info->audio_out_count == 2);
	} else {
		assert(data->info->audio_in_count == 1);
		assert(data->info->audio_out_count == 2);
		pin_index0 = data->info->in_audios[0].extra_index;

		dsp_copy(temp_buffer[0], pin, 1.0f/32768.0f, numsamples);
		pin_buffer0 = linx_graph_instance_get_propagated_pin_buffer(data->graph_data, pin_index0);
		linx_buffer_write(pin_buffer0, temp_buffer[0], numsamples);
	}

	linx_graph_instance_process(data->graph_data, 0, data->in_values, data->out_values, numsamples);

	pin_index0 = data->info->out_audios[0].extra_index;
	pin_index1 = data->info->out_audios[1].extra_index;

	pin_buffer0 = linx_graph_instance_get_propagated_pin_buffer(data->graph_data, pin_index0);
	if (pin_buffer0->write_count > 0) {
		temp_buffer_ptr[0] = pin_buffer0->float_buffer;
	} else {
		temp_buffer_ptr[0] = silent_buffer;
	}

	pin_buffer1 = linx_graph_instance_get_propagated_pin_buffer(data->graph_data, pin_index1);
	if (pin_buffer1->write_count > 0) {
		temp_buffer_ptr[1] = pin_buffer1->float_buffer;
	} else {
		temp_buffer_ptr[1] = silent_buffer;
	}
	dsp_s2i(pout, temp_buffer_ptr, 32768.0f, numsamples);

	linx_graph_instance_process_clear(data->graph_data);

	data->in_values->length = 0;
	data->out_values->length = 0;
	return 1;
}

void CMachineInterface_Stop(struct CMachineInterface* self) {}
void CMachineInterface_Save(struct CMachineInterface* self, void* const po) {}
void CMachineInterface_AttributesChanged(struct CMachineInterface* self) {}
void CMachineInterface_Command(struct CMachineInterface* self, int const i) {}
void CMachineInterface_SetNumTracks(struct CMachineInterface* self, int const n) {}
void CMachineInterface_MuteTrack(struct CMachineInterface* self, int const i) {}
int CMachineInterface_IsTrackMuted(struct CMachineInterface* self, int const i) { return 0; }

void CMachineInterface_MidiNote(struct CMachineInterface* self, int const channel, int const value, int const velocity) {
	int j;
	int extra_index;
	struct buzz_userdata* data = (struct buzz_userdata*)self->userdata;
	unsigned int message;
	for (j = 0; j < data->info->midi_in_count; j++) {
		extra_index = data->info->in_midis[j].extra_index;

		message = midi_make(channel, 9, value, velocity);
		linx_value_array_push_midi(data->in_values, extra_index, linx_pin_group_propagated, message, 0);
	}
}

void CMachineInterface_Event(struct CMachineInterface* self, uint32_t data) {}

const char* CMachineInterface_DescribeValue(struct CMachineInterface* self, int const index, int const value) { 
	struct buzz_userdata* data = (struct buzz_userdata*)self->userdata;

	int pin_index = data->info->in_parameters[index].extra_index;

	struct linx_pin_ref* pref = &data->graph->propagated_pins[pin_index];
	struct linx_pin* pin = &data->graph_data->propagated_pins[pin_index];

	float floatvalue = linx_scalar_from_buzz_value(value, pin);
	static char name[64];
	memset(name, 0, sizeof(name));
	linx_graph_instance_describe_value(data->graph_data, pin_index, floatvalue, name, 64 - 1);

	if (name[0] != 0) {
		return name;
	}
	return NULL; 
}

void** CMachineInterface_GetEnvelopeInfos(struct CMachineInterface* self) { return NULL; }
int CMachineInterface_PlayWave(struct CMachineInterface* self, int const wave, int const note, float const volume) { return 0; }
void CMachineInterface_StopWave(struct CMachineInterface* self) {}
int CMachineInterface_GetWaveEnvPlayPos(struct CMachineInterface* self, int const env) { return -1; }

struct thiscall_function CMachineInterface_vtbl[] = {
	{ &CMachineInterface_destructor, 1 },
	{ &CMachineInterface_Init, 1 },
	{ &CMachineInterface_Tick, 0 },
	{ &CMachineInterface_Work, 3 },
	{ &CMachineInterface_WorkMonoToStereo, 4 },
	{ &CMachineInterface_Stop, 0 },
	{ &CMachineInterface_Save, 1 },
	{ &CMachineInterface_AttributesChanged, 0 },
	{ &CMachineInterface_Command, 1 },
	{ &CMachineInterface_SetNumTracks, 1 },
	{ &CMachineInterface_MuteTrack, 1 },
	{ &CMachineInterface_IsTrackMuted, 1 },
	{ &CMachineInterface_MidiNote, 3 },
	{ &CMachineInterface_Event, 1 },
	{ &CMachineInterface_DescribeValue, 2 },
	{ &CMachineInterface_GetEnvelopeInfos, 0 },
	{ &CMachineInterface_PlayWave, 3 },
	{ &CMachineInterface_StopWave, 0 },
	{ &CMachineInterface_GetWaveEnvPlayPos, 1 },
};

int CMachineInterface_vtbl_count = sizeof(CMachineInterface_vtbl) / sizeof(struct thiscall_function);

char const * CMachineInterfaceEx_DescribeParam(struct CMachineInterfaceEx* self, int const param) { 
	return NULL; 
}

int CMachineInterfaceEx_SetInstrument(struct CMachineInterfaceEx* self, char const *name) { return 0; }
void CMachineInterfaceEx_GetSubMenu(struct CMachineInterfaceEx* self, int const i, void* pout) {}
void CMachineInterfaceEx_AddInput(struct CMachineInterfaceEx* self, char const *macname, int stereo) {}
void CMachineInterfaceEx_DeleteInput(struct CMachineInterfaceEx* self, char const *macename) {}
void CMachineInterfaceEx_RenameInput(struct CMachineInterfaceEx* self, char const *macoldname, char const *macnewname) {}
void CMachineInterfaceEx_Input(struct CMachineInterfaceEx* self, float *psamples, int numsamples, float amp) {}

void CMachineInterfaceEx_MidiControlChange(struct CMachineInterfaceEx* self, int const ctrl, int const channel, int const value) {
	// TODO: in_midis
}

void CMachineInterfaceEx_SetInputChannels(struct CMachineInterfaceEx* self, char const *macname, int stereo) {}
int CMachineInterfaceEx_HandleInput(struct CMachineInterfaceEx* self, int index, int amp, int pan) { return 0; }
void CMachineInterfaceEx_CreatePattern(struct CMachineInterfaceEx* self, void* p, int numrows) {}
void CMachineInterfaceEx_CreatePatternCopy(struct CMachineInterfaceEx* self, void* pnew, const void* pold) {}
void CMachineInterfaceEx_DeletePattern(struct CMachineInterfaceEx* self, void* p) {}
void CMachineInterfaceEx_RenamePattern(struct CMachineInterfaceEx* self, void* p, char const *name) {}
void CMachineInterfaceEx_SetPatternLength(struct CMachineInterfaceEx* self, void* p, int length) {}
void CMachineInterfaceEx_PlayPattern(struct CMachineInterfaceEx* self, void* p, void *s, int offset) {}
void *CMachineInterfaceEx_CreatePatternEditor(struct CMachineInterfaceEx* self, void *parenthwnd) { return NULL; }
void CMachineInterfaceEx_SetEditorPattern(struct CMachineInterfaceEx* self, void* p) {}
void CMachineInterfaceEx_AddTrack(struct CMachineInterfaceEx* self) {}
void CMachineInterfaceEx_DeleteLastTrack(struct CMachineInterfaceEx* self) {}
int CMachineInterfaceEx_EnableCommandUI(struct CMachineInterfaceEx* self, int id) { return 0; }
void CMachineInterfaceEx_DrawPatternBox(struct CMachineInterfaceEx* self, void *ctx) {}
void CMachineInterfaceEx_SetPatternTargetMachine(struct CMachineInterfaceEx* self, void* p, void* pmac) {}
void *CMachineInterfaceEx_CreateEmbeddedGUI(struct CMachineInterfaceEx* self, void *parenthwnd) { return NULL; }
void CMachineInterfaceEx_SelectWave(struct CMachineInterfaceEx* self, int i) {}
void CMachineInterfaceEx_SetDeletedState(struct CMachineInterfaceEx* self, int deleted) {}
int CMachineInterfaceEx_ShowPatternProperties(struct CMachineInterfaceEx* self) { return 0; }
int CMachineInterfaceEx_ImportPattern(struct CMachineInterfaceEx* self, void* p) { return 0; }
int CMachineInterfaceEx_GetLatency(struct CMachineInterfaceEx* self) { return 0; }
void CMachineInterfaceEx_RecordControlChange(struct CMachineInterfaceEx* self, void* pmac, int group, int track, int param, int value) {}
void CMachineInterfaceEx_GotMidiFocus(struct CMachineInterfaceEx* self) {}
void CMachineInterfaceEx_LostMidiFocus(struct CMachineInterfaceEx* self) {}
void CMachineInterfaceEx_BeginWriteToPlayingPattern(struct CMachineInterfaceEx* self, void* pmac, int quantization, void* outpwi) {}
void CMachineInterfaceEx_WriteToPlayingPattern(struct CMachineInterfaceEx* self, void* pmac, int group, int track, int param, int value) {}
void CMachineInterfaceEx_EndWriteToPlayingPattern(struct CMachineInterfaceEx* self, void* pmac) {}
int CMachineInterfaceEx_ShowPatternEditorHelp(struct CMachineInterfaceEx* self) { return 0; }
void CMachineInterfaceEx_SetBaseOctave(struct CMachineInterfaceEx* self, int bo) {}
int CMachineInterfaceEx_GetEditorPatternPosition(struct CMachineInterfaceEx* self) { return 0; }

void CMachineInterfaceEx_MultiWork(struct CMachineInterfaceEx* self, float const * const *inputs, float **outputs, int numsamples) {
	// used by any 2+->n or n->2+, or 2->1
}

char const *CMachineInterfaceEx_GetChannelName(struct CMachineInterfaceEx* self, int input, int index) { return NULL; }
int CMachineInterfaceEx_HandleGUIMessage(struct CMachineInterfaceEx* self, void *pout, void *pin) { return 0; }
int CMachineInterfaceEx_ExportMidiEvents(struct CMachineInterfaceEx* self, void* p, void *pout) { return 0; }
int CMachineInterfaceEx_ImportMidiEvents(struct CMachineInterfaceEx* self, void* p, void *pin) { return 0; }
void CMachineInterfaceEx_ThemeChanged(struct CMachineInterfaceEx* self) {}
void CMachineInterfaceEx_Load(struct CMachineInterfaceEx* self, void * const pi) {}
void CMachineInterfaceEx_ImportFinished(struct CMachineInterfaceEx* self) {}
int CMachineInterfaceEx_GetInstrument(struct CMachineInterfaceEx* self, char *buf, int bufsize) { return 0; }
void CMachineInterfaceEx_UpdateWaveReferences(struct CMachineInterfaceEx* self, void* ppat, uint8_t const *remap) {}
int CMachineInterfaceEx_IsValidAsciiChar(struct CMachineInterfaceEx* self, int param, char ch) { return 1; }
void CMachineInterfaceEx_DebugConsoleMessage(struct CMachineInterfaceEx* self, char const *text) {}

struct thiscall_function CMachineInterfaceEx_vtbl[] = {
	{ &CMachineInterfaceEx_DescribeParam, 1 },
	{ &CMachineInterfaceEx_SetInstrument, 1 },
	{ &CMachineInterfaceEx_GetSubMenu, 2 },
	{ &CMachineInterfaceEx_AddInput, 2 },
	{ &CMachineInterfaceEx_DeleteInput, 1 },
	{ &CMachineInterfaceEx_RenameInput, 2 },
	{ &CMachineInterfaceEx_Input, 3 },
	{ &CMachineInterfaceEx_MidiControlChange, 3 },
	{ &CMachineInterfaceEx_SetInputChannels, 2 },
	{ &CMachineInterfaceEx_HandleInput, 3 },
	{ &CMachineInterfaceEx_CreatePattern, 2 },
	{ &CMachineInterfaceEx_CreatePatternCopy, 2 },
	{ &CMachineInterfaceEx_DeletePattern, 1 },
	{ &CMachineInterfaceEx_RenamePattern, 2 },
	{ &CMachineInterfaceEx_SetPatternLength, 2 },
	{ &CMachineInterfaceEx_PlayPattern, 3 },
	{ &CMachineInterfaceEx_CreatePatternEditor, 1 },
	{ &CMachineInterfaceEx_SetEditorPattern, 1 },
	{ &CMachineInterfaceEx_AddTrack, 0 },
	{ &CMachineInterfaceEx_DeleteLastTrack, 0 },
	{ &CMachineInterfaceEx_EnableCommandUI, 1 },
	{ &CMachineInterfaceEx_DrawPatternBox, 1 },
	{ &CMachineInterfaceEx_SetPatternTargetMachine, 2 },
	{ &CMachineInterfaceEx_CreateEmbeddedGUI, 1 },
	{ &CMachineInterfaceEx_SelectWave, 1 },
	{ &CMachineInterfaceEx_SetDeletedState, 1 },
	{ &CMachineInterfaceEx_ShowPatternProperties, 0 },
	{ &CMachineInterfaceEx_ImportPattern, 1 },
	{ &CMachineInterfaceEx_GetLatency, 0 },
	{ &CMachineInterfaceEx_RecordControlChange, 5 },
	{ &CMachineInterfaceEx_GotMidiFocus, 0 },
	{ &CMachineInterfaceEx_LostMidiFocus, 0 },
	{ &CMachineInterfaceEx_BeginWriteToPlayingPattern, 3 },
	{ &CMachineInterfaceEx_WriteToPlayingPattern, 5 },
	{ &CMachineInterfaceEx_EndWriteToPlayingPattern, 1 },
	{ &CMachineInterfaceEx_ShowPatternEditorHelp, 0 },
	{ &CMachineInterfaceEx_SetBaseOctave, 1 },
	{ &CMachineInterfaceEx_GetEditorPatternPosition, 0 },
	{ &CMachineInterfaceEx_MultiWork, 3 },
	{ &CMachineInterfaceEx_GetChannelName, 2 },
	{ &CMachineInterfaceEx_HandleGUIMessage, 2 },
	{ &CMachineInterfaceEx_ExportMidiEvents, 2 },
	{ &CMachineInterfaceEx_ImportMidiEvents, 2 },
	{ &CMachineInterfaceEx_ThemeChanged, 0 },
	{ &CMachineInterfaceEx_Load, 1 },
	{ &CMachineInterfaceEx_ImportFinished, 0 },
	{ &CMachineInterfaceEx_GetInstrument, 2 },
	{ &CMachineInterfaceEx_UpdateWaveReferences, 2 },
	{ &CMachineInterfaceEx_IsValidAsciiChar, 2 },
	{ &CMachineInterfaceEx_DebugConsoleMessage, 1 }
};

int CMachineInterfaceEx_vtbl_count = sizeof(CMachineInterfaceEx_vtbl) / sizeof(struct thiscall_function);

void* CMICallbacks_wrap = 0;

struct thiscall_wrap CMICallbacks_vtbl[] = {
	{ 1 }, // GetWave
	{ 2 },
	{ 1 },
	{ 0 }, // Lock
	{ 0 }, // Unlock
	{ 0 }, // GetWritePos
	{ 0 }, // GetPlayPos
	{ 0 }, // GetAuxBuffer
	{ 0 }, // ClearAuxBuffer
	{ 0 }, // GetFreeWave
	{ 3 }, // AllocateWave
	{ 2 }, // ScheduleEvent
	{ 2 }, // MidiOut
	{ 1 }, // GetOscillatorTable
	{ 2 }, // GetEnvSize
	{ 6 }, // GetEnvPoint
	{ 2 }, // GetNearestWaveLevel
	{ 1 }, // SetNumberOfTracks
	{ 2 }, // CreatePattern
	{ 1 }, // GetPattern
	{ 1 }, // GetPatternName
	{ 2 }, // RenamePattern
	{ 1 }, // DeletePattern
	{ 5 }, // GetPatternData
	{ 6 }, // SetPatternData
	{ 0 }, // 
	{ 1 }, // 
	{ 1 }, // GetSequenceData
	{ 2 }, // SetSequenceData
	{ 1 }, // SetMachineInterfaceEx
	{ 4 }, // ControlChange__obsolete__
	{ 1 }, // ADGetnumChannels
	{ 3 }, // ADWrite
	{ 3 }, // ADRead
	{ 0 }, // GetThisMachine
	{ 5 }, // ControlChange
	{ 0 }, // GetPlayingSequence
	{ 3 }, // GetPlayingRow
	{ 0 }, // GetStateFlags
	{ 2 }, // SetnumOutputChannels
	{ 4 }, // SetEventHandler (vtbl index 40)
	{ 1 }, // GetWaveName
	{ 3 }, // SetInternalWaveName
	{ 1 }, // GetMachineNames
	{ 1 }, // GetMachine
	{ 1 }, // GetMachineInfo
	{ 1 }, // GetMachineName
	{ 5 }, // GetInput
	{ 0 }, // GetHostVersion
	{ 0 }, // GetSongPosition
	{ 1 }, // SetSongPosition (vtbl index 50)
	{ 0 }, // GetTempo
	{ 1 }, // SetTempo
	{ 0 }, // GetTPB
	{ 1 }, // SetTPB
	{ 0 }, // GetLoopStart
	{ 0 }, // GetLoopEnd
	{ 0 }, // GetSongEnd
	{ 0 }, // Play
	{ 0 }, // Stop
	{ 2 }, // RenameMachine (vtbl index 60)
	{ 0 }, // SetModifiedFlag
	{ 0 }, // GetAudioFrame
	{ 0 }, // HostMIDIFiltering
	{ 1 }, // GetThemeColor
	{ 2 }, // WriteProfileInt
	{ 2 }, // WriteProfileString
	{ 3 }, // WriteProfileBinary
	{ 2 }, // GetProfileInt
	{ 3 }, // GetProfileString
	{ 3 }, // GetProfileBinary (vtbl index 70)
	{ 1 }, // FreeProfileBinary
	{ 1 }, // GetNumTracks
	{ 2 }, // SetNumTracks
	{ 2 }, // SetPatternEditorStatusText
	{ 3 }, // DescribeValue
	{ 0 }, // GetBaseOctave
	{ 0 }, // GetSelectedWave
	{ 1 }, // SelectWave
	{ 2 }, // SetPatternLength
	{ 4 }, // GetpinState (80)
	{ 2 }, // ShowMachineWindow
	{ 2 }, // SetPatternEditorMachine
	{ 0 }, // GetSubTickInfo
	{ 1 }, // GetSequenceColumn
	{ 2 }, // SetGroovePattern
	{ 5 }, // ControlChangeImmediate
	{ 1 }, // SendControlChanges
	{ 2 }, // GetAttribute
	{ 3 }, // SetAttribute
	{ 1 }, // AttributesChanged (90)
	{ 3 }, // GetMachinePosition
	{ 3 }, // SetMachinePosition
	{ 2 }, // MuteMachine
	{ 1 }, // SoloMachine
	{ 1 }, // UpdatepinDisplays
	{ 1 }, // WriteLine
	{ 1 }, // GetOption
	{ 0 }, // GetPlayNotesState
	{ 1 }, // EnableMultithreading
	{ 2 }, // GetPatternByName (vtbl index 100)
	{ 2 }, // SetPatternName
	{ 1 }, // GetPatternLength
	{ 1 }, // GetPatternOwner
	{ 3 }, // MachineImplementsFunction
	{ 4 }, // SendMidiNote
	{ 4 }, // SendMidiControlChange
	{ 0 }, // GetBuildNumber
	{ 1 }, // SetMidiFocus
	{ 3 }, // BeginWriteToPlayingPattern
	{ 5 }, // WriteToPlayingPattern (110)
	{ 1 }, // EndWriteToPlayingPattern
	{ 0 }, // GetMainWindow
	{ 1 }, // DebugLock
	{ 1 }, // SetInputChannelCount
	{ 1 }, // SetOutputChannelCount
	{ 0 }, // IsSongClosing
	{ 1 }, // SetMidiInputMode
	{ 2 }, // RemapLoadedMachinepinIndex
	{ 0 }, // GetThemePath
	{ 2 }, // InvalidatepinValueDescription
	{ 2 }, // RemapLoadedMachineName
	{ 1 }, // IsMachineMuted
	{ 2 }, // GetInputChannelConnectionCount
	{ 2 }, // GetOutputChannelConnectionCount
	{ 0 }, // ToggleRecordMode
	{ 1 }, // GetSequenceCount
	{ 2 }, // GetSequence
	{ 1 }, // GetPlayingPattern
	{ 1 }, // GetPlayingPatternPosition
	{ 3 }, // IsValidAsciiChar
	{ 2 }, // GetConnectionCount
	{ 3 }, // GetConnection
	{ 2 }, // GetConnectionSource
	{ 2 }, // GetConnectionDestination
	{ 0 }, // GetTotalLatency
};

int CMICallbacks_vtbl_count = sizeof(CMICallbacks_vtbl) / sizeof(struct thiscall_wrap);

void linx_buzz_init_wrap() {
	if (CMICallbacks_wrap != 0) {
		return ;
	}
	CMICallbacks_wrap = thiscall_wrap_new_trampolines(CMICallbacks_vtbl_count, CMICallbacks_vtbl);
}

void linx_buzz_get_type_flags(int input_count, int output_count, int* type, int* flags) {
	switch (input_count) {
		case 0:
			*type = MT_GENERATOR;
			switch (output_count) {
				case 0:
					*flags = MIF_NO_OUTPUT;
					break;
				case 1:
					*flags = 0;
					break;
				case 2:
					*flags = MIF_MONO_TO_STEREO;
					break;
				default:
					*flags = MIF_MULTI_IO;
					break;
			}
			break;
		case 1:
			*type = MT_EFFECT;
			switch (output_count) {
				case 0:
					*flags = MIF_NO_OUTPUT;
					break;
				case 1:
					*flags = 0;
					break;
				case 2:
					*flags = MIF_MONO_TO_STEREO;
					break;
				default:
					*flags = MIF_MULTI_IO;
					break;
			}
			break;
		case 2:
			*type = MT_EFFECT;
			switch (output_count) {
				case 0:
					*flags = MIF_NO_OUTPUT | MIF_STEREO_EFFECT;
					break;
				case 1:
					*flags = MIF_MULTI_IO;
					break;
				case 2:
					*flags = MIF_STEREO_EFFECT;
					break;
				default:
					*flags = MIF_MULTI_IO;
					break;
			}
			break;
		default:
			// any 2+ input is a multi-io
			*type = MT_EFFECT;
			*flags = MIF_MULTI_IO;
			break;
	}
}

static struct CMachineInfo linx_buzz_machine_info = {0};

struct CMachineInfo* GetInfo() {
	int i;
	if (linx_buzz_machine_info.Type != 0) {
		return &linx_buzz_machine_info;
	}

	linx_buzz_init_wrap();

	memset(&linx_buzz_machine_info, 0, sizeof(struct CMachineInfo));
	linx_buzz_machine_info.Name = linx_graph_product;
	linx_buzz_machine_info.Author = linx_graph_author;
	linx_buzz_machine_info.ShortName = linx_graph_product;
	linx_host_get_pins(linx_graph,
		&linx_buzz_machine_info.param_in_count, &linx_buzz_machine_info.in_parameters, 
		&linx_buzz_machine_info.audio_in_count, &linx_buzz_machine_info.in_audios, 
		&linx_buzz_machine_info.audio_out_count, &linx_buzz_machine_info.out_audios, 
		&linx_buzz_machine_info.midi_in_count, &linx_buzz_machine_info.in_midis);

	linx_buzz_machine_info.Version = 15;

	// set type and flags
	linx_buzz_get_type_flags(linx_buzz_machine_info.audio_in_count, linx_buzz_machine_info.audio_out_count, 
		&linx_buzz_machine_info.Type, &linx_buzz_machine_info.Flags);

	linx_buzz_machine_info.numGlobalParameters = linx_buzz_machine_info.param_in_count;
	linx_buzz_machine_info.Parameters = (struct CMachineParameter**)malloc(sizeof(void*) * linx_buzz_machine_info.param_in_count);

	for (i = 0; i < linx_buzz_machine_info.param_in_count; i++) {
		struct linx_host_parameter* hostparam = &linx_buzz_machine_info.in_parameters[i];
		struct linx_pin_ref* pref = &linx_graph->propagated_pins[hostparam->extra_index];
		int resolved_pin_index;
		struct linx_vertex_definition* resolved_vertdef;
		struct linx_pin* extraparam = linx_graph_definition_resolve_pin(linx_graph, hostparam->extra_index, &resolved_vertdef, &resolved_pin_index);
		float default_value = linx_vertex_definition_get_pin_default_init_value(resolved_vertdef, resolved_pin_index);
		struct CMachineParameter* param = (struct CMachineParameter*)malloc(sizeof(struct CMachineParameter));
		param->Name = pref->name;
		param->Description = pref->name;
		param->MinValue = 0;
		param->MaxValue = 127;
		param->DefValue = linx_buzz_value_from_scalar(default_value, extraparam);
		param->NoValue = 255;
		param->Type = pt_byte;
		param->Flags = MPF_STATE;
		linx_buzz_machine_info.Parameters[i] = param;
	}

	return &linx_buzz_machine_info;
}

struct CMachineInterface* CreateMachine() {
	struct buzz_userdata* data = (struct buzz_userdata*)malloc(sizeof(struct buzz_userdata));

	linx_buzz_init_wrap();
	
	memset(data, 0, sizeof(struct buzz_userdata));
	data->mi = (struct CMachineInterface*)malloc(sizeof(struct CMachineInterface));
	data->miex = (struct CMachineInterfaceEx*)malloc(sizeof(struct CMachineInterfaceEx));

	data->info = GetInfo();
	data->CMachineInterface_trampolines = thiscall_object_new_trampolines(CMachineInterface_vtbl_count, CMachineInterface_vtbl);
	data->CMachineInterfaceEx_trampolines = thiscall_object_new_trampolines(CMachineInterfaceEx_vtbl_count, CMachineInterfaceEx_vtbl);

	memset(data->mi, 0, sizeof(struct CMachineInterface));
	data->mi->vtbl = thiscall_object_new_vtbl(CMachineInterface_vtbl_count, CMachineInterface_vtbl);

	// assume 0..127 per pin, host sets to default values and calls Tick()
	data->mi->GlobalVals = malloc(data->info->numGlobalParameters);

	memset(data->miex, 0, sizeof(struct CMachineInterfaceEx));
	data->miex->vtbl = thiscall_object_new_vtbl(CMachineInterfaceEx_vtbl_count, CMachineInterfaceEx_vtbl);

	data->mi->userdata = data;
	data->miex->userdata = data;

	data->graph = linx_graph;
	data->in_values = linx_value_array_create(max_value_queue_per_vertex);
	data->out_values = linx_value_array_create(max_value_queue_per_vertex);
	return data->mi;
}
