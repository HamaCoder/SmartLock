#include <WiFi.h>
#include <WebServer.h>
#define LED_PIN GPIO_NUM_2
#define LED_ON HIGH
#define LED_OFF LOW

const char* ssid = "Ngoc Yen Coffee";
const char* password = "66668888";
//const char* ssid = "Trong Thuc";//"YourNetworkName";
//const char* password = "Cotroimoibietnhe";//"YourNetworkPassword";
const char *ssidAP = "DEMOESP32AP";
const char *passwordAP = "123456789";
WebServer server(80);
bool LEDstatus = LED_OFF;


void handleNotFound() {
 String message = "File Not Found\n\n";
 message += "URI: ";
 message += server.uri();
 message += "\nMethod: ";
 message += (server.method() == HTTP_GET) ? "GET" : "POST";
 message += "\nArguments: ";
 message += server.args();
 message += "\n";
 for (uint8_t i = 0; i < server.args(); i++) {
 message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
 }
 server.send(404, "text/plain", message);
}



String SendHTML(uint8_t ledstat) {
 String ptr = "<!DOCTYPE html> <html>\n";
 ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
 ptr += "<title>LED Control</title>\n";
 ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
 ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
 ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
 ptr += ".button-on {background-color: #3498db;}\n";
 ptr += ".button-on:active {background-color: #2980b9;}\n";
 ptr += ".button-off {background-color: #34495e;}\n";
 ptr += ".button-off:active {background-color: #2c3e50;}\n";
 ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
 ptr += "</style>\n";
 ptr += "</head>\n";
 ptr += "<body>\n";
 ptr += "<h1>ESP32 Web Server</h1>\n";
 if (ledstat)
 {
 ptr += "<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/ledoff\">OFF</a>\n";
 }
 else
 {
 ptr += "<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/ledon\">ON</a>\n";
 }
 ptr += "</body>\n";
 ptr += "</html>\n";
 return ptr;
}



void handleRoot() {
 server.send(200, "text/html", SendHTML(LEDstatus));
}



void setup(void) {
 Serial.begin(115200);
 pinMode(LED_PIN, OUTPUT);//Cáº¥u hÃ¬nh GPIO2 lÃ  output
 digitalWrite(LED_PIN, LED_OFF);
 WiFi.mode(WIFI_AP_STA);
 WiFi.softAP(ssidAP, passwordAP);
 Serial.println(WiFi.softAPIP());
 WiFi.begin(ssid, password);
 Serial.println("");
 // Wait for connection
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }
 Serial.println("");
 Serial.print("Connected to ");
 Serial.println(ssid);
 Serial.print("IP address: ");
 Serial.println(WiFi.localIP());
 // server.on("/", []() {
 // server.send(200, "text/plain", "ESP32 Webserver running");
 // });
 server.on("/", handleRoot);
 server.on("/ledon", []() {
 LEDstatus = LED_ON;
 Serial.println("Set LED to ON");
 server.send(200, "text/html", SendHTML(LEDstatus));
 });
 server.on("/ledoff", []() {
 LEDstatus = LED_OFF;
 Serial.println("Set LED to OFF");
 server.send(200, "text/html", SendHTML(LEDstatus));
 });
 server.onNotFound(handleNotFound);
 server.begin();
 Serial.println("HTTP server started");
}


void loop(void) {
 server.handleClient();
 digitalWrite(LED_PIN, LEDstatus);
}
