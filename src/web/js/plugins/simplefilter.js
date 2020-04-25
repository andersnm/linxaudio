var SimpleFilter = {
  "system": {
    "uniqueId": "NOID",
    "product": "SimpleFilter",
    "author": "Andyw",
    "myjsonId": ""
  },
  "factories": [
    {
      "name": "filter",
      "symbol": "biquad_filter_factory",
      "file": "biquad.obj",
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
        "name": "filter",
        "factory": "filter",
        "subgraph": null,
        "values": {},
        "x": 0,
        "y": 0
      }
    ],
    "edges": [],
    "pins": [
      {
        "name": "In",
        "vertex": "filter",
        "pin": "In"
      },
      {
        "name": "Out",
        "vertex": "filter",
        "pin": "Out"
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
        "name": "Q",
        "vertex": "filter",
        "pin": "Q"
      }
    ]
  },
  "subgraphs": []
};