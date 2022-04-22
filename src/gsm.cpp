#include <Arduino.h>
#include <gsm.h>

// The serial connection to the GSM device
HardwareSerial ss(1);
#define MAX_RX_BUF 1024
// SIM800L reset pin is connected to GPIO 21
#define pin_RST 21    
// SIM800L rxd and txd are connected to GPIO pins
#define pin_RXD 18
#define pin_TXD 17
    



gsm_rxSMS_callback * mycallback = nullptr;

void gsm_set_rxSMS_callback(gsm_rxSMS_callback * callback){
    mycallback =  callback;    
}

String getLine();

char rxbuf[MAX_RX_BUF];
int wp=0;

// check if wp=0 
// if not, set it to 0
String Assert_wp(){
    if(wp > 0){
        wp=0;
        return "KO";
    }
    return "OK";

}

void print_rxbuf(){
    char ppbuf[1024];
    strncpy(ppbuf,rxbuf,wp);
    ppbuf[wp]=0;
    Serial.printf("wp=%d rxbuf=\r\n-----START---------%s\r\n-----END--------\r\n",wp,ppbuf); 
    for(int i=0;i<wp;i++){
        Serial.printf("rxbuf[%d]=0x%x %c\r\n",i,rxbuf[i], rxbuf[i]>32?rxbuf[i]:' ' );
    }
}

bool waiting_for_SMS_message = false;
bool waiting_for_CMT_line = false;
String SMSmessage, CMTline, SMSphone, SMSdatetime="";  
  
