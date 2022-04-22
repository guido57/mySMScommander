#include <Arduino.h>
const char * default_settings_string= R"( 
{
    "phone1": "+393351234567",
    "msgH1": "message when the input is high",
    "msgL1": "message when the input is low",
    "sendOnChanged1": false,
    "level": "Actual level: 100 cm",
    "threshold": 80,
    "msgLevelH1": "message when the level goes above the threshold",
    "msgLevelL1": "message when the level goes below the threshold",
    "sendOnLevelChanged1": false,
    "Out1": false
}
)";
