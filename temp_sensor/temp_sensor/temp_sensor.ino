#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include "config.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


const int oneWireBus = 2;
const char* host = "wetter";
const char* ssid = STASSID;
const char* password = STAPSK; 

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

void setup(void) {

  Serial.begin(115200);
  Serial.println();

int prg = 1;
 
if (prg == 0) {
  //Frissito uzem
  Serial.println("Inditas...");
  Serial.println("Frissito uzem");
  //Kepernyot be
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
    }
  delay(1000);
  display.clearDisplay();
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);
  display.cp437(true);
  display.setTextSize(1);
  
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  display.clearDisplay();
  display.write("IP: "+WiFi.localIP());

    server.on("/", handleRoot);
    httpUpdater.setup(&server); 
    server.begin();
    display.clearDisplay();
    display.write("Waiting for new firmware...");

}else{
  //Normal uzem
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("IP-cim: ");
  Serial.println(WiFi.localIP());

    server.on("/temp", tempDataSubmit);
    server.on("/", handleRoot);
    httpUpdater.setup(&server); 
    server.begin();
    sensors.begin();
  displayTemp();
  tempDataSubmit();
  delay(5000);
}
}
void loop(void) {
  server.handleClient();
}

void handleRoot(){
  server.send(200, "text/plain","OK");
}

char* readTempSensor(){
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  int tempCint = (int) temperatureC;
  String tempC = String(tempCint);
  char tempCchar[50];
  tempC.toCharArray(tempCchar, tempC.length()+1);
  return tempCchar;
}

void tempDataSubmit(void) {
  HTTPClient http;
   http.begin("http://cloudspeaker.local/temp?temp="+String(readTempSensor()));
   int httpCode = http.GET();
 
    if (httpCode > 0) {
 
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
      }
 
    else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources
   http.end();  //Close connection
   server.send(200, "text/plain", String(readTempSensor()));
   Serial.println("Going into deep sleep for 20 minutes");
  ESP.deepSleep(1200e6); 
}

void displayTemp(void) {
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
    }
  delay(1000);
  display.clearDisplay();
  display.setTextSize(3);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.write(readTempSensor());
  display.setTextSize(1);
  display.write(char(248));
  display.setTextSize(3);
  display.write("C");
  display.display();
  delay(10000);
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  delay(10000);
}
