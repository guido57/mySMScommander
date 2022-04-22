#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#endif
#include <time.h>
#include <AutoConnect.h>
#include <LittleFS.h>
#include <AutoConnect.h>
#include <ArduinoJson.h>
#include <auxfile.h>
#include <settings_page.h>
#include <default_settings.h>
#include <gsm.h>
#include <hcsr04.h>

#define inputPin 33
#define outputPin 26

AutoConnect portal;
AutoConnectConfig config;

const char* settingsPageFile = "/settings_page.json";
AutoConnectAux* settings_page;

const char* settingsDataFile = "/settings_data.json";

// JSONsetting contains the actual settings values
// it it created on setup and updated when new settings are set
StaticJsonDocument<1024> JSONsettings;
  
void CreateDefaultSettingsFile(){
  File settings_data_file = LITTLEFS.open(settingsDataFile, "w");
  settings_data_file.print(default_settings_string);
  settings_data_file.close();
  
}

// This function is for redirect only.
// The actual sending the HTML performs in PageBuilder.
void sendRedirect(String uri) {
  WebServer& ws = portal.host();
  ws.sendHeader("Location", uri, true);
  ws.send(302, "text/plain", "");
  ws.client().stop();
}


String OnSettingsSave(AutoConnectAux& aux, PageArgument& args) {
  Serial.println("OnSettingsSave");
  // Open the settings file on the LITTLEFS
  File settings_data_file = LITTLEFS.open(settingsDataFile, "w");
  String phone1 = settings_page->getElement<AutoConnectInput>("phone1").value;
  String msgH1 = settings_page->getElement<AutoConnectInput>("msgH1").value;
  String msgL1 = settings_page->getElement<AutoConnectInput>("msgL1").value;
  bool sendOnChanged1 = settings_page->getElement<AutoConnectCheckbox>("sendOnChanged1").checked;
  String msgLevelH1 = settings_page->getElement<AutoConnectInput>("msgLevelH1").value;
  String msgLevelL1 = settings_page->getElement<AutoConnectInput>("msgLevelL1").value;
  bool sendOnLevelChanged1 = settings_page->getElement<AutoConnectCheckbox>("sendOnLevelChanged1").checked;
  int threshold = settings_page->getElement<AutoConnectRange>("threshold").value;
  
  Serial.printf("phone1=%s, msgH1=%s, msgL1=%s sendOnChanged1=%d\r\n",
  phone1.c_str(),msgH1.c_str(),msgL1.c_str(),sendOnChanged1);
  JSONsettings["phone1"] = phone1;
  JSONsettings["msgH1"] = msgH1;
  JSONsettings["msgL1"] = msgL1;
  JSONsettings["sendOnChanged1"] = sendOnChanged1;
  JSONsettings["threshold"] = threshold;
  JSONsettings["msgLevelH1"] = msgLevelH1;
  JSONsettings["msgLevelL1"] = msgLevelL1;
  JSONsettings["sendOnLevelChanged1"] = sendOnLevelChanged1;
  JSONsettings["Out1"] = (1 == digitalRead(outputPin))? true:false;
  serializeJson(JSONsettings,settings_data_file);
  settings_data_file.close();
  Serial.printf("OnSettingsSave: completed successfully\r\n");
  settings_data_file.close();
  return "ciao";
}

// Get settings from file and load them to JSONsettings
bool LoadSettingsFromFile(){
  File settings_data_file = LITTLEFS.open(settingsDataFile, "r");
  if(!settings_data_file){
    Serial.printf("the file '%s doesn't exist -> create it\r\n", settingsDataFile);
    CreateDefaultSettingsFile();
    settings_data_file = LITTLEFS.open(settingsDataFile, "r");
  }
  
  DeserializationError error = deserializeJson(JSONsettings, settings_data_file);
  if (error) {
    Serial.printf("JSON %s de-serialization failed -> create it with default values\r\n",settingsDataFile);
    CreateDefaultSettingsFile();
    settings_data_file = LITTLEFS.open(settingsDataFile, "r");
    error = deserializeJson(JSONsettings, settings_data_file);
    settings_data_file.close();
  }
  if(error){
    Serial.printf("JSON %s de-serialization failed after creation\r\n",settingsDataFile);
    return false;
  }
  return true;
}

