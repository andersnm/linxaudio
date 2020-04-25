var WobbleSynth = {
  "system": {
    "paths": [
      "lib"
    ],
    "uniqueId": "Y0Y9",
    "product": "WobbleSynth",
    "author": "Andy Wong"
  },
  "targets": [
    {
      "name": "plain-win32",
      "arch": "i386",
      "postfix": ".dll",
      "deps": [
        "msvcsupp.obj",
        "graph.obj",
        "ftol2.obj",
        "chkstk.obj",
        "memcpy.obj",
        "memset.obj"
      ],
      "exports": [
        "linx_host_get_graph"
      ]
    },
    {
      "name": "vst-win32",
      "arch": "i386",
      "postfix": "VST.dll",
      "deps": [
        "vsthost.obj",
        "commonhost.obj",
        "msvcsupp.obj",
        "graph.obj",
        "ftol2.obj",
        "chkstk.obj",
        "memcpy.obj",
        "memset.obj"
      ],
      "exports": [
        "VSTPluginMain",
        "linx_host_get_graph"
      ]
    },
    {
      "name": "buzz-win32",
      "arch": "i386",
      "postfix": "Buzz.dll",
      "deps": [
        "buzzhost.obj",
        "commonhost.obj",
        "msvcsupp.obj",
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
      ]
    }
  ],
  "graph": {
    "vertices": [
      {
        "name": "osc1",
        "factory": "oscbuffer",
        "subgraph": null,
        "values": {
          "Waveform": 2,
          "Amp": 0.64
        },
        "x": -0.0014641288433381305,
        "y": 0.033651190319057056
      },
      {
        "name": "filter",
        "factory": "filter",
        "subgraph": null,
        "values": {
          "Q": 1.58,
          "Type": 0,
          "Cutoff": 16444
        },
        "x": 0.0011505129059956598,
        "y": 0.5995635270287729
      },
      {
        "name": "midinote",
        "factory": "midinote",
        "subgraph": null,
        "values": {
          "MidiChannel": 0
        },
        "x": 0.03327769203203462,
        "y": -0.7434043595127747
      },
      {
        "name": "ampadsr",
        "factory": "adsrvalue",
        "subgraph": null,
        "values": {
          "Trigger": 0,
          "Release": 236
        },
        "x": -0.4330685216907686,
        "y": -0.07932747720899769
      },
      {
        "name": "osc1freq",
        "factory": "notefrequency",
        "subgraph": null,
        "values": {},
        "x": -0.0014641288433385746,
        "y": -0.20800979655450713
      },
      {
        "name": "osc1lfo",
        "factory": "lfovalue",
        "subgraph": null,
        "values": {
          "Frequency": 0.01,
          "Amp": 0.3
        },
        "x": -0.48023426061493424,
        "y": -0.6352830824536908
      },
      {
        "name": "osc2",
        "factory": "oscbuffer",
        "subgraph": null,
        "values": {
          "Phase": 0.5184,
          "Waveform": 2,
          "Amp": 0.85
        },
        "x": 0.23572474377745212,
        "y": 0.027281662790976657
      },
      {
        "name": "osc2freq",
        "factory": "notefrequency",
        "subgraph": null,
        "values": {
          "Detune": 0.1
        },
        "x": 0.23279648609077608,
        "y": -0.2057032579771294
      },
      {
        "name": "osc2lfo",
        "factory": "lfovalue",
        "subgraph": null,
        "values": {
          "Amp": 0.12
        },
        "x": 0.32650073206442154,
        "y": -0.40867355500683245
      },
      {
        "name": "cutoffadsr",
        "factory": "adsrvalue",
        "subgraph": null,
        "values": {
          "Attack": 38
        },
        "x": 0.5636896046852125,
        "y": -0.601993204246533
      },
      {
        "name": "cutoffparam",
        "factory": "uservalue_hertz",
        "subgraph": null,
        "values": {
          "In": 12703
        },
        "x": 0.7613469985358714,
        "y": -0.8168316831683169
      },
      {
        "name": "cutoffenv",
        "factory": "envscale",
        "subgraph": null,
        "values": {
          "EnvMod": 1,
          "UserMin": 33,
          "UserMax": 22050
        },
        "x": 0.5607613469985355,
        "y": 0.1831683168316831
      },
      {
        "name": "cutofflfo",
        "factory": "lfovalue",
        "subgraph": null,
        "values": {
          "Amp": 0.16,
          "Frequency": 0.941
        },
        "x": 0.7818448023426059,
        "y": -0.1831683168316831
      },
      {
        "name": "cutoffmix",
        "factory": "arithmeticvalue",
        "subgraph": null,
        "values": {},
        "x": 0.5329428989751099,
        "y": -0.23267326732673266
      },
      {
        "name": "osc1detune",
        "factory": "uservalue_fn1_1",
        "subgraph": null,
        "values": {
          "In": -0.03
        },
        "x": -0.25036603221083453,
        "y": -0.8613861386138614
      },
      {
        "name": "osc1tunemix",
        "factory": "arithmeticvalue",
        "subgraph": null,
        "values": {},
        "x": -0.2518301610541728,
        "y": -0.6534653465346535
      },
      {
        "name": "gain",
        "factory": "gain",
        "subgraph": null,
        "values": {
          "Amp": 0
        },
        "x": -0.18062083309357652,
        "y": 0.306930693069307
      }
    ],
    "edges": [
      {
        "from_vertex": "midinote",
        "from_pin": "OutTrigger",
        "to_vertex": "ampadsr",
        "to_pin": "Trigger"
      },
      {
        "from_vertex": "midinote",
        "from_pin": "OutNote",
        "to_vertex": "osc1freq",
        "to_pin": "In"
      },
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
        "from_vertex": "midinote",
        "from_pin": "OutNote",
        "to_vertex": "osc2freq",
        "to_pin": "In"
      },
      {
        "from_vertex": "osc2lfo",
        "from_pin": "Out",
        "to_vertex": "osc2freq",
        "to_pin": "Detune"
      },
      {
        "from_vertex": "midinote",
        "from_pin": "OutTrigger",
        "to_vertex": "cutoffadsr",
        "to_pin": "Trigger"
      },
      {
        "from_vertex": "cutoffparam",
        "from_pin": "Out",
        "to_vertex": "cutoffenv",
        "to_pin": "UserValue"
      },
      {
        "from_vertex": "cutoffenv",
        "from_pin": "OutValue",
        "to_vertex": "filter",
        "to_pin": "Cutoff"
      },
      {
        "from_vertex": "cutoffmix",
        "from_pin": "Out",
        "to_vertex": "cutoffenv",
        "to_pin": "EnvValue"
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
        "from_vertex": "gain",
        "from_pin": "Out",
        "to_vertex": "filter",
        "to_pin": "In"
      },
      {
        "from_vertex": "osc2",
        "from_pin": "Out",
        "to_vertex": "gain",
        "to_pin": "In"
      },
      {
        "from_vertex": "osc1",
        "from_pin": "Out",
        "to_vertex": "gain",
        "to_pin": "In"
      },
      {
        "from_vertex": "ampadsr",
        "from_pin": "Out",
        "to_vertex": "gain",
        "to_pin": "Amp"
      }
    ],
    "pins": [
      {
        "name": "MidiIn1",
        "vertex": "midinote",
        "pin": "MidiIn"
      },
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
        "name": "Out",
        "vertex": "filter",
        "pin": "Out"
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
    ]
  },
  "subgraphs": []
};
