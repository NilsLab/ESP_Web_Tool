#include <Arduino.h>
#include "ESP_Web_Tool.h"
#include "Upload_Page.h"

ESP_Webtool::ESP_Webtool():server(80){
  using namespace std::placeholders;
  webSocket = new WebSocketsServer(1337);
  setCallback(NULL);
  server.on("/ota", HTTP_GET,std::bind(&ESP_Webtool::update_page,this));
  server.on("/fs", HTTP_GET,std::bind(&ESP_Webtool::fs_page,this));
  server.on("/wifi", HTTP_GET,std::bind(&ESP_Webtool::wifi_page,this));
  server.on("/terminal", HTTP_GET,std::bind(&ESP_Webtool::terminal_page,this));
  server.on("/update", HTTP_POST, std::bind(&ESP_Webtool::uploadResp,this),std::bind(&ESP_Webtool::handleUpload,this));
  server.onNotFound(std::bind(&ESP_Webtool::notFound,this));
}

ESP_Webtool::ESP_Webtool(uint16_t port,uint16_t port1):server(port){   
  using namespace std::placeholders; 
  webSocket = new WebSocketsServer(port1);
  setCallback(NULL);
  server.on("/ota", HTTP_GET,std::bind(&ESP_Webtool::update_page,this));
  server.on("/fs", HTTP_GET,std::bind(&ESP_Webtool::fs_page,this));
  server.on("/wifi", HTTP_GET,std::bind(&ESP_Webtool::wifi_page,this));
  server.on("/terminal", HTTP_GET,std::bind(&ESP_Webtool::terminal_page,this));
  server.on("/update", HTTP_POST, std::bind(&ESP_Webtool::uploadResp,this),std::bind(&ESP_Webtool::handleUpload,this));
  server.onNotFound(std::bind(&ESP_Webtool::notFound,this));
} 

void ESP_Webtool::setup(){
  using namespace std::placeholders;
  server.begin();   
  webSocket->begin();
  webSocket->onEvent(std::bind(&ESP_Webtool::onWebSocketEvent,this,_1,_2,_3,_4));
  #ifdef ESP8266
  if (!SPIFFS.begin()) {  
  #else
  if (!SPIFFS.begin(true)) {
  #endif
  
      logs("SPIFFS initialisation failed!");
  }
  logs(ESP_WEB_TOOL_VERSION);
} 

void ESP_Webtool::enableDebug(boolean debug){
    this->debug = debug;
}

void ESP_Webtool::logs(String log_str){
    if(debug){
      Serial.println(log_str);
    }
}

void ESP_Webtool::loop(){
  server.handleClient();
  webSocket->loop(); 
} 

void ESP_Webtool::print(String logs){
  for(int i=0;i<MAX_CLIENT;i++){
    if(client[i]){
      webSocket->sendTXT(i,logs);
    }
  }
}

void ESP_Webtool::onWebSocketEvent(uint8_t client_num, WStype_t type,uint8_t * payload,size_t length) 
{
  // Figure out the type of WebSocket event
  String      message     = "";
  switch(type) {
    // Client has disconnected
    case WStype_DISCONNECTED:
      logs("Client :"+String(client_num) + " Disconnected!");
      if(client_num < MAX_CLIENT)
      client[client_num] = false;
      break;

    // New client has connected
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket->remoteIP(client_num);
        logs("Client :"+ String(client_num)+" Connected " );
        logs(ip.toString());
        if(client_num < MAX_CLIENT)
        client[client_num] = true;
      }
      break;

    // Handle text messages from client
    case WStype_TEXT:
      // Print out raw message
      
      logs("Client :"+String( client_num) + " Received text: " + String((char*)payload));
      payload[length] = 0;
      message = (char*)payload;
      if (message.indexOf("delete_") >= 0) {
          String file_name =  message.substring(7);
          logs(file_name);
          handleFileDelete(file_name);
          listFiles();
          print(fileslist);
      }
      
      if (strcmp((char*)payload, "getFiles") == 0) {
        //notifyClients(getSliderValues());
        listFiles();
        print(fileslist);
      }
      else if (callback) {
          callback(payload,length);
      }
      break;

    // For everything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}

void ESP_Webtool::notFound(void) { 
  logs("Not Found " + server.uri());
  if (!handleFileRead(server.uri()))   {               // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  }
 }

String ESP_Webtool::getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool ESP_Webtool::handleFileRead(String path) { // send the right file to the client (if it exists)
  logs("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  logs("\tFile Not Found");

  if(path.equals("/index.html")){
      server.sendHeader("Connection", "close");
      //server.send(200, "text/html", index_html);
      server.send(200, "text/html", upload_page);
      return true; 
  }
  return false;                                         // If the file doesn't exist, return false
}

void ESP_Webtool::uploadResp(void) {  
  const char upload_ok[] = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <body>
        <p>ESP  Upload Success</p>
    <script>
        window.onload = function() { 
            if(!alert("ESP Upload Success Goto Main Page")) document.location = "/";
        };
    </script>
    </body>
    </html>)rawliteral";
    
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", (Update.hasError()) ? "ESP Firmware Upload Failed" : upload_ok);
  delay(1000);
  if(restart){
    ESP.restart();
  }  
}

void ESP_Webtool::wifi_page(void) {  
  String path = "/wifi_setup.html";
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, "text/html"); // And send it to the client
    file.close();                                       // Then close the file again
  }else{
    logs("\tFile Not Found");
  }
}

void ESP_Webtool::update_page(void) {  
  String path = "/update.html";
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, "text/html"); // And send it to the client
    file.close();                                       // Then close the file again
  }else{
    logs("\tFile Not Found");
  }
}

