#ifndef WIFI_H
#define WIFI_H

#include <ESP8266WiFi.h>  // Defines WiFi class


class Wifi {

  // wifi ssid and password
  const char* ssid = "propertycam";
  const char* password = "propertycam";
  
public:
    Wifi(){
    }

    // Connect to wifi
    void connect(){
      Serial.print("Connecting to wifi ");
      Serial.println(ssid);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.print("WiFi connected, local IP is ");
      Serial.println(WiFi.localIP());
      
    }

};

#endif
