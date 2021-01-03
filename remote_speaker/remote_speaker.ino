

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "i2s.h"
#include "StatusLedModes.h"
#include "config.h"

extern "C" {
  #include "user_interface.h"
}

const char* ssid = STASSID;
const char* password = STAPSK;

const uint8_t staticIP[] = {0, 0, 0, 0};   // set to 0, 0, 0, 0 for DHCP
const uint8_t gwIP[] = {192, 168, 1, 1};
const uint8_t subnet[] = {255, 255, 255, 0};
const uint16_t port = 5522;

WiFiServer server(port);

// valid buffer Sizes are e.g. 0x1000, 0x2000, 0x4000
#define BUFFER_SIZE    0x4000
#define ONBOARD_LED    2
#define STATUS_LED     12
#define AMP_ENABLE_PIN 15

uint8_t buffer8b[BUFFER_SIZE];
uint16_t bufferPtrIn;
uint16_t bufferPtrOut;
uint32_t ultimeout;
uint8_t toggleOffOn = 0;
uint8_t OTA_update = 0;

enum RAMP_DIRECTIONS { DOWN, UP };
enum PWM_MODES { PWM_RESET, PWM_NORMAL, PWM_DIRECT };
volatile STATUS_LED_MODES statusLEDmode = WIFI_DISCONNECTED;

// **************************************************
// **************************************************
// **************************************************
void ICACHE_RAM_ATTR statusLED_ISR()
{
  switch (statusLEDmode)
  {
    default:                  // --
    case WIFI_DISCONNECTED:   timer0_write(ESP.getCycleCount() + (160000 * 250));       // om & off-time [ms]
                              break;
    case WIFI_CONNECTED:      if (toggleOffOn)
                                timer0_write(ESP.getCycleCount() + (160000 * 20));      // off-time [ms]
                              else
                                timer0_write(ESP.getCycleCount() + (160000 * 2000));    // on-time [ms]
                              break;
    case STREAMING:           timer0_write(ESP.getCycleCount() + (160000 * 1000));      // dummy timer
                              toggleOffOn = 1;
                              break;
  }

  if (toggleOffOn || OTA_update)
  {
    digitalWrite(STATUS_LED, LOW);
    //digitalWrite(ONBOARD_LED, LOW);
  } else {
    digitalWrite(STATUS_LED, HIGH);
    //digitalWrite(ONBOARD_LED, HIGH);
  }

  toggleOffOn ^= 0x01;
}

// **************************************************
// **************************************************
// **************************************************
void setStatusLEDmode(STATUS_LED_MODES new_statusLEDmode)
{
  if (statusLEDmode != new_statusLEDmode)
  {
    toggleOffOn = 0;
    statusLEDmode = new_statusLEDmode;

    // update timer immediately
    timer0_write(ESP.getCycleCount() + 160000);
  }
}

// **************************************************
// **************************************************
// **************************************************
void WiFiStart()
{
  // connect to wifi
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) 
    yield();

  setStatusLEDmode(WIFI_CONNECTED);

  // use static IP if specified - otherwise use DHCP
  if (*staticIP != '\0') 
  {
    IPAddress ip(staticIP[0], staticIP[1], staticIP[2], staticIP[3]);
    IPAddress gateway(gwIP[0], gwIP[1], gwIP[2], gwIP[3]);
    IPAddress subnet(subnet[0], subnet[1], subnet[2], subnet[3]);
    WiFi.config(ip, gateway, subnet);
  }

  // start server
  server.begin();
}

// **************************************************
// **************************************************
// **************************************************
void setup()
{
  system_update_cpu_freq(SYS_CPU_160MHZ);

  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);

  timer0_isr_init();
  timer0_attachInterrupt(statusLED_ISR);
  timer0_write(ESP.getCycleCount() + 160000000);  
  interrupts();

  WiFi.mode(WIFI_STA);
  WiFiStart();

  i2s_begin();
  i2s_set_rate(96000);      // 33 ksps

  wifi_status_led_uninstall();

  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, HIGH);

  pinMode(AMP_ENABLE_PIN, OUTPUT);
  digitalWrite(AMP_ENABLE_PIN, LOW);

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
  {
    OTA_update = 1;
    timer0_write(ESP.getCycleCount() + 160000);
  });
  ArduinoOTA.onError([](ota_error_t error)
  {
    OTA_update = 0;
    timer0_write(ESP.getCycleCount() + 160000);
  });
  ArduinoOTA.onEnd([]()
  {
    OTA_update = 0;
    timer0_write(ESP.getCycleCount() + 160000);
  });
  ArduinoOTA.setHostname("ESP8266_speaker");
  ArduinoOTA.begin();
}

