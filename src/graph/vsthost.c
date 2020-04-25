#include <stdint.h> 
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "linxaudio.h"
#include "commonhost.h"

#define DECLARE_VST_DEPRECATED(x) x

struct AEffect;

typedef	intptr_t (*audioMasterCallback) (struct AEffect* effect, int32_t opcode, int32_t index, intptr_t value, void* ptr, float opt);
typedef intptr_t (*AEffectDispatcherProc) (struct AEffect* effect, int32_t opcode, int32_t index, intptr_t value, void* ptr, float opt);
typedef void (*AEffectProcessProc) (struct AEffect* effect, float** inputs, float** outputs, int32_t sampleFrames);
typedef void (*AEffectProcessDoubleProc) (struct AEffect* effect, double** inputs, double** outputs, int32_t sampleFrames);
typedef void (*AEffectSetParameterProc) (struct AEffect* effect, int32_t index, float parameter);
typedef float (*AEffectGetParameterProc) (struct AEffect* effect, int32_t index);

#define kEffectMagic ('V', 's', 't', 'P')

struct AEffect {
	int32_t magic;
	AEffectDispatcherProc dispatcher;
	AEffectProcessProc DECLARE_VST_DEPRECATED (process);
	AEffectSetParameterProc setParameter;
	AEffectGetParameterProc getParameter;
	int32_t numPrograms;
	int32_t numParams;
	int32_t numInputs;
	int32_t numOutputs;
	int32_t flags;
	intptr_t resvd1;
	intptr_t resvd2;
	int32_t initialDelay;
	int32_t DECLARE_VST_DEPRECATED (realQualities);
	int32_t DECLARE_VST_DEPRECATED (offQualities);
	float DECLARE_VST_DEPRECATED (ioRatio);
	void* object;
	void* user;
	int32_t uniqueID;
	int32_t version;
	AEffectProcessProc processReplacing;
#if VST_2_4_EXTENSIONS
	AEffectProcessDoubleProc processDoubleReplacing;	
	char future[56];
#else
	char future[60];
#endif
//-------------------------------------------------------------------------------------------------------
};

enum AEffectOpcodes {
	effOpen = 0,
	effClose,
	effSetProgram,
	effGetProgram,
	effSetProgramName,
	effGetProgramName,
	effGetParamLabel,
	effGetParamDisplay,
	effGetParamName,
	DECLARE_VST_DEPRECATED(effGetVu),
	effSetSampleRate,
	effSetBlockSize,
	effMainsChanged,
	effEditGetRect,
	effEditOpen,
	effEditClose,
	DECLARE_VST_DEPRECATED (effEditDraw),
	DECLARE_VST_DEPRECATED (effEditMouse),
	DECLARE_VST_DEPRECATED (effEditKey),
	effEditIdle,
	DECLARE_VST_DEPRECATED (effEditTop),
	DECLARE_VST_DEPRECATED (effEditSleep),
	DECLARE_VST_DEPRECATED (effIdentify),
	effGetChunk,
	effSetChunk,
	effNumOpcodes,

	effProcessEvents = effSetChunk + 1,
	effCanBeAutomated,
	effString2Parameter,
	DECLARE_VST_DEPRECATED (effGetNumProgramCategories),
	effGetProgramNameIndexed,
	DECLARE_VST_DEPRECATED (effCopyProgram),
	DECLARE_VST_DEPRECATED (effConnectInput),
	DECLARE_VST_DEPRECATED (effConnectOutput),
	effGetInputProperties,
	effGetOutputProperties,
	effGetPlugCategory,
	DECLARE_VST_DEPRECATED (effGetCurrentPosition),
	DECLARE_VST_DEPRECATED (effGetDestinationBuffer),
	effOfflineNotify,
	effOfflinePrepare,
	effOfflineRun,
	effProcessVarIo,
	effSetSpeakerArrangement,
	DECLARE_VST_DEPRECATED (effSetBlockSizeAndSampleRate),
	effSetBypass,
	effGetEffectName,
	DECLARE_VST_DEPRECATED (effGetErrorText),
	effGetVendorString,
	effGetProductString,
	effGetVendorVersion,
	effVendorSpecific,
	effCanDo,
	effGetTailSize,
	DECLARE_VST_DEPRECATED (effIdle),
	DECLARE_VST_DEPRECATED (effGetIcon),
	DECLARE_VST_DEPRECATED (effSetViewPosition),
	effGetParameterProperties,
	DECLARE_VST_DEPRECATED (effKeysRequired),
	effGetVstVersion,

#if VST_2_1_EXTENSIONS
	effEditKeyDown,
	effEditKeyUp,
	effSetEditKnobMode,
	effGetMidiProgramName,
	effGetCurrentMidiProgram,
	effGetMidiProgramCategory,
	effHasMidiProgramsChanged,
	effGetMidiKeyName,
	effBeginSetProgram,
	effEndSetProgram,
#endif

#if VST_2_3_EXTENSIONS
	effGetSpeakerArrangement,
	effShellGetNextPlugin,
	effStartProcess,
	effStopProcess,
	effSetTotalSampleToProcess,
	effSetPanLaw,
	effBeginLoadBank,
	effBeginLoadProgram,
#endif

#if VST_2_4_EXTENSIONS
	effSetProcessPrecision,
	effGetNumMidiInputChannels,
	effGetNumMidiOutputChannels
#endif
};