void ESP_Webtool::terminal_page(void) {  
  String path = "/terminal.html";
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, "text/html"); // And send it to the client
    file.close();                                       // Then close the file again
  }else{
    logs("\tFile Not Found");
  }
}

void ESP_Webtool::fs_page(void) {  
  String path = "/fs_index.html";
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, "text/html"); // And send it to the client
    file.close();                                       // Then close the file again
  }else{
    logs("\tFile Not Found");
  }
}

void ESP_Webtool::handleUpload(void){  
    
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      logs("Web Update :" + String(upload.filename.c_str()));
      String filename = upload.filename;
      if(filename.endsWith(".bin")){
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        logs("Updating Firmware ...");
        print("Updating Firmware ...");
        if (!Update.begin(maxSketchSpace)) { //start with max available size
          Update.printError(Serial);
        }
      }else{
          logs("Not Bin file");
          print("Not Bin file\n");
          if (!filename.startsWith("/")) {
            filename = "/" + filename;
          }
          fsUploadFile = SPIFFS.open(filename, "w");
          filename = String();
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      String filename = upload.filename;
      if(filename.endsWith(".bin")){
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      }else{
         if (fsUploadFile) {
          fsUploadFile.write(upload.buf, upload.currentSize);
        }
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      String filename = upload.filename;
        if(filename.endsWith(".bin")){
          if (Update.end(true)) { //true to set the size to the current progress
            logs(" Web Update Success: "+String(upload.totalSize)+" Rebooting Board...");
            print("Rebooting Board...\n");
            restart = true;
          } else {
            Update.printError(Serial);
          }
        }
        else{
         
          if (fsUploadFile) {
            fsUploadFile.close();
            print("File Uploaded");
         } 
 
          logs("Web Update Success: "+String(upload.totalSize));
          restart = false;
        }
   }
}

ESP_Webtool& ESP_Webtool::setCallback(CALLBACK_SIGNATURE) {
    this->callback = callback;
    return *this;
}


#ifdef ESP8266
void  ESP_Webtool::listFiles(void) {
 logs("\n");
  logs("SPIFFS files found:");

  fs::Dir dir = SPIFFS.openDir("/"); // Root directory
  String  line = "=====================================";

  logs(line);
  logs("  File name               Size");
  logs(line);
   fileslist = "[";
  int i=0;
  while (dir.next()) {
    String fileName = dir.fileName();
    Serial.print(fileName);
    int spaces = 21 - fileName.length(); // Tabulate nicely
    while (spaces--) Serial.print(" ");

    fs::File f = dir.openFile("r");
    String fileSize = (String) f.size();
    spaces = 10 - fileSize.length(); // Tabulate nicely
    while (spaces--) Serial.print(" ");
    logs(fileSize + " bytes");
     if(i==0)
        fileslist += "{\"fname\":\""+ fileName+"\",\"fsize\":\""+fileSize+"\"}";
      else
        fileslist += ",{\"fname\":\""+ fileName+"\",\"fsize\":\""+fileSize+"\"}";
      i++;
  }
 fileslist += "]";
  logs(line);
  logs("\n");
  delay(1000);
}
#endif

//====================================================================================

#ifdef ESP32

void  ESP_Webtool::listFiles(void) {
  listDir(SPIFFS, "/", 0);
}

void  ESP_Webtool::listDir(fs::FS &fs, const char * dirname, uint8_t levels) {

  logs("\n");
  logs("SPIFFS files found:");

  logs("Listing directory: /");
  String  line = "=====================================";

  logs(line);
  logs("  File name               Size");
  logs(line);

  fs::File root = fs.open(dirname);
  if (!root) {
    logs("Failed to open directory");
    return;
  }
  
  if (!root.isDirectory()) {
    logs("Not a directory");
    return;
  }

  fileslist = "[";
  int i=0;
  fs::File file = root.openNextFile();
  while (file) {

    if (file.isDirectory()) {
      Serial.print("DIR : ");
      String fileName = file.name();
      Serial.print(fileName);
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      String fileName = file.name();
      Serial.print("  " + fileName);
      
      int spaces = 20 - fileName.length(); // Tabulate nicely
      while (spaces--) Serial.print(" ");
      String fileSize = (String) file.size();
      spaces = 10 - fileSize.length(); // Tabulate nicely
      while (spaces--) Serial.print(" ");
      logs(fileSize + " bytes");
      if(i==0)
        fileslist += "{\"fname\":\""+ fileName+"\",\"fsize\":\""+fileSize+"\"}";
      else
        fileslist += ",{\"fname\":\""+ fileName+"\",\"fsize\":\""+fileSize+"\"}";
      i++;
    }
    
    file = root.openNextFile();
  }

  fileslist += "]";
  logs(line);
  logs("\n");
  delay(1000);
}

#endif

void ESP_Webtool::handleFileDelete(String filename) {
  if (!filename.startsWith("/")) {
    filename = "/" + filename;
  }

  if (!exists(filename)) {
    return print("FileNotFound");
  }

  SPIFFS.remove(filename);
}

bool ESP_Webtool::exists(String path){
  bool yes = false;
  File file = SPIFFS.open(path, "r");
  if(!file.isDirectory()){
    yes = true;
  }
  file.close();
  return yes;
}
