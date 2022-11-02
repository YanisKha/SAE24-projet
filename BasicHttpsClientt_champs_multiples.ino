
/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>
#include <OneWire.h>
#include <DallasTemperature.h>
// Fingerprint for demo URL, expires on June 2, 2021, needs to be updated well before this date
const uint8_t fingerprint[20] = {0xdf, 0xd8, 0x61, 0xf7, 0xd9, 0x03, 0x84, 0x22, 0xf5, 0xd4, 0x82, 0x51, 0x2c, 0x46, 0x2c, 0x92, 0x26, 0x71, 0xcf, 0x29};

ESP8266WiFiMulti WiFiMulti;
String MAC_Address ="";
int numreq = 0;
const int oneWireBus = 4;

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
float temp;

void setup() {
  
  sensors.begin();
  Serial.println();
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("binome_11", "tpRT9025");
}

void loop() {
   
 sensors.requestTemperatures();
 temp = sensors.getTempCByIndex(0); // Temperature in Celsius degrees
 Serial.printf("Message: %.2f \n", temp);
  // wait for WiFi connection
      MAC_Address=WiFi.macAddress();
      Serial.print("ESP Board MAC Address: "+MAC_Address);

  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    // client->setInsecure();
    HTTPClient https;


    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://192.168.1.101:80/echo_3_parametres.php?temp="+String(temp)+"17&numreq="+String(numreq)+"&mac="+MAC_Address)) {  // HTTPS

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  numreq = numreq+1 ;
  Serial.print(numreq);

  Serial.println("Wait 10s before next round...");
  delay(10000);
  numreq = numreq+1 ;

}