enum AudioMasterOpcodes {
	audioMasterAutomate = 0,
	audioMasterVersion,
	audioMasterCurrentId,
	audioMasterIdle,
	audioMasterPinConnected
};

struct VstEvent {
	int32_t type;
	int32_t byteSize;
	int32_t deltaFrames;
	int32_t flags;
	char data[16];
};

enum VstEventTypes {
	kVstMidiType = 1,
	DECLARE_VST_DEPRECATED (kVstAudioType),
	DECLARE_VST_DEPRECATED (kVstVideoType),
	DECLARE_VST_DEPRECATED (kVstParameterType),
	DECLARE_VST_DEPRECATED (kVstTriggerType),
	kVstSysExType
};

struct VstEvents {
	int32_t numEvents;
	intptr_t reserved;
	struct VstEvent* events[2];
};

struct VstMidiEvent {
	int32_t type;
	int32_t byteSize;
	int32_t deltaFrames;
	int32_t flags;
	int32_t noteLength;
	int32_t noteOffset;
	char midiData[4];
	char detune;
	char noteOffVelocity;
	char reserved1;
	char reserved2;
};

// 
// END OF VST SDK
// 

struct vst_userdata {
	audioMasterCallback audioMaster;
	struct linx_graph_definition* graph;
	struct linx_graph_instance* graph_data;
	struct linx_value_array* in_values;
	struct linx_value_array* out_values;
	struct linx_host_parameter* in_parameters;
	int param_in_count;
	struct linx_host_parameter* in_midis;
	int audio_in_count;
	struct linx_host_parameter* in_audios;
	int audio_out_count;
	struct linx_host_parameter* out_audios;
	int midi_in_count;
	int samplerate;
	float* state_values;
};

intptr_t linx_vst_can_do(struct AEffect* effect, const char* cando) {
	// effCanDo x[return]: 1='cando', 0='don't know' (default), -1='No'
	/*if (strcmp((const char*)ptr, "sendVstMidiEvents")) {
		return 1;
	} else if (strcmp((const char*)ptr, "sendVstEvents")) {
		return 1;
	}*/
	return 0;
}

void linx_vst_process_events(struct AEffect* effect, struct VstEvents* events) {
	int i, j;
	int extra_index;
	struct vst_userdata* userdata = (struct vst_userdata*)effect->user;
	struct VstMidiEvent* midi_event;

	assert(userdata != 0);
	for (i = 0; i < events->numEvents; i++) {
		if (events->events[i]->type == kVstMidiType) {
			midi_event = (struct VstMidiEvent*)events->events[i];
			for (j = 0; j < userdata->midi_in_count; j++) {
				extra_index = userdata->in_midis[j].extra_index;

				linx_value_array_push_midi(userdata->in_values, extra_index,  linx_pin_group_propagated, *(unsigned int*)midi_event->midiData, 0);
			}
		}
	}
}

