/* 
    Copyright (c) 2021 Think Make. All rights reserved.
    Author : Nilesh Mundphan
    Date  : 22:04:21  20:00 IST 
*/

#ifndef _ESP_WEB_TOOL_
#define _ESP_WEB_TOOL_

#include <Arduino.h>
#include <FS.h>
#include <WebSocketsServer.h>

#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#elif ESP32
#include <WebServer.h>
#include <Update.h>
#include <SPIFFS.h>
#endif

#define CALLBACK_SIGNATURE std::function<void(uint8_t*,unsigned int)> callback
#define ESP_WEB_TOOL_VERSION "ESP Web Tools \nDesign By N!lesh M\nDate :19/06/22"
#define MAX_CLIENT 5

class ESP_Webtool
{
        public: 
            ESP_Webtool();
            ESP_Webtool(uint16_t port,uint16_t port1);  
            CALLBACK_SIGNATURE;
            void print(String logs);
            void sock_callback();   
            void setup();
            void loop(); 
            ESP_Webtool& setCallback(CALLBACK_SIGNATURE);
            void update_page(void);
            void terminal_page(void);
            void fs_page(void);
            void wifi_page(void);
            void notFound(void);
            String getContentType(String filename);
            bool handleFileRead(String path);
            void uploadResp(void);
            void handleUpload(void);
            void onWebSocketEvent(uint8_t client_num, WStype_t type,uint8_t * payload,size_t length);
            void listFiles(void);
            void handleFileDelete(String path);
            bool exists(String path);
            void enableDebug(boolean debug);
            void logs(String log_str);
            File fsUploadFile;
            boolean client[MAX_CLIENT];
    #ifdef ESP32
            void  listDir(fs::FS &fs, const char * dirname, uint8_t levels);
    #endif
            boolean debug = false;    
            String fileslist;     
    protected:
    #ifdef ESP8266
            ESP8266WebServer server;
    #elif ESP32
            WebServer server;
    #endif
    
            WebSocketsServer *webSocket;
            boolean fs_enable=false;
            boolean restart  = false;
};

#endif
