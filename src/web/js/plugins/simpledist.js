var SimpleDist = {
  "system": {
    "paths": [
      "lib"
    ],
    "uniqueId": "NOID",
    "product": "SimpleDist",
    "author": "",
    "myjsonId": ""
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
        "name": "distortion",
        "factory": "distortion",
        "subgraph": null,
        "values": {},
        "x": -0.08199121522693997,
        "y": -0.39603960396039606
      },
      {
        "name": "clip",
        "factory": "clip",
        "subgraph": null,
        "values": {},
        "x": -0.09516837481698392,
        "y": -0.044554455445544594
      }
    ],
    "edges": [
      {
        "from_vertex": "distortion",
        "from_pin": "Out",
        "to_vertex": "clip",
        "to_pin": "In"
      }
    ],
    "pins": [
      {
        "name": "In",
        "vertex": "distortion",
        "pin": "In"
      },
      {
        "name": "Out",
        "vertex": "clip",
        "pin": "Out"
      },
      {
        "name": "Amount",
        "vertex": "distortion",
        "pin": "Amount"
      },
      {
        "name": "PreGain",
        "vertex": "clip",
        "pin": "PreGain"
      }
    ]
  },
  "subgraphs": []
};
