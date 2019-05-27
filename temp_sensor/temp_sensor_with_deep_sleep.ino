#include <ESP8266WiFi.h>
#include <OneWire.h>

const int sleepTimeS = 30;
const char* host = "192.168.44.8"; 
const int httpGetPort = 80;
const char* ssid     = "Los-Kabellos";
const char* password = "habostorta29!";
float celsius;
const char* urlGet = "/temp";
ADC_MODE(ADC_VCC);

OneWire  ds(2);

void setup() {

Serial.begin(115200);
delay(10);

Serial.print("Connecting to ");
Serial.println(ssid);

WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
Serial.println("");
Serial.println("WiFi connected");
// Print the IP address
Serial.println(WiFi.localIP());
while(!Serial) {}

byte i;
byte present = 0;
byte type_s;
byte data[12];
byte addr[8];
int current;

if ( !ds.search(addr)) {
Serial.println("No more addresses.");
Serial.println();
ds.reset_search();
delay(250);
return;
}

Serial.print("ROM =");
for ( i = 0; i < 8; i++) {
Serial.write(' ');
Serial.print(addr[i], HEX);
}

if (OneWire::crc8(addr, 7) != addr[7]) {
Serial.println("CRC is not valid!");
return;
}
Serial.println();

// the first ROM byte indicates which chip
switch (addr[0]) {
case 0x10:
Serial.println("  Chip = DS18S20");  // or old DS1820
type_s = 1;
break;
case 0x28:
Serial.println("  Chip = DS18B20");
type_s = 0;
break;
case 0x22:
Serial.println("  Chip = DS1822");
type_s = 0;
break;
default:
Serial.println("Device is not a DS18x20 family device.");
return;
}

ds.reset();
ds.select(addr);
ds.write(0x44, 1);       
delay(1000);
present = ds.reset();
ds.select(addr);
ds.write(0xBE);
Serial.print("  Data = ");
Serial.print(present, HEX);
Serial.print(" ");
for ( i = 0; i < 9; i++) {
data[i] = ds.read();
Serial.print(data[i], HEX);
Serial.print(" ");
}
Serial.print(" CRC=");
Serial.print(OneWire::crc8(data, 8), HEX);
Serial.println();

int16_t raw = (data[1] << 8) | data[0];
if (type_s) {
raw = raw << 3; // 9 bit resolution default
if (data[7] == 0x10) {
// "count remain" gives full 12 bit resolution
raw = (raw & 0xFFF0) + 12 - data[6];
}
} else {
byte cfg = (data[4] & 0x60);
// at lower res, the low bits are undefined, so let's zero them
if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
//// default is 12 bit resolution, 750 ms conversion time
}
celsius = (float)raw / 16.0;
//current = round(100-((3000-ESP.getVcc())/9));
Serial.print("  Temperature = ");
Serial.print(celsius);
Serial.print(" °C ");
Serial.print(current);
Serial.print(" V ");
return;
}

WiFiClient clientGet;
   

   //the path and file to send the data to:

 
  // We now create and add parameters:

//urlGet += "?temp=" + String(celsius);
   
Serial.print(">>> Connecting to host: ");
Serial.println(host);
      
       if (!clientGet.connect(host, httpGetPort)) {
        Serial.print("Connection failed: ");
        Serial.print(host);
      } else {
          clientGet.println("GET " + "/temp?temp=" + String(celsius) + " HTTP/1.1");
          clientGet.print("Host: ");
          clientGet.println(host);
          clientGet.println("User-Agent: ESP8266/1.0");
          clientGet.println("Connection: close\r\n\r\n");
          
          unsigned long timeoutP = millis();
          while (clientGet.available() == 0) {
            
            if (millis() - timeoutP > 10000) {
              Serial.print(">>> Client Timeout: ");
              Serial.println(host);
              clientGet.stop();
              return;
            }
          }

           //just checks the 1st line of the server response. Could be expanded if needed.
          while(clientGet.available()){
            String retLine = clientGet.readStringUntil('\r');
            Serial.println(retLine);
            break; 
          }

      } //end client connection if else
                        
      Serial.print(">>> Closing host: ");
      Serial.println(host);
          
      clientGet.stop();
        Serial.print("Entering deep sleep");
  ESP.deepSleep(sleepTimeS *1000000);

}

void loop() {
}