void gsm_updateSerial()
{
  delay(100);
  while (Serial.available()) 
  {
    ss.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(ss.available()) 
  {
    char rxbyte = ss.read();
    if(wp<MAX_RX_BUF){
      //Serial.printf("writing %c %x at wp=%d\r\n",rxbyte,rxbyte, wp);
      rxbuf[wp++] = rxbyte;
    }
    Serial.write(rxbyte);//Forward what Software Serial received to Serial Port
  }

  //if(wp>0)
  //  print_rxbuf();  

  //check for received SMS
  if(strncmp(rxbuf,"\r\n+CMT:",7)==0){
      String pn, dt, msg;
      Serial.printf("SMS ricevuto");
      //print_rxbuf();
      strncpy(rxbuf,rxbuf+2, MAX_RX_BUF - 2); // get rid of \r\n
      wp=wp-2;
      waiting_for_CMT_line=true;  
      waiting_for_SMS_message = false;
  }
  if(waiting_for_CMT_line){
      String ret = getLine();
      if(ret != ""){
        CMTline = ret;
        Serial.printf("CMTline=%s\r\n",CMTline.c_str());
        waiting_for_CMT_line = false;
        waiting_for_SMS_message = true;
      }  
  }
  if(waiting_for_SMS_message){
      String ret = getLine();
      if(ret != ""){
          SMSmessage = ret; 
          Serial.printf("SMSmessage=%s\r\n",SMSmessage.c_str());
          waiting_for_CMT_line = false;
          waiting_for_SMS_message = false;
          if(mycallback != nullptr){
            int first_comma_pos = CMTline.indexOf(",");  
            String SMSphone = CMTline.substring(7,first_comma_pos-1);
            mycallback(&SMSphone,&SMSdatetime,&SMSmessage);    

          }
          waiting_for_CMT_line = false;
          waiting_for_SMS_message = false;
      }
    }
  //if(wp>0)
  //  print_rxbuf();      
    
}

// get the first line (CR LF terminated string) available
// it returns the line or ""
String getLine(){
  int ndx = 1;
  while(ndx<wp){
    //Serial.printf("scanning rxbuf[%d]=%c  wp=%d\r\n",ndx,rxbuf[ndx],wp );
    if(rxbuf[ndx-1]=='\r' && rxbuf[ndx]=='\n'){
      //Serial.println("found a line (CR LF terminated)  ");
      char tbuf[MAX_RX_BUF];   // temporary buffer
      // copy the line, skipping 0x0d 0x0a and every character lower than 32
      int ii = 0;  int io = 0;
      while(ii<ndx){
        if(rxbuf[ii]>=32)
          tbuf[io++] = rxbuf[ii];
        ii++;
      }
      tbuf[io] = 0;    // null terminate the found line
      wp=wp-ndx-1; // shift left the write pointer
      strncpy(rxbuf,rxbuf+ndx+1,MAX_RX_BUF-ndx);
      return String(tbuf);
    }  
    ndx++;
  }
  return String("");
}

// send an AT command and wait for the answer 
// return the answer or "" if timeout (5 seconds);
String sendAT(String ATcmd, unsigned long timeout_msecs = 5000L){
  ss.println( ATcmd + "\r\n");
  unsigned long start = millis();
  String retline = "";
  while(millis()< start + timeout_msecs){
    gsm_updateSerial();
    retline = getLine();
    if(retline.length() >0 && retline != ATcmd ){
      //Serial.printf("the answer is %s. Its length is %d\r\n",retline.c_str(),retline.length());
      return retline;
    }
  }
  return String("");
}

// send an AT command AT+CSQ and wait for the answer 
// return the CSQ value (e.g. +CSQ: 6,0) or "" if timeout (5 seconds);
// +CSQ: 0,0 means no GSM signal
String sendATCSQ(){

  String retline = "";
  String CSQline = "";
  ss.println( "AT+CSQ\r\n");
  unsigned long start = millis();
  while(millis()< start + 5000  ){
    gsm_updateSerial();
    retline = getLine();
    if(retline.startsWith("+CSQ"))
      CSQline = retline;  
    if(retline == "OK")
      return CSQline;
    if(retline == "ERROR")
      return "ERROR";
  }
  return String("");
}

// send an AT command AT+CMGS and wait for the answer 
String sendATCMGS(String phonenumber, String message){
  String retline = "";
  String ATcmd = "AT+CMGS=\"" + phonenumber + "\"";
  ss.print( ATcmd + "\r\n");
  unsigned long start = millis();
  while(millis()< start + 30000  ){
    gsm_updateSerial();
    retline = getLine(); // get rid of AT+CMGS="223444"\r\n
    
    if(rxbuf[0] == '>'){
      //Serial.println("> showed up -> write the message");
      ss.print(message);
      ss.write(26);
      rxbuf[0] = '-';
      
    } 
    if(retline == "OK"){
        
      return "OK";
    }
    if(retline == "ERROR")
      return "ERROR";
  }
  return String("");
}

bool gsm_reset(){
    // send reset pin to LOW for 100 msec
    digitalWrite(pin_RST,LOW);  
    delay(100);
    digitalWrite(pin_RST,HIGH);  
   return true;
}

void gsm_setup()
{
  // set the Hardware Serial and the reset pin  
  ss.begin(9600, SERIAL_8N1, pin_RXD, pin_TXD);
  pinMode(pin_RST,OUTPUT);  
  digitalWrite(pin_RST,HIGH);  

  Serial.println("Initializing GSM...");
  /*
  bool SMS_ready = gsm_reset();
  if(!SMS_ready)
    ESP.restart();
  //sendAT("AT");
  */
  /*
  ss.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  ss.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  //spi_struct_t.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  //updateSerial();
  //static_assert.println("AT+CREG?"); //Check whether it has registered in the network
  //updateSerial();
  ss.println("AT+CSCS=\"GSM\""); 
  updateSerial();
  ss.println("AT+CMGF=1"); 
  updateSerial();
  ss.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();
  ss.println("AT+CMGS=\"+393358267954\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  ss.print("Last Minute Engineers | lastminuteengineers.com"); //text content
  updateSerial();
  ss.write(26);
  */
}


bool gsm_init(){
    Serial.println("GSM reset ...");
    gsm_reset();
    unsigned long start = millis();
    String ret = "";
    while(ret != "OK" && millis() < start + 30000L ){
        ret = sendAT("AT",1000);
        Serial.print(".");
        gsm_updateSerial();
    }
    
    while(ret != "SMS Ready" && millis() < start + 30000L ){
        Serial.print(".");
        ret = getLine();
        gsm_updateSerial();
    }


    Serial.printf("GSM completed reset after %f seconds since start\r\n", (millis()-start)/1000.0 );
    //Serial.printf("AT returned %s\r\n",ret.c_str());
    //print_rxbuf();
    ret = sendATCSQ();
    Serial.printf("AT+CSQ returned %s\r\n",ret.c_str());
    if(!ret.startsWith("+CSQ") ){
      Serial.printf("AT+CSQ returned %s\r\n", ret.c_str());
      return false;  
    }
    print_rxbuf();
    if( ret == "+CSQ: 0,0" ){
      Serial.println("Sorry, there's no GSM signal here!");
    }
      
    // CSQ > 0 i.e. GSM signal is present
    // now set GMS mode
    ret = sendAT("AT+CSCS=\"GSM\"");
    if(ret != "OK"){
      Serial.printf("AT+CSCS=\"GSM\" returned %s\r\n", ret.c_str());
      return false;
    }
    print_rxbuf();
    ret = sendAT("AT+CNMI=1,2,0,0,0");
      
    if(ret != "OK"){
      Serial.printf("AT+CNMI=1,2,0,0,0 returned %s\r\n", ret.c_str());
      return false;
    }

    ret == sendAT("AT+CMGF=1");
    if(ret!="OK"){
      Serial.printf("AT+CMGF=1 returned %s\r\n", ret.c_str());
      return false;
    }
    Serial.printf("GSM completed setup after %f seconds since start\r\n",(millis()-start)/1000.0);
    return true;
}

bool gsm_sendSMS(String phonenumber, String message){

    unsigned long start = millis();

    if(String("KO") == Assert_wp()){
        Serial.println("the rxbuf should be empty before sending an SMS but it isn't");  
        print_rxbuf();
    }

    String ret = sendATCMGS(phonenumber, message);
    if(ret == ""){
        Serial.printf("ATCMGS timeout!\r\n");
        return false;
    }
    if(ret != "OK"){
      Serial.printf("ATCMGS returned %s\r\n", ret.c_str());
      return false;
    }
    if(String("KO") == Assert_wp()){
        Serial.println("the rxbuf should be empty after sending an SMS but it isn't");  
        print_rxbuf();
    }
    if(ret=="OK"){
      Serial.printf("sendSMS completed in %f seconds\r\n",(millis()-start)/1000.0);  
    }

    return true;
     
}


