var SimpleGain = {
  "system": {
    "uniqueId": "S001",
    "product": "SimpleGain",
    "author": "Andyw",
    "myjsonId": ""
  },
  "factories": [
    {
      "name": "gain",
      "symbol": "gain_factory",
      "file": "gain.obj",
      "deps": [
        "inertia.obj"
      ]
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
        "name": "gain",
        "factory": "gain",
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
        "vertex": "gain",
        "pin": "In"
      },
      {
        "name": "Out",
        "vertex": "gain",
        "pin": "Out"
      },
      {
        "name": "Amp",
        "vertex": "gain",
        "pin": "Amp"
      }
    ]
  },
  "subgraphs": []
};
