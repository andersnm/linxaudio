var SimpleSynth = {
  "system": {
    "uniqueId": "Y0Y1",
    "product": "SimpleSynth",
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
      "name": "filter",
      "symbol": "biquad_filter_factory",
      "file": "biquad.obj",
      "deps": []
    },
    {
      "name": "midinote",
      "symbol": "midinotesplit_factory",
      "file": "midinotesplit.obj",
      "deps": []
    },
    {
      "name": "adsrvalue",
      "symbol": "adsrvalue_factory",
      "file": "adsrvalue.obj",
      "deps": []
    },
    {
      "name": "arithmeticvalue",
      "symbol": "arithmeticvalue_factory",
      "file": "arithmeticvalue.obj",
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
        "name": "polycontainer",
        "factory": "polycontainer",
        "subgraph": "subgraph0",
        "values": {},
        "x": -0.017569546120058566,
        "y": -0.13286713286713292
      }
    ],
    "edges": [],
    "pins": [
      {
        "name": "MidiIn",
        "vertex": "polycontainer",
        "pin": "Midi In"
      },
      {
        "name": "Out",
        "vertex": "polycontainer",
        "pin": "Audio Out"
      },
      {
        "name": "Wave",
        "vertex": "polycontainer",
        "pin": "Wave"
      },
      {
        "name": "Type",
        "vertex": "polycontainer",
        "pin": "Type"
      },
      {
        "name": "Cutoff",
        "vertex": "polycontainer",
        "pin": "Cutoff"
      },
      {
        "name": "Resonance",
        "vertex": "polycontainer",
        "pin": "Resonance"
      },
      {
        "name": "Attack",
        "vertex": "polycontainer",
        "pin": "Attack"
      },
      {
        "name": "Decay",
        "vertex": "polycontainer",
        "pin": "Decay"
      },
      {
        "name": "Sustain",
        "vertex": "polycontainer",
        "pin": "Sustain"
      },
      {
        "name": "Release",
        "vertex": "polycontainer",
        "pin": "Release"
      }
    ]
  },
  "subgraphs": [
    {
      "vertices": [
        {
          "name": "osc",
          "factory": "oscbuffer",
          "subgraph": null,
          "values": {
            "Waveform": 2,
            "Amp": 0
          },
          "x": 0.1976573938506586,
          "y": 0.21328671328671334
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
          "x": -0.007717915581435364,
          "y": -0.5443686370476135
        },
        {
          "name": "adsr",
          "factory": "adsrvalue",
          "subgraph": null,
          "values": {},
          "x": -0.3759674968005784,
          "y": -0.11821877553979909
        },
        {
          "name": "ampscale",
          "factory": "arithmeticvalue",
          "subgraph": null,
          "values": {
            "Type": 2,
            "Rhs": 0
          },
          "x": -0.12445095168374842,
          "y": 0.034965034965035224
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
          "to_vertex": "ampscale",
          "to_pin": "Lhs"
        },
        {
          "from_vertex": "midinote",
          "from_pin": "OutVelo",
          "to_vertex": "ampscale",
          "to_pin": "Rhs"
        },
        {
          "from_vertex": "ampscale",
          "from_pin": "Out",
          "to_vertex": "osc",
          "to_pin": "Amp"
        },
        {
          "from_vertex": "PARENT",
          "from_pin": "From Midi In",
          "to_vertex": "midinote",
          "to_pin": "MidiIn"
        },
        {
          "from_vertex": "filter",
          "from_pin": "Out",
          "to_vertex": "PARENT",
          "to_pin": "To Audio Out"
        }
      ],
      "pins": [
        {
          "name": "Wave",
          "vertex": "osc",
          "pin": "Waveform"
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
      ],
      "name": "subgraph0"
    }
  ]
}