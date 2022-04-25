# mySMScommander

See the project at https://hackaday.io/project/185002-sms-water-tank-controller

## void setup()

During setup()

Init the input and output pins of ESP32

```
  pinMode(inputPin,INPUT_PULLUP);
  lastInputPinValue = HIGH;
  pinMode(outputPin,OUTPUT);
  digitalWrite(outputPin,HIGH); // yes, the relay is active LOW
```  

Initialize the LITTLEFS filesystem
```
  // Initialize LittleFS
  if (!LITTLEFS.begin(false /* false: Do not format if mount failed */)) {
    Serial.println("Failed to mount LittleFS -> format LittleFS");
    if (!LITTLEFS.begin(true /* true: format */)) {
      Serial.println("Failed to format LittleFS");
    } else {
      Serial.println("LittleFS formatted successfully");
    }
  } else { // Initial mount success
      Serial.println("LittleFS succesfully mounted");
  }
```

Load the Settings from a file stored in the filesystem
If not found, load default settings stored in 
```
  LoadSettingsFromFile();
```

Restore the outpot relay to the state it had before
```
// restore the Out1 value to the stored one
  digitalWrite(outputPin,JSONsettings["Out1"].as<bool>());
```

Init the SIM800L device and set its callback function, that will be called when an SMS is received
```
  // SIM800L GSM
  gsm_setup();
  gsm_init();
  gsm_set_rxSMS_callback(rxSMS_callback);
```

```
  // don't send a SMS when started
  lastInputPinValue = digitalRead(inputPin); 
  lastLevelValue = readLevel();

  portal.load(settings_page_string);
  settings_page = portal.aux("/");
  portal.on("/settings_save", OnSettingsSave,AC_EXIT_AHEAD);
  portal.on("/", OnSettingsLoad, AC_EXIT_AHEAD);
  if (portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }

  hcsr04_setup();

}