// Save settings from JSONsettings to file
bool SaveSettingsToFile(){
  File settings_data_file = LITTLEFS.open(settingsDataFile, "w");
  
  serializeJson(JSONsettings, settings_data_file);  
  settings_data_file.close();
  return true;
}

// Init settings values
String OnSettingsLoad(AutoConnectAux& aux, PageArgument& args) {

      LoadSettingsFromFile();
      // Parsing the parameter JSON was successful.
      // Set the values
      AutoConnectInput& phone1 = aux.getElement<AutoConnectInput>("phone1");
      phone1.value = JSONsettings["phone1"].as<String>();
      AutoConnectInput& msgH1 = aux.getElement<AutoConnectInput>("msgH1");
      msgH1.value = JSONsettings["msgH1"].as<String>();
      AutoConnectInput& msgL1 = aux.getElement<AutoConnectInput>("msgL1");
      msgL1.value = JSONsettings["msgL1"].as<String>();
      AutoConnectCheckbox& sendOnChanged1 = aux.getElement<AutoConnectCheckbox>("sendOnChanged1");
      sendOnChanged1.checked = JSONsettings["sendOnChanged1"].as<bool>() ;
      AutoConnectInput& msgLevelH1 = aux.getElement<AutoConnectInput>("msgLevelH1");
      msgLevelH1.value = JSONsettings["msgLevelH1"].as<String>();
      AutoConnectInput& msgLevelL1 = aux.getElement<AutoConnectInput>("msgLevelL1");
      msgLevelL1.value = JSONsettings["msgLevelL1"].as<String>();
      AutoConnectCheckbox& sendOnLevelChanged1 = aux.getElement<AutoConnectCheckbox>("sendOnLevelChanged1");
      sendOnLevelChanged1.checked = JSONsettings["sendOnLevelChanged1"].as<bool>() ;
      AutoConnectText& Out1 =  aux.getElement<AutoConnectText>("Out1");
      Out1.value = String("Output is: ") + (JSONsettings["Out1"].as<bool>() ? String("Off") : String("On")) ;
      AutoConnectRange& threshold = aux.getElement<AutoConnectRange>("threshold");
      threshold.value = JSONsettings["threshold"].as<int>();
      AutoConnectText& level = aux.getElement<AutoConnectText>("level");
      level.value = String("Level is ") + String(hcsr04_getDistanceCm()) + String(" cm");

      Serial.printf("OnSettingsLoad: completed successfully\r\n");
      Serial.printf("phone1=%s msgH1=%s msgL1=%s sendOnChanged=%d ",
          phone1.value.c_str(),
          msgH1.value.c_str(),
          msgH1.value.c_str(),
          sendOnChanged1.checked);
      Serial.printf("msgLevelH1=%s msgLevelL1=%s sendOnLevelChanged=%d ",
          msgLevelH1.value.c_str(),
          msgLevelH1.value.c_str(),
          sendOnLevelChanged1.checked);
      Serial.printf("threshold=%d %s\r\n ",
          threshold.value,
          Out1.value.c_str());
  
      return String();
}