void linx_vst_describe_value(struct vst_userdata* data, char* str, int index) { 

	if (data->graph_data != 0) {
		int pin_index = data->in_parameters[index].extra_index;
		struct linx_pin* pin = &data->graph_data->propagated_pins[pin_index];
		float value = data->state_values[index];

		linx_graph_instance_describe_value(data->graph_data, pin_index, value, str, 64 - 1);

		if (strlen(str) == 0) {
			if (pin->pin_type == linx_pin_type_in_scalar_float) {
				snprintf(str, 64 -1, "%0.2f", value);
			} else {
				snprintf(str, 64 -1, "%i", (int)value);
			}
		}
	}
}

void linx_vst_init_state(struct vst_userdata* userdata) {
	int i;
	assert(userdata->graph_data != 0);
	for (i = 0; i < userdata->param_in_count; i++) {
		int pin_index = userdata->in_parameters[i].extra_index;
		userdata->state_values[i] = userdata->graph_data->propagated_pins[pin_index].default_value;//linx_graph_get_value(userdata->graph_data, userdata->in_parameters[i].extra_index);
	}
}

/* vst entry point */
intptr_t linx_vst_dispatcher(struct AEffect* effect, int32_t opcode, int32_t index, intptr_t value, void* ptr, float opt) {
	struct vst_userdata* userdata = (struct vst_userdata*)effect->user;
	assert(userdata != 0);

	switch (opcode) {
		case effCanDo:
			return linx_vst_can_do(effect, (const char*)ptr);
		case effGetVendorString:
			strcpy((char*)ptr, linx_graph_author);
			return 1;
		case effGetProductString:
			strcpy((char*)ptr, linx_graph_product);
			return 1;
		case effGetParamName:
			strcpy((char*)ptr, userdata->graph->propagated_pins[userdata->in_parameters[index].extra_index].name);
			return 1;
		case effGetParamLabel:
			linx_vst_describe_value(userdata, (char*)ptr, index);
			return 1;
		case effOpen:
			break;
		case effClose:
			break;
		case effMainsChanged:
			// = recreate graph w/new samplerate
			if (value == 0 && userdata->graph_data != 0) {
				// TODO: delete userdata->graph
			} else if (value == 1 && userdata->graph_data == 0) {
				userdata->graph_data = linx_graph_definition_create_instance(userdata->graph, userdata->samplerate);
				userdata->in_values = linx_value_array_create(max_value_queue_per_vertex);
				userdata->out_values = linx_value_array_create(max_value_queue_per_vertex);
				linx_vst_init_state(userdata);
			}
			return 0;
		case effSetSampleRate:
			userdata->samplerate = (int)opt;
			break;
		case effSetBlockSize:
			break;

		case effProcessEvents:
			// midi from host to plugin
			// send to userdata->in_midis parameters
			linx_vst_process_events(effect, (struct VstEvents*)ptr);
			break;
	}
	return 0;
}

float linx_vst_value_from_scalar(float value, struct linx_pin* pin) {
	float param_scale = pin->max_value - pin->min_value;
	float result = (value - pin->min_value) / param_scale;
	return result;
}

float linx_scalar_from_vst_value(float value, struct linx_pin* pin) {
	float param_scale = pin->max_value - pin->min_value;
	float result = value * param_scale + pin->min_value;
	if (result > pin->max_value) {
		result = pin->max_value;
	} else if (result < pin->min_value) {
		result = pin->min_value;
	}
	return result;
}

void linx_vst_set_parameter(struct AEffect* effect, int32_t index, float value) {
	struct vst_userdata* userdata = (struct vst_userdata*)effect->user;
	int pin_index;
	struct linx_pin* pin;

	assert(userdata != 0);
	assert(userdata->graph_data != 0);
	assert(index < userdata->param_in_count);

	pin_index = userdata->in_parameters[index].extra_index;
	pin = &userdata->graph_data->propagated_pins[pin_index];

	value = linx_scalar_from_vst_value(value, pin);

	if (pin->pin_type == linx_pin_type_in_scalar_int) {
		linx_value_array_push_int(userdata->in_values, pin_index, linx_pin_group_propagated, (int)value, 0);
	} else if (pin->pin_type == linx_pin_type_in_scalar_float) {
		linx_value_array_push_float(userdata->in_values, pin_index, linx_pin_group_propagated, value, 0);
	} else {
		assert(0);
	}

	userdata->state_values[index] = value;
}

