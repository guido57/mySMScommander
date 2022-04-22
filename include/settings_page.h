#include <Arduino.h>
const char * settings_page_string = R"( 
[
  {
    "title": "Settings",
    "uri": "/",
    "menu": true,
    "element": [

      {
        "name": "style",
        "type": "ACStyle",
        "value":"#phone1, #msgL1, #msgH1, #threshold,#msgLevelH1,#msgLevelL1  {width: 100%;}"
      },
      {
        "name": "header",
        "type": "ACText",
        "value":"<h2>Input 1</h2>"
      },
      {
        "name": "phone1",
        "type": "ACInput",
        "label": "Phone (e.g. +393351234567) "
      },
      {
        "name": "line1",
        "type": "ACElement",
        "value": "<br><hr><br>"
      },
      {
        "name": "msgH1",
        "type": "ACInput",
        "label": "Message when Input goes HIGH (OPEN) "
      },
      {
        "name": "msgL1",
        "type": "ACInput",
        "label": "Message when Input goes LOW (CLOSED) "
      },
      {
        "name": "sendOnChanged1",
        "type": "ACCheckbox",
        "label": "Send on changed"
      },
      {
        "name": "header2",
        "type": "ACText",
        "value":"<h2>Level 1</h2>"
      },
      {
        "name": "level",
        "type": "ACText",
        "value": "Actual level: 120 cm"
      },
      {
        "name": "break",
        "type": "ACElement",
        "value": "<br><br>"
      },
      {
        "name": "threshold",
        "type": "ACRange",
        "label": "Input threshold (cm) ",
        "min": "10",
        "max": "400",
        "step": "10",
        "magnify": "behind"
      },
      {
        "name": "msgLevelH1",
        "type": "ACInput",
        "label": "Message when Level goes above threshold"
      },
      {
        "name": "msgLevelL1",
        "type": "ACInput",
        "label": "Message when Level goes below threshold"
      },
      {
        "name": "sendOnLevelChanged1",
        "type": "ACCheckbox",
        "label": "Send on level changed above/below the threshold"
      },
      {
        "name": "line5",
        "type": "ACElement",
        "value": "<br><hr><br>"
      },
      {
        "name": "header3",
        "type": "ACText",
        "value":"<h2>Output 1</h2>"
      },
      {
        "name": "Out1",
        "type": "ACText",
        "value": "Out Level is: LOW"
      },
      {
        "name": "line6",
        "type": "ACElement",
        "value": "<br><br>"
      },
      {
        "name": "save",
        "type": "ACSubmit",
        "value": "Save",
        "uri": "/settings_save"
      },
      {
        "name": "discard",
        "type": "ACSubmit",
        "value": "Discard",
        "uri": "/"
      }
    ]
  },
  {
    "title": "SettingsSave",
    "uri": "/settings_save",
    "menu": false,
    "element": [

        {
        "name": "js",
        "type": "ACElement",
        "value": "<script type='text/javascript'>location.replace('/')</script>"
        }

    ]
  }
]
)";
