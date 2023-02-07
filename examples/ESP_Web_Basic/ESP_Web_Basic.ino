/* 
 * This is the ESP Web Tools library,For Upload Firmware using web Browser and Web Socket Terminal on Browser
 * File:   ESP_Web_Basic.ino
 * Author: Nilesh Mundphan
 * Created on June 08, 2023, 11:03 PM
 */

#include <ArduinoJson.h> 
#include <EEPROM.h>
#include <ESPmDNS.h>
#include "ESP_Web_Tool.h"

#define   EEPROM_SIZE   128

struct wifi_crd{
    char  ssid[32];
    char  password[32];  
}wifi_credential;

StaticJsonDocument<512>   jsondoc;

#define     LED         10
#define     UDATE_TIME  1000

const char* hssid       = "xxxxxxxxxxxx";
const char* hpassword   = "000000000000";

const char* ssid1       = "ESP32Server";
const char* password1   = "n1234567890";

long        lastupdate  = 0;
int         counter     = 0;

ESP_Webtool   tool;

void setup(){
    Serial.begin(115200); // Used for messages and the C array generator
    delay(100);
    Serial.println("ESP WebTool test!");
    Serial.println("Setup Started"); 
    setup_wifi();
    tool.enableDebug(true);
    tool.setup();
    tool.setCallback(tool_callback);
    pinMode(LED, OUTPUT);    
    Serial.println("Entering Loop");
}

void loop(){
    tool.loop();
    if(millis()- lastupdate > UDATE_TIME){
        digitalWrite(LED, !digitalRead(LED));
        logs("Board LED " +String(digitalRead(LED)?"OFF":"ON ") + " Request: " +String(counter++));
        lastupdate = millis();
    }
}
/*
void setup_wifi(){
    int i =0;
    WiFi.begin(hssid, hpassword);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if(i++>20)
        {
          i=0;
          WiFi.reconnect();
          Serial.println("reconnect");        
        }
    }
    Serial.println(); 
    Serial.println("WiFi connected"); 
    Serial.print("IP address: ");
    Serial.print(WiFi.localIP());
    Serial.println();

    WiFi.softAP(ssid1, password1);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    Serial.println(); 
}
*/

void logs(String log_str){
  Serial.println(log_str);
  tool.print(log_str);
}

String IpAddressToString(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}

void tool_callback(uint8_t* data,unsigned int len){
  Serial.println("Tool CallBack");
  Serial.println((char*)data);
      if (strcmp((char*)data, "getwifi") == 0) {
      logs(String("SSID: ")+String(wifi_credential.ssid) +String(" IP Address: ")+ String(IpAddressToString(WiFi.localIP())));
      return;
    }

    DeserializationError error = deserializeJson(jsondoc, (char*)data);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    String iconfig  = jsondoc["config"];
    logs(iconfig);
    if(iconfig.equals("wifi")){
         /*
            {"config":"wifi","ssid":"JioFiber-Home-2G","password":"iot19internet9229"}
         */
         String ssid  = jsondoc["ssid"];
         String password  = jsondoc["password"];
         logs(ssid);
         logs(password);
         strcpy(wifi_credential.ssid,ssid.c_str());
         strcpy(wifi_credential.password,password.c_str());
         Serial.println("Strcut Data");
         Serial.println(wifi_credential.ssid);
         Serial.println(wifi_credential.password); 
         
         EEPROM.put(0,wifi_credential);
         EEPROM.commit();
         delay(1000);
         setup_wifi();
    }
    else if(iconfig.equals("wifiscan")){
         /*
            {"config":"wifiscan"}
         */
      scan_wifi();  
    }   
}

void setup_wifi(){
    EEPROM.get(0,wifi_credential);
    Serial.println("EEPROM Data");
    
    Serial.println(wifi_credential.ssid);
    Serial.println(wifi_credential.password);
    
    //WiFi.begin(hssid, hpassword);
    WiFi.begin(wifi_credential.ssid, wifi_credential.password);
    int i =0;
    while (i++ < 20) {
        delay(500);
        Serial.print(".");
        if(WiFi.status() == WL_CONNECTED)
        {
            break;   
        }
    }
    
    
    Serial.print("WiFi connected: "); Serial.println(wifi_credential.ssid); 
    Serial.print("IP address: ");
    Serial.print(WiFi.localIP());
    Serial.println();
    WiFi.softAP(ssid1, password1);
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    Serial.println(); 

    if (!MDNS.begin(ssid1)) {
      Serial.println("Error setting up MDNS responder!");
      //while(1) {
          delay(1000);
      //}
    }
    else{
      Serial.println("mDNS responder started");
      // Add service to MDNS-SD
      MDNS.addService("http", "tcp", 80);
    }
}

void scan_wifi()
{

    String wifilist = "[";
    Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            if(i==0)
              wifilist += "{\"ssid\":\""+ WiFi.SSID(i)+"\",\"RSSI\":\""+WiFi.RSSI(i)+"\",\"AUTH\":\""+String((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*") + "\"}";
            else
              wifilist += ",{\"ssid\":\""+ WiFi.SSID(i)+"\",\"RSSI\":\""+WiFi.RSSI(i)+"\",\"AUTH\":\""+String((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*") + "\"}";
            
            delay(10);
        }
    }

    wifilist += "]";
    Serial.println("");
    logs(wifilist);
}