// this function will be called when an SMS is received
void rxSMS_callback(String * phonenum, String * datetime, String * SMSmessage){
  Serial.printf("callback: phonenum=%s message=%s\r\n",phonenum->c_str(), SMSmessage->c_str());
  SMSmessage->toUpperCase(); 
  // handle the received message
  if( *SMSmessage == "ON" ){
    // turn on something
    digitalWrite(outputPin,LOW);
    JSONsettings["Out1"] = false;
    bool newvalue = JSONsettings["Out1"].as<bool>();
    Serial.printf("%s asked to turn ON. Now Out1 is %d\r\n",phonenum->c_str(),newvalue);
    SaveSettingsToFile(); 
  }else if( *SMSmessage == "OFF" ){
    // turn off something
    Serial.printf("%s asked to turn OFF\r\n",phonenum->c_str());
    digitalWrite(outputPin,HIGH);
    JSONsettings["Out1"] = true;
    bool newvalue = JSONsettings["Out1"].as<bool>();
    Serial.printf("%s asked to turn OFF. Now Out1 is %d\r\n",phonenum->c_str(),newvalue);
    SaveSettingsToFile(); 
  }else{
    Serial.printf("%s asked %s\r\n",phonenum->c_str(), SMSmessage->c_str());
    String msgL1 = JSONsettings["msgL1"].as<String>();
    String msgH1 = JSONsettings["msgH1"].as<String>();
    String input_message = (digitalRead(inputPin) == HIGH) ? msgL1 : msgH1;
    String msgLevelL1 = JSONsettings["msgLevelL1"].as<String>();
    String msgLevelH1 = JSONsettings["msgLevelH1"].as<String>();
    String level_message = "the tank level is 110 cm";
    String threshold_message = "threshold level is " + JSONsettings["threshold"].as<String>() + " cm";
    String out_message = String("Out is ") + ((digitalRead(outputPin) == HIGH) ? "OFF.": "ON."); 

    String SMS_msg = input_message + level_message + threshold_message + out_message;
    gsm_sendSMS(*phonenum,SMS_msg);  

  }

}

// read the level in cm by ultrasonic sensor HC SR04
int readLevel(){
  
  int level = hcsr04_getDistanceCm();
  return level;

}

bool lastInputPinValue;
int lastLevelValue;
void setup() {
  delay(1000);
  Serial.begin(9600);
  Serial.println("setup");

  //init input and output
  pinMode(inputPin,INPUT_PULLUP);
  lastInputPinValue = HIGH;
  pinMode(outputPin,OUTPUT);
  digitalWrite(outputPin,HIGH); // yes, the relay is active LOW
  
  
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
  
  LoadSettingsFromFile();
  // restore the Out1 value to the stored one
  digitalWrite(outputPin,JSONsettings["Out1"].as<bool>());

  // SIM800L GSM
  gsm_setup();
  gsm_init();
  gsm_set_rxSMS_callback(rxSMS_callback);

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

void loop() {

  String phone1 = JSONsettings["phone1"].as<String>();
   
  // check input pin
  String msgInput = "";
  int inputPinValue = digitalRead(inputPin);
  if( inputPinValue != lastInputPinValue){
    Serial.printf("inputPin changed from %d to %d\r\n",lastInputPinValue, inputPinValue);
    bool sendOnChanged = JSONsettings["sendOnChanged1"].as<bool>();
    if(sendOnChanged){
      // read the actual settings values
      String msgL1 = JSONsettings["msgL1"].as<String>();
      String msgH1 = JSONsettings["msgH1"].as<String>();
      msgInput = (inputPinValue == LOW) ? msgH1 : msgL1;
    }
    lastInputPinValue = inputPinValue;
  }

  // check input level
  String msgLevel = "";
  int levelValue = readLevel();
  int threshold = JSONsettings["threshold"].as<int>();
  if( (levelValue > threshold && lastLevelValue < threshold)  || 
      (levelValue < threshold && lastLevelValue > threshold)
  ){
    Serial.printf("level changed from %d to %d\r\n",lastLevelValue, levelValue);
    bool sendOnLevelChanged = JSONsettings["sendOnLevelChanged1"].as<bool>();
    if(sendOnLevelChanged){
      // read the actual settings values
      String phone1 = JSONsettings["phone1"].as<String>();
      String msgLevelL1 = JSONsettings["msgLevelL1"].as<String>();
      String msgLevelH1 = JSONsettings["msgLevelH1"].as<String>();
      msgLevel += (levelValue > threshold) ? msgLevelH1 : msgLevelL1;
      // send the SMS
    }

    lastLevelValue = levelValue;
  }

  if(msgInput.length() + msgLevel.length() >0 )
      gsm_sendSMS(phone1, msgInput + msgLevel);


  portal.handleClient();
  gsm_updateSerial();
}