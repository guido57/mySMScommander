#include <Arduino.h>

void gsm_updateSerial();

String gsm_sendAT(String ATcmd);
bool gsm_init();
bool gsm_sendSMS(String phonenumber, String message);

void gsm_setup();

void print_rxbuf();

typedef void (gsm_rxSMS_callback)(String * phonenum, String * datetime, String * message);

void gsm_set_rxSMS_callback(gsm_rxSMS_callback * callback);
