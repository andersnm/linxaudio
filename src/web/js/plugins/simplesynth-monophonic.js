var SimpleSynth = {
  "system": {
    "paths": [
      "lib"
    ],
    "uniqueId": "Y0Y1",
    "product": "SimpleSynth",
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
        "name": "osc",
        "factory": "oscbuffer",
        "subgraph": null,
        "values": {
          "Waveform": 2
        },
        "x": 0,
        "y": 0.5
      },
      {
        "name": "filter",
        "factory": "filter",
        "subgraph": null,
        "values": {},
        "x": 0.47552825814757677,
        "y": 0.15450849718747373
      },
      {
        "name": "midinote",
        "factory": "midinote",
        "subgraph": null,
        "values": {},
        "x": 0.2938926261462366,
        "y": -0.40450849718747367
      },
      {
        "name": "gain",
        "factory": "gain",
        "subgraph": null,
        "values": {
          "Amp": 0
        },
        "x": -0.2938926261462365,
        "y": -0.4045084971874738
      },
      {
        "name": "adsr",
        "factory": "adsrvalue",
        "subgraph": null,
        "values": {},
        "x": -0.4755282581475768,
        "y": 0.15450849718747361
      }
    ],
    "edges": [
      {
        "from_vertex": "osc",
        "from_pin": "Out",
        "to_vertex": "filter",
        "to_pin": "In"
      },
      {
        "from_vertex": "midinote",
        "from_pin": "OutFreq",
        "to_vertex": "osc",
        "to_pin": "Frequency"
      },
      {
        "from_vertex": "midinote",
        "from_pin": "OutTrigger",
        "to_vertex": "adsr",
        "to_pin": "Trigger"
      },
      {
        "from_vertex": "adsr",
        "from_pin": "Out",
        "to_vertex": "gain",
        "to_pin": "Amp"
      },
      {
        "from_vertex": "filter",
        "from_pin": "Out",
        "to_vertex": "gain",
        "to_pin": "In"
      }
    ],
    "pins": [
      {
        "name": "Wave",
        "vertex": "osc",
        "pin": "Waveform"
      },
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
        "vertex": "filter",
        "pin": "Cutoff"
      },
      {
        "name": "Resonance",
        "vertex": "filter",
        "pin": "Q"
      },
      {
        "name": "Out",
        "vertex": "gain",
        "pin": "Out"
      },
      {
        "name": "Attack",
        "vertex": "adsr",
        "pin": "Attack"
      },
      {
        "name": "Decay",
        "vertex": "adsr",
        "pin": "Decay"
      },
      {
        "name": "Sustain",
        "vertex": "adsr",
        "pin": "Sustain"
      },
      {
        "name": "Release",
        "vertex": "adsr",
        "pin": "Release"
      }
    ]
  },
  "subgraphs": []
};