// **************************************************
// **************************************************
// **************************************************
// ctrl = 0 : reset the PWM
// ctrl = 1 : normal playback
// ctrl = 2 : direct 8-bit value output
//
inline void doPWM(uint8_t ctrl, uint8_t value8b)
{
  static uint8_t dly = 0;
  static uint8_t sample = 0;
  static uint8_t dWordNr = 0;
  static uint32_t i2sbuf32b[4];
  static uint8_t prng;
  uint8_t shift;

  // reset
  if (ctrl == PWM_RESET)
  {
    dWordNr = 0;
    sample = 0;
    i2sbuf32b[0] = 0;
    i2sbuf32b[1] = 0;
    i2sbuf32b[2] = 0;
    i2sbuf32b[3] = 0;
    return;
  }

  // playback
  if (i2s_write_sample_nb(i2sbuf32b[dWordNr]))
  {
    dWordNr = (dWordNr + 1) & 0x03;

    // previous 4 DWORDS written?
    if (dWordNr == 0x00)
    {
      yield();

      // normal playback
      if (ctrl == PWM_NORMAL)
      {
        // new data in ring-buffer?
        if (bufferPtrOut != bufferPtrIn)
        {
          sample = buffer8b[bufferPtrOut];
          bufferPtrOut = (bufferPtrOut + 1) & (BUFFER_SIZE - 1);
        }
      } else if (ctrl == PWM_DIRECT) 
      {
        // direct output mode
        sample = value8b;
      }

      shift = sample >> 1;

      if (!(sample & 0x01))
      {
        shift -= (prng & 0x01);     // subtract 0 or 1 from "shift" for dithering
        prng ^= 0x01;
      }

      yield();

      shift = 0x80 - shift;     // inverse shift
      
      if (shift < 0x20)
      {
        i2sbuf32b[0] = 0xFFFFFFFF >> shift;
        i2sbuf32b[1] = 0xFFFFFFFF;
        i2sbuf32b[2] = 0xFFFFFFFF;
        i2sbuf32b[3] = 0xFFFFFFFF;
      } else if (shift < 0x40)
      {
        i2sbuf32b[0] = 0x00000000;
        i2sbuf32b[1] = 0xFFFFFFFF >> (shift - 0x20);
        i2sbuf32b[2] = 0xFFFFFFFF;
        i2sbuf32b[3] = 0xFFFFFFFF;
      } else if (shift < 0x60)      
      {
        i2sbuf32b[0] = 0x00000000;
        i2sbuf32b[1] = 0x00000000;
        i2sbuf32b[2] = 0xFFFFFFFF >> (shift - 0x40);
        i2sbuf32b[3] = 0xFFFFFFFF;
      } else if (shift < 0x80)
      {
        i2sbuf32b[0] = 0x00000000;
        i2sbuf32b[1] = 0x00000000;
        i2sbuf32b[2] = 0x00000000;
        i2sbuf32b[3] = 0xFFFFFFFF >> (shift - 0x60);
      } else {
        i2sbuf32b[0] = 0x00000000;
        i2sbuf32b[1] = 0x00000000;
        i2sbuf32b[2] = 0x00000000;
        i2sbuf32b[3] = 0x00000000;
      }
    }
  }
}

// **************************************************
// **************************************************
// **************************************************
void rampPWM(uint8_t direction)
{
  uint8_t dl = 0;

  for (uint8_t value = 0; value < 0x80; value++)
  {
    while (dl++ < 250)
    {
      if (direction == UP)
        doPWM(PWM_DIRECT, value);
      else
        doPWM(PWM_DIRECT, 0x80 - value);
    }
    dl = 0;
  }
}

// **************************************************
// **************************************************
// **************************************************
inline void startStreaming(WiFiClient *client)
{
  uint8_t i;
  uint8_t dl;

  bufferPtrIn = 0;
  bufferPtrOut = 0;

  setStatusLEDmode(STREAMING);
  digitalWrite(AMP_ENABLE_PIN, HIGH);

  // ===================================================================================
  // fill buffer
  
  ultimeout = millis() + 500;
  do
  {
    // yield();
    
    if (client->available())
    {
      buffer8b[bufferPtrIn] = client->read();
      bufferPtrIn = (bufferPtrIn + 1) & (BUFFER_SIZE - 1);
      ultimeout = millis() + 500;
    }
  } while ((bufferPtrIn < (BUFFER_SIZE - 1)) && (client->connected()) && (millis() < ultimeout));

  if ((!client->connected()) || (millis() >= ultimeout))
    return;
  
  // ===================================================================================
  // ramp-up PWM to 50% (=Vspeaker/2) to avoid "blops"  

  rampPWM(UP);
  
  // ===================================================================================
  // start playback

  ultimeout = millis() + 500;
  do
  {
    doPWM(PWM_NORMAL,0);

    // new data in wifi rx-buffer?
    if (client->available())
    {
      // ring-buffer free?
      if (((bufferPtrIn + 1) & (BUFFER_SIZE - 1)) != bufferPtrOut)
      {
        buffer8b[bufferPtrIn] = client->read();
        bufferPtrIn = (bufferPtrIn + 1) & (BUFFER_SIZE - 1);
      }
    }

    if (bufferPtrOut != bufferPtrIn)
      ultimeout = millis() + 500;

  } while (client->available() || (millis() < ultimeout) || (bufferPtrOut != bufferPtrIn));

  // disabling the amplifier does not "plop", so it's better to do it before ramping down
  digitalWrite(AMP_ENABLE_PIN, LOW);

  // ===================================================================================
  // ramp-down PWM to 0% (=0Vdc) to avoid "blops"
  rampPWM(DOWN);



}

// **************************************************
// **************************************************
// **************************************************
void loop()
{
  ArduinoOTA.handle();
  yield();
  
  // reconnect if wifi got disconnected
  if (WiFi.status() != WL_CONNECTED)
  {
    setStatusLEDmode(WIFI_DISCONNECTED);
    WiFiStart();
  } else {
    setStatusLEDmode(WIFI_CONNECTED);
  }

  WiFiClient client = server.available();

  // new client?
  if (client)
  {
    startStreaming(&client);
    client.stop();
  }
}
