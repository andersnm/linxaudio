var SimpleKick = {
  "system": {
    "paths": [
      "lib"
    ],
    "uniqueId": "NOID",
    "product": "SimpleKick",
    "author": "andy",
    "myjsonId": "4jmpy"
  },
  "graph": {
    "vertices": [
      {
        "name": "oscbuffer",
        "factory": "oscbuffer",
        "subgraph": null,
        "values": {
          "Amp": 0
        },
        "x": 0.008784773060029227,
        "y": 0.41512605042016837
      },
      {
        "name": "ampadsr",
        "factory": "adsrvalue",
        "subgraph": null,
        "values": {
          "Attack": 0,
          "Decay": 324,
          "Sustain": 0,
          "Release": 53
        },
        "x": -0.32210834553440704,
        "y": -0.20000000000000007
      },
      {
        "name": "pitchadsr",
        "factory": "adsrvalue",
        "subgraph": null,
        "values": {
          "Release": 0,
          "Sustain": 1,
          "Attack": 563,
          "Decay": 0
        },
        "x": 0.13762811127379193,
        "y": -0.5159663865546218
      },
      {
        "name": "notefrequency",
        "factory": "notefrequency",
        "subgraph": null,
        "values": {},
        "x": 0.15080527086383622,
        "y": 0.03865546218487381
      },
      {
        "name": "midinote",
        "factory": "midinote",
        "subgraph": null,
        "values": {},
        "x": 0.005856515373352744,
        "y": -0.811764705882353
      },
      {
        "name": "PitchDiff",
        "factory": "arithmeticvalue",
        "subgraph": null,
        "values": {
          "Type": 1
        },
        "x": 0.5402635431918008,
        "y": -0.5226890756302519
      },
      {
        "name": "StartPitch",
        "factory": "uservalue_u7",
        "subgraph": null,
        "values": {
          "In": 57
        },
        "x": 0.3660322108345536,
        "y": -0.8016806722689076
      },
      {
        "name": "EndPitch",
        "factory": "uservalue_u7",
        "subgraph": null,
        "values": {
          "In": 29
        },
        "x": 0.6764275256222547,
        "y": -0.788235294117647
      },
      {
        "name": "PitchScale",
        "factory": "arithmeticvalue",
        "subgraph": null,
        "values": {
          "Type": 2
        },
        "x": 0.5431918008784773,
        "y": -0.28403361344537825
      },
      {
        "name": "PitchSum",
        "factory": "arithmeticvalue",
        "subgraph": null,
        "values": {},
        "x": 0.430453879941435,
        "y": 0.04201680672268915
      }
    ],
    "edges": [
      {
        "from_vertex": "ampadsr",
        "from_pin": "Out",
        "to_vertex": "oscbuffer",
        "to_pin": "Amp"
      },
      {
        "from_vertex": "notefrequency",
        "from_pin": "OutFreq",
        "to_vertex": "oscbuffer",
        "to_pin": "Frequency"
      },
      {
        "from_vertex": "midinote",
        "from_pin": "OutTrigger",
        "to_vertex": "ampadsr",
        "to_pin": "Trigger"
      },
      {
        "from_vertex": "midinote",
        "from_pin": "OutTrigger",
        "to_vertex": "pitchadsr",
        "to_pin": "Trigger"
      },
      {
        "from_vertex": "EndPitch",
        "from_pin": "Out",
        "to_vertex": "PitchDiff",
        "to_pin": "Lhs"
      },
      {
        "from_vertex": "StartPitch",
        "from_pin": "Out",
        "to_vertex": "PitchDiff",
        "to_pin": "Rhs"
      },
      {
        "from_vertex": "PitchDiff",
        "from_pin": "Out",
        "to_vertex": "PitchScale",
        "to_pin": "Rhs"
      },
      {
        "from_vertex": "pitchadsr",
        "from_pin": "Out",
        "to_vertex": "PitchScale",
        "to_pin": "Lhs"
      },
      {
        "from_vertex": "PitchScale",
        "from_pin": "Out",
        "to_vertex": "PitchSum",
        "to_pin": "Rhs"
      },
      {
        "from_vertex": "StartPitch",
        "from_pin": "Out",
        "to_vertex": "PitchSum",
        "to_pin": "Lhs"
      },
      {
        "from_vertex": "PitchSum",
        "from_pin": "Out",
        "to_vertex": "notefrequency",
        "to_pin": "In"
      }
    ],
    "pins": [
      {
        "name": "Out",
        "vertex": "oscbuffer",
        "pin": "Out"
      },
      {
        "name": "MidiIn",
        "vertex": "midinote",
        "pin": "MidiIn"
      },
      {
        "name": "StartPitch",
        "vertex": "StartPitch",
        "pin": "In"
      },
      {
        "name": "EndPitch",
        "vertex": "EndPitch",
        "pin": "In"
      }
    ]
  },
  "subgraphs": []
}
