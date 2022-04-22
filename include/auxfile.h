#include <Arduino.h>

const char * auxfile_string = R"( 
[
  {
    "title": "Heartbeat",
    "uri": "/",
    "menu": true,
    "element": [
      {
        "name": "param",
        "type": "ACFile",
        "label": "Parameter file:",
        "store": "fs"
      },
      {
        "name": "set",
        "type": "ACSubmit",
        "value": "SET",
        "uri": "/set"
      }
    ]
  },
  {
    "title": "Heartbeat",
    "uri": "/set",
    "menu": false,
    "element": [
      {
        "name": "param",
        "type": "ACText"
      }
    ]
  }
]
)";