#define MAX_BUFFER_LENGTH		256

// machine types
#define MT_MASTER				0 
#define MT_GENERATOR			1
#define MT_EFFECT				2

// CMachineInfo flags
#define MIF_MONO_TO_STEREO		(1<<0)
#define MIF_PLAYS_WAVES			(1<<1)
#define MIF_USES_LIB_INTERFACE	(1<<2)
#define MIF_USES_INSTRUMENTS	(1<<3)
#define MIF_DOES_INPUT_MIXING	(1<<4)
#define MIF_NO_OUTPUT			(1<<5)
#define MIF_CONTROL_MACHINE		(1<<6)
#define MIF_INTERNAL_AUX		(1<<7)
#define MIF_EXTENDED_MENUS		(1<<8)
#define MIF_PATTERN_EDITOR		(1<<9)
#define MIF_PE_NO_CLIENT_EDGE	(1<<10)
#define MIF_GROOVE_CONTROL		(1<<11)
#define MIF_DRAW_PATTERN_BOX	(1<<12)
#define MIF_STEREO_EFFECT		(1<<13)
#define MIF_MULTI_IO			(1<<14)
#define MIF_PREFER_MIDI_NOTES	(1<<15)
#define MIF_LOAD_DATA_RUNTIME	(1<<16)
#define MIF_ALWAYS_SHOW_PLUGS	(1<<17)

// CMachineParameter flags
#define MPF_WAVE				1
#define MPF_STATE				2
#define MPF_TICK_ON_EDIT		4
#define MPF_TIE_TO_NEXT			8
#define MPF_ASCII				16

enum CMPType { pt_note, pt_switch, pt_byte, pt_word, pt_internal=127 };

struct CMachineParameter {
	enum CMPType Type;
	char const *Name;
	char const *Description;
	int MinValue;
	int MaxValue;
	int NoValue;
	int Flags;
	int DefValue;
};

struct CMachineInfo {
	int Type;
	int Version;
	int Flags;
	int minTracks;
	int maxTracks;
	int numGlobalParameters;
	int numTrackParameters;
	struct CMachineParameter** Parameters;
	int numAttributes;
	void **Attributes;
	char const *Name;
	char const *ShortName;
	char const *Author;
	char const *Commands;
	void *pLI;

	struct linx_host_parameter* in_parameters;
	struct linx_host_parameter* in_audios;
	struct linx_host_parameter* out_audios;
	struct linx_host_parameter* in_midis;
	int param_in_count;
	int audio_in_count;
	int audio_out_count;
	int midi_in_count;
};

struct CMasterInfo {
	int BeatsPerMin;
	int TicksPerBeat;
	int SamplesPerSec;
	int SamplesPerTick;
	int PosInTick;
	float TicksPerSec;
	int GrooveSize;
	int PosInGroove;
	float *GrooveData;
};

struct CMachineInterface {
	void* vtbl;
	void *GlobalVals;
	void *TrackVals;
	int *AttrVals;
	struct CMasterInfo* pMasterInfo;
	struct CMICallbacks* pCB;
	void* userdata;
};

struct CMachineInterfaceEx {
	void* vtbl;
	void* userdata;
};

typedef struct CMachineInterface* (CREATEMACHINE)();
typedef struct CMachineInfo* (GETINFO)();