float linx_vst_get_parameter(struct AEffect* effect, int32_t index) {
	struct vst_userdata* userdata = (struct vst_userdata*)effect->user;
	float value;
	int pin_index;
	int resolved_pin_index;
	struct linx_vertex_definition* resolved_vertdef;
	struct linx_pin* pin;

	assert(userdata != 0);
	assert(userdata->graph != 0);
	assert(index < userdata->param_in_count);

	pin_index = userdata->in_parameters[index].extra_index;
	pin = linx_graph_definition_resolve_pin(userdata->graph, pin_index, &resolved_vertdef, &resolved_pin_index);

	if (userdata->graph_data != 0) {
		value = userdata->state_values[index];
		// OR: linx_graph_instance_get_current_value
	} else {
		// hosts can probe for default parameter value before any instances are created 
		value = linx_vertex_definition_get_pin_default_init_value(resolved_vertdef, resolved_pin_index);
	}
	return linx_vst_value_from_scalar(value, pin);
}

void linx_vst_process_replacing(struct AEffect* effect, float** inputs, float** outputs, int32_t sampleFrames) {
	int i;
	struct vst_userdata* userdata = (struct vst_userdata*)effect->user;
	assert(userdata != 0);

	if (userdata->graph_data == 0) {
		return ;
	}

	// copy input from the host into graph audio input extra pins

	for (i = 0; i < userdata->audio_in_count; i++) {
		if (inputs && inputs[i] != 0) {
			struct linx_host_parameter* vstp = &userdata->in_audios[i];
			struct linx_buffer* vstbuf = linx_graph_instance_get_propagated_pin_buffer(userdata->graph_data, vstp->extra_index);
			linx_buffer_write(vstbuf, inputs[i], sampleFrames);
		}
	}

	linx_graph_instance_process(userdata->graph_data, 0, userdata->in_values, userdata->out_values, sampleFrames);

	for (i = 0; i < userdata->audio_out_count; i++) {
		if (outputs && outputs[i]) {
			struct linx_host_parameter* vstp = &userdata->out_audios[i];
			struct linx_buffer* vstbuf = linx_graph_instance_get_propagated_pin_buffer(userdata->graph_data, vstp->extra_index);
			linx_buffer_read(outputs[i], vstbuf, sampleFrames);
		}
	}

	linx_graph_instance_process_clear(userdata->graph_data);
	userdata->in_values->length = 0;
	userdata->out_values->length = 0;
}

void linx_vst_process(struct AEffect* effect, float** inputs, float** outputs, int32_t sampleFrames) {
	assert(0);
}

struct AEffect* VSTPluginMain(audioMasterCallback audioMaster) {
	struct AEffect* result;
	struct vst_userdata* userdata;
	
	userdata = (struct vst_userdata*)malloc(sizeof(struct vst_userdata));
	userdata->audioMaster = audioMaster;
	userdata->graph = linx_graph;
	userdata->graph_data = 0;
	userdata->samplerate = 44100;

	linx_host_get_pins(linx_graph,
		&userdata->param_in_count, &userdata->in_parameters, 
		&userdata->audio_in_count, &userdata->in_audios, 
		&userdata->audio_out_count, &userdata->out_audios, 
		&userdata->midi_in_count, &userdata->in_midis);

	userdata->state_values = (float*)malloc(userdata->param_in_count * sizeof(float));

	result = (struct AEffect*)malloc(sizeof(struct AEffect));
	memset(result, 0, sizeof(struct AEffect));
	result->magic = kEffectMagic;
	result->version = 0x1000;
	result->uniqueID = linx_graph_uniqueid;
	result->user = userdata;
	result->numParams = userdata->param_in_count;
	result->numInputs = userdata->audio_in_count;
	result->numOutputs = userdata->audio_out_count;
	// reply to vst canMidiIn/Out based on in/out_midi extra_parameters 
	result->dispatcher = linx_vst_dispatcher;
	result->process = linx_vst_process; 
	result->processReplacing = linx_vst_process_replacing; 
	result->getParameter = linx_vst_get_parameter;
	result->setParameter = linx_vst_set_parameter;

	// TODO? according to this thread, there exist "a host" which crashes
	// unless result->object points at an instance of the VST SDK C++ class
	// AudioEffectX: http://www.kvraudio.com/forum/viewtopic.php?t=168353
	return result;
}
