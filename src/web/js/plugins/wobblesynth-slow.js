var WobbleSynth = {
  "system": {
    "uniqueId": "Y0Y9",
    "product": "WobbleSynth",
    "author": "Andy Wong"
  },
  "factories": [
    {
      "name": "oscbuffer",
      "symbol": "oscbuffer_factory",
      "file": "oscbuffer.obj",
      "deps": [
        "inertia.obj"
      ]
    },
    {
      "name": "notefrequency",
      "symbol": "notefrequency_factory",
      "file": "notefrequency.obj",
      "deps": []
    },
    {
      "name": "lfovalue",
      "symbol": "lfovalue_factory",
      "file": "lfovalue.obj",
      "deps": []
    },
    {
      "name": "uservalue_fn1_1",
      "symbol": "uservalue_fn1_1_factory",
      "file": "uservalue.obj",
      "deps": []
    },
    {
      "name": "arithmeticvalue",
      "symbol": "arithmeticvalue_factory",
      "file": "arithmeticvalue.obj",
      "deps": []
    },
    {
      "name": "midinote",
      "symbol": "midinotesplit_factory",
      "file": "midinotesplit.obj",
      "deps": []
    },
    {
      "name": "gain",
      "symbol": "gain_factory",
      "file": "gain.obj",
      "deps": [
        "inertia.obj"
      ]
    },
    {
      "name": "adsrvalue",
      "symbol": "adsrvalue_factory",
      "file": "adsrvalue.obj",
      "deps": []
    },
    {
      "name": "filter",
      "symbol": "biquad_filter_factory",
      "file": "biquad.obj",
      "deps": []
    },
    {
      "name": "uservalue_hertz",
      "symbol": "uservalue_hertz_factory",
      "file": "uservalue.obj",
      "deps": []
    },
    {
      "name": "envscale",
      "symbol": "envscale_factory",
      "file": "envscale.obj",
      "deps": []
    },
    {
      "name": "polycontainer",
      "symbol": "polycontainer_factory",
      "file": "polycontainer.obj",
      "deps": []
    }
  ],
  "targets": [
    {
      "name": "plain-win32",
      "arch": "i386",
      "postfix": ".dll",
      "deps": [
        "math_x86.obj",
        "graph.obj",
        "ftol2.obj",
        "chkstk.obj",
        "memcpy.obj",
        "memset.obj"
      ],
      "exports": [
        "linx_host_get_graph"
      ],
      "paths": [
        "lib/cl-x86"
      ]
    },
    {
      "name": "vst-win32",
      "arch": "i386",
      "postfix": "VST.dll",
      "deps": [
        "vsthost.obj",
        "commonhost.obj",
        "math_x86.obj",
        "graph.obj",
        "ftol2.obj",
        "chkstk.obj",
        "memcpy.obj",
        "memset.obj"
      ],
      "exports": [
        "VSTPluginMain",
        "linx_host_get_graph"
      ],
      "paths": [
        "lib/cl-x86"
      ]
    },
    {
      "name": "vst-win64",
      "arch": "x64",
      "postfix": "VST64.dll",
      "deps": [
        "vsthost.obj",
        "commonhost.obj",
        "math_sse.obj",
        "fminfmax.obj",
        "graph.obj",
        "chkstk.obj",
        "memcpy.obj",
        "memset.obj"
      ],
      "exports": [
        "VSTPluginMain",
        "linx_host_get_graph"
      ],
      "paths": [
        "lib/cl-x64"
      ]
    },
    {
      "name": "buzz-win32",
      "arch": "i386",
      "postfix": "Buzz.dll",
      "deps": [
        "buzzhost.obj",
        "commonhost.obj",
        "math_x86.obj",
        "graph.obj",
        "ftol2.obj",
        "chkstk.obj",
        "memcpy.obj",
        "memset.obj",
        "thiscall.obj",
        "mman.obj"
      ],
      "exports": [
        "GetInfo",
        "CreateMachine",
        "linx_host_get_graph"
      ],
      "paths": [
        "lib/cl-x86"
      ]
    }
  ],
  "graph": {
    "vertices": [
      {
        "name": "polyosc",
        "factory": "polycontainer",
        "subgraph": "subgraph0",
        "values": {},
        "x": -0.048316251830161305,
        "y": -0.7722363134542148
      }
    ],
    "edges": [],
    "pins": [
      {
        "name": "MidiIn1",
        "vertex": "polyosc",
        "pin": "Midi In"
      },
      {
        "name": "Type",
        "vertex": "polyosc",
        "pin": "Type"
      },
      {
        "name": "Cutoff",
        "vertex": "polyosc",
        "pin": "Cutoff"
      },
      {
        "name": "Resonance",
        "vertex": "polyosc",
        "pin": "Resonance"
      },
      {
        "name": "Out",
        "vertex": "polyosc",
        "pin": "Audio Out"
      },
      {
        "name": "Osc1Type",
        "vertex": "polyosc",
        "pin": "Osc1Type"
      },
      {
        "name": "Osc1Amp",
        "vertex": "polyosc",
        "pin": "Osc1Amp"
      },
      {
        "name": "Osc1Detune",
        "vertex": "polyosc",
        "pin": "Osc1Detune"
      },
      {
        "name": "Osc1LfoFreq",
        "vertex": "polyosc",
        "pin": "Osc1LfoFreq"
      },
      {
        "name": "Osc1LfoDepth",
        "vertex": "polyosc",
        "pin": "Osc1LfoDepth"
      },
      {
        "name": "Osc2Type",
        "vertex": "polyosc",
        "pin": "Osc2Type"
      },
      {
        "name": "Osc2Amp",
        "vertex": "polyosc",
        "pin": "Osc2Amp"
      },
      {
        "name": "Osc2LfoFreq",
        "vertex": "polyosc",
        "pin": "Osc2LfoFreq"
      },
      {
        "name": "Osc2LfoDepth",
        "vertex": "polyosc",
        "pin": "Osc2LfoDepth"
      },
      {
        "name": "Attack",
        "vertex": "polyosc",
        "pin": "Attack"
      },
      {
        "name": "Decay",
        "vertex": "polyosc",
        "pin": "Decay"
      },
      {
        "name": "Sustain",
        "vertex": "polyosc",
        "pin": "Sustain"
      },
      {
        "name": "Release",
        "vertex": "polyosc",
        "pin": "Release"
      },
      {
        "name": "FilterAttack",
        "vertex": "polyosc",
        "pin": "FilterAttack"
      },
      {
        "name": "FilterDecay",
        "vertex": "polyosc",
        "pin": "FilterDecay"
      },
      {
        "name": "FilterSustain",
        "vertex": "polyosc",
        "pin": "FilterSustain"
      },
      {
        "name": "FilterRelease",
        "vertex": "polyosc",
        "pin": "FilterRelease"
      },
      {
        "name": "FilterLfoFreq",
        "vertex": "polyosc",
        "pin": "FilterLfoFreq"
      },
      {
        "name": "FilterLfoDepth",
        "vertex": "polyosc",
        "pin": "FilterLfoDepth"
      },
      {
        "name": "FilterEnvMod",
        "vertex": "polyosc",
        "pin": "FilterEnvMod"
      }
    ]
  },
  "subgraphs": [
    {
      "vertices": [
        {
          "name": "osc1",
          "factory": "oscbuffer",
          "subgraph": null,
          "values": {
            "Waveform": 2,
            "Amp": 0.46
          },
          "x": -0.13762811127379193,
          "y": 0.15809563476350164
        },
        {
          "name": "osc1freq",
          "factory": "notefrequency",
          "subgraph": null,
          "values": {},
          "x": -0.14055636896046897,
          "y": -0.15912090766561826
        },
        {
          "name": "osc1lfo",
          "factory": "lfovalue",
          "subgraph": null,
          "values": {
            "Frequency": 0.031,
            "Amp": 0.04
          },
          "x": -0.6881405563689605,
          "y": -0.5871101678977976
        },
        {
          "name": "osc2",
          "factory": "oscbuffer",
          "subgraph": null,
          "values": {
            "Phase": 0.5184,
            "Waveform": 2,
            "Amp": 0.52
          },
          "x": 0.13762811127379204,
          "y": 0.1561705516798655
        },
        {
          "name": "osc2freq",
          "factory": "notefrequency",
          "subgraph": null,
          "values": {
            "Detune": 0.1
          },
          "x": 0.143484626647145,
          "y": -0.12570325797712933
        },
        {
          "name": "osc2lfo",
          "factory": "lfovalue",
          "subgraph": null,
          "values": {
            "Amp": 0.03
          },
          "x": 0.3587115666178622,
          "y": -0.4886735550068324
        },
        {
          "name": "osc1detune",
          "factory": "uservalue_fn1_1",
          "subgraph": null,
          "values": {
            "In": -0.35
          },
          "x": -0.6837481698389458,
          "y": -0.18314230207215232
        },
        {
          "name": "osc1tunemix",
          "factory": "arithmeticvalue",
          "subgraph": null,
          "values": {},
          "x": -0.40263543191800877,
          "y": -0.16984494802063788
        },
        {
          "name": "oscnote",
          "factory": "midinote",
          "subgraph": null,
          "values": {},
          "x": -0.024890190336749662,
          "y": -0.7325227963525835
        },
        {
          "name": "oscamp",
          "factory": "gain",
          "subgraph": null,
          "values": {
            "Amp": 0
          },
          "x": 0.4158125915080524,
          "y": 0.6493617021276596
        },
        {
          "name": "ampadsr",
          "factory": "adsrvalue",
          "subgraph": null,
          "values": {},
          "x": 0.44802342606149326,
          "y": -0.10816616008105373
        },
        {
          "name": "filter",
          "factory": "filter",
          "subgraph": null,
          "values": {
            "Q": 0.17,
            "Type": 0,
            "Cutoff": 16444
          },
          "x": 0.7947083459953077,
          "y": 0.6306746381398842
        },
        {
          "name": "cutoffadsr",
          "factory": "adsrvalue",
          "subgraph": null,
          "values": {
            "Attack": 38
          },
          "x": 0.5109809663250366,
          "y": -0.7212772299135374
        },
        {
          "name": "cutoffparam",
          "factory": "uservalue_hertz",
          "subgraph": null,
          "values": {
            "In": 10349
          },
          "x": 0.8770131771595902,
          "y": -0.8390539053905391
        },
        {
          "name": "cutoffenv",
          "factory": "envscale",
          "subgraph": null,
          "values": {
            "EnvMod": 1,
            "InMin": 0,
            "InMax": 1,
            "OutMin": 33,
            "OutMax": 22050
          },
          "x": 0.6837481698389456,
          "y": 0.3931244127452258
        },
        {
          "name": "cutofflfo",
          "factory": "lfovalue",
          "subgraph": null,
          "values": {
            "Amp": 0.28,
            "Frequency": 0.641
          },
          "x": 0.7203513909224011,
          "y": -0.405282467456472
        },
        {
          "name": "cutoffmix",
          "factory": "arithmeticvalue",
          "subgraph": null,
          "values": {},
          "x": 0.6939970717423136,
          "y": -0.1416904912375736
        },
        {
          "name": "cutoffscale",
          "factory": "envscale",
          "subgraph": null,
          "values": {
            "InValue": 0,
            "InMin": 33,
            "InMax": 22050
          },
          "x": 0.8770131771595902,
          "y": -0.5695643363728469
        },
        {
          "name": "cutoffmix2",
          "factory": "arithmeticvalue",
          "subgraph": null,
          "values": {
            "Type": 2
          },
          "x": 0.8096632503660322,
          "y": 0.1450050658561297
        },
        {
          "name": "ampdiv2",
          "factory": "arithmeticvalue",
          "subgraph": null,
          "values": {
            "Rhs": 2,
            "Type": 3
          },
          "x": 0.4421669106881405,
          "y": 0.2088888888888889
        }
      ],
      "edges": [
        {
          "from_vertex": "osc1freq",
          "from_pin": "OutFreq",
          "to_vertex": "osc1",
          "to_pin": "Frequency"
        },
        {
          "from_vertex": "osc2freq",
          "from_pin": "OutFreq",
          "to_vertex": "osc2",
          "to_pin": "Frequency"
        },
        {
          "from_vertex": "osc2lfo",
          "from_pin": "Out",
          "to_vertex": "osc2freq",
          "to_pin": "Detune"
        },
        {
          "from_vertex": "osc1tunemix",
          "from_pin": "Out",
          "to_vertex": "osc1freq",
          "to_pin": "Detune"
        },
        {
          "from_vertex": "osc1lfo",
          "from_pin": "Out",
          "to_vertex": "osc1tunemix",
          "to_pin": "Rhs"
        },
        {
          "from_vertex": "osc1detune",
          "from_pin": "Out",
          "to_vertex": "osc1tunemix",
          "to_pin": "Lhs"
        },
        {
          "from_vertex": "PARENT",
          "from_pin": "From Midi In",
          "to_vertex": "oscnote",
          "to_pin": "MidiIn"
        },
        {
          "from_vertex": "oscnote",
          "from_pin": "OutNote",
          "to_vertex": "osc1freq",
          "to_pin": "In"
        },
        {
          "from_vertex": "oscnote",
          "from_pin": "OutNote",
          "to_vertex": "osc2freq",
          "to_pin": "In"
        },
        {
          "from_vertex": "oscnote",
          "from_pin": "OutTrigger",
          "to_vertex": "ampadsr",
          "to_pin": "Trigger"
        },
        {
          "from_vertex": "cutofflfo",
          "from_pin": "Out",
          "to_vertex": "cutoffmix",
          "to_pin": "Rhs"
        },
        {
          "from_vertex": "cutoffadsr",
          "from_pin": "Out",
          "to_vertex": "cutoffmix",
          "to_pin": "Lhs"
        },
        {
          "from_vertex": "cutoffparam",
          "from_pin": "Out",
          "to_vertex": "cutoffscale",
          "to_pin": "InValue"
        },
        {
          "from_vertex": "cutoffmix",
          "from_pin": "Out",
          "to_vertex": "cutoffmix2",
          "to_pin": "Lhs"
        },
        {
          "from_vertex": "cutoffscale",
          "from_pin": "OutValue",
          "to_vertex": "cutoffmix2",
          "to_pin": "Rhs"
        },
        {
          "from_vertex": "cutoffmix2",
          "from_pin": "Out",
          "to_vertex": "cutoffenv",
          "to_pin": "InValue"
        },
        {
          "from_vertex": "cutoffenv",
          "from_pin": "OutValue",
          "to_vertex": "filter",
          "to_pin": "Cutoff"
        },
        {
          "from_vertex": "oscamp",
          "from_pin": "Out",
          "to_vertex": "filter",
          "to_pin": "In"
        },
        {
          "from_vertex": "filter",
          "from_pin": "Out",
          "to_vertex": "PARENT",
          "to_pin": "To Audio Out"
        },
        {
          "from_vertex": "oscnote",
          "from_pin": "OutTrigger",
          "to_vertex": "cutoffadsr",
          "to_pin": "Trigger"
        },
        {
          "from_vertex": "ampadsr",
          "from_pin": "Out",
          "to_vertex": "ampdiv2",
          "to_pin": "Lhs"
        },
        {
          "from_vertex": "ampdiv2",
          "from_pin": "Out",
          "to_vertex": "oscamp",
          "to_pin": "Amp"
        },
        {
          "from_vertex": "osc1",
          "from_pin": "Out",
          "to_vertex": "oscamp",
          "to_pin": "In"
        },
        {
          "from_vertex": "osc2",
          "from_pin": "Out",
          "to_vertex": "oscamp",
          "to_pin": "In"
        }
      ],
      "pins": [
        {
          "name": "Type",
          "vertex": "filter",
          "pin": "Type"
        },
        {
          "name": "Cutoff",
          "vertex": "cutoffparam",
          "pin": "In"
        },
        {
          "name": "Resonance",
          "vertex": "filter",
          "pin": "Q"
        },
        {
          "name": "Osc1Type",
          "vertex": "osc1",
          "pin": "Waveform"
        },
        {
          "name": "Osc1Amp",
          "vertex": "osc1",
          "pin": "Amp"
        },
        {
          "name": "Osc1Detune",
          "vertex": "osc1detune",
          "pin": "In"
        },
        {
          "name": "Osc1LfoFreq",
          "vertex": "osc1lfo",
          "pin": "Frequency"
        },
        {
          "name": "Osc1LfoDepth",
          "vertex": "osc1lfo",
          "pin": "Amp"
        },
        {
          "name": "Osc2Type",
          "vertex": "osc2",
          "pin": "Waveform"
        },
        {
          "name": "Osc2Amp",
          "vertex": "osc2",
          "pin": "Amp"
        },
        {
          "name": "Osc2LfoFreq",
          "vertex": "osc2lfo",
          "pin": "Frequency"
        },
        {
          "name": "Osc2LfoDepth",
          "vertex": "osc2lfo",
          "pin": "Amp"
        },
        {
          "name": "OutTrigger",
          "vertex": "oscnote",
          "pin": "OutTrigger"
        },
        {
          "name": "OutNote",
          "vertex": "oscnote",
          "pin": "OutNote"
        },
        {
          "name": "Attack",
          "vertex": "ampadsr",
          "pin": "Attack"
        },
        {
          "name": "Decay",
          "vertex": "ampadsr",
          "pin": "Decay"
        },
        {
          "name": "Sustain",
          "vertex": "ampadsr",
          "pin": "Sustain"
        },
        {
          "name": "Release",
          "vertex": "ampadsr",
          "pin": "Release"
        },
        {
          "name": "FilterAttack",
          "vertex": "cutoffadsr",
          "pin": "Attack"
        },
        {
          "name": "FilterDecay",
          "vertex": "cutoffadsr",
          "pin": "Decay"
        },
        {
          "name": "FilterSustain",
          "vertex": "cutoffadsr",
          "pin": "Sustain"
        },
        {
          "name": "FilterRelease",
          "vertex": "cutoffadsr",
          "pin": "Release"
        },
        {
          "name": "FilterLfoFreq",
          "vertex": "cutofflfo",
          "pin": "Frequency"
        },
        {
          "name": "FilterLfoDepth",
          "vertex": "cutofflfo",
          "pin": "Amp"
        },
        {
          "name": "FilterEnvMod",
          "vertex": "cutoffenv",
          "pin": "EnvMod"
        }
      ],
      "name": "subgraph0"
    }
  ]
};
