#include "Station.h"

#define RETRY 5

Station station;
void setup() {
  pinMode(D5, INPUT);
  Serial.begin(115200);
  delay(3000);
  Serial.println("Start");

  station.begin();

  bool enableSetConfig = false;
  if(digitalRead(D5) == HIGH){
    delay(1000);
    if(digitalRead(D5) == HIGH){
      delay(1000);
      if(digitalRead(D5) == HIGH){
        enableSetConfig = true;
      }
    }
  }

  bool enableSaveConfig = false;
  if(enableSetConfig || station.loadConfig() == false){
    Serial.println("SetConfig");
    station.getConfig();
    enableSaveConfig = true;
  }

  if(enableSaveConfig){
    Serial.println("SaveConfig");
    if(station.saveConfig() == false){
      station.dispError("SAVE CONFIG ERROR");
      Serial.println("SaveConfig ERR");
    }
  }

  station.setColorMode();

  bool enableWLAN = false;
  for(int i=0;i < RETRY;i++){
    if(station.tryWLAN()){
      enableWLAN = true;
      Serial.println("WLAN OK");
      break;
    }else{
      Serial.println("WLAN ERR");
      delay(1000);
      if(i == RETRY - 1){
        station.dispError("WLAN ERROR");
      }
    }
  }

  bool enableNTP = false;
  if(enableWLAN){
    for(int i=0;i < RETRY;i++){
      if(station.tryNTP()){
        enableNTP = true;
        Serial.println("NTP OK");
        break;
      }else{
        Serial.println("NTP ERR");
        delay(1000);
        if(i == RETRY - 1){
          station.dispError("NTP ERROR");
        }
      }
    }
  }

  bool enableJSON = false;
  if(enableNTP){
    for(int i=0;i < RETRY;i++){
      if(station.getWeather()){
        enableJSON = true;
        Serial.println("GET OK");
        break;
      }else{
        Serial.println("GET ERR");
        delay(1000);
        if(i == RETRY - 1){
          station.dispError("HTTP ERROR");
        }
      }
    }
  }

  station.disconnectWLAN();

  bool enableParse = false;
  if(enableJSON){
    if(station.parseWeather()){
      enableParse = true;
      Serial.println("PARSE OK");
    }else{
      station.dispError("DATA ERROR");
      Serial.println("PARSE ERR");
    }
  }

  if(enableParse){
    station.dispWeather();
  }

  station.sleep(enableParse);
}

void loop() {}
