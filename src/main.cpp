#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <Arduino.h>

#include <PubSubClient.h>

#include <FastLED.h>

#include "delayable_looper.h"

#define NUM_LEDS_PER_STRIP 4
#define PIXEL_PIN D4
CRGB leds[NUM_LEDS_PER_STRIP];
static const char* topic = "snydergd/work/batsignal/power";
DelayableLooper looper;
WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
  // Loop until we're reconnected
  Serial.print("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  if (client.connect(clientId.c_str())) {
    Serial.println("connected");
    client.subscribe(topic);
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    looper.delay(5000);
  }
}
int ledState = 0;
int ledOn = 0;
// void led_blink() {
//   if (ledState == 2) {
//     if (ledOn) {
//       digitalWrite(LED_BUILTIN, LOW);
//       ledOn = 0;
//     } else {
//       digitalWrite(LED_BUILTIN, HIGH);
//       ledOn = 1;
//     }
//   }
//   looper.delay(1000);
// }
int pixelFade = 0;
int pixelFadeDirection = 1;
void pixel_pattern() {
  if (ledState == 3) { // red emergency
    if (pixelFadeDirection == 1) {
      pixelFade++;
      if (pixelFade >= 255) {
        pixelFadeDirection = 0;
      }
    } else {
      pixelFade--;
      if (pixelFade <= 0) {
        pixelFadeDirection = 1;
      }
    }
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      leds[i] = CRGB(pixelFade, 0, 0);
    }
    FastLED.show();
  } else if (ledState == 4) { // blue emergency
    if (pixelFadeDirection == 1) {
      pixelFade++;
      if (pixelFade >= 255) {
        pixelFadeDirection = 0;
      }
    } else {
      pixelFade--;
      if (pixelFade <= 0) {
        pixelFadeDirection = 1;
      }
    }
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      leds[i] = CRGB(0, 0, pixelFade);
    }
    FastLED.show();
  } else if (ledState == 5) { // red/blue emergency
    if (pixelFadeDirection == 1) {
      pixelFade++;
      if (pixelFade >= 255) {
        pixelFadeDirection = 0;
      }
    } else {
      pixelFade--;
      if (pixelFade <= 0) {
        pixelFadeDirection = 1;
      }
    }
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      if (i % 2 == 0) {
        leds[i] = CRGB(pixelFade, 0, 0);
      } else {
        leds[i] = CRGB(0, 0, pixelFade);
      }
    }
    FastLED.show();
  }

  looper.delay(10);
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      leds[i] = CRGB::White;
    }
    FastLED.show();
    ledState = 0;
    // digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else if ((char)payload[0] == '0') {
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      leds[i] = CRGB::Black;
    }
    FastLED.show();
    ledState = 1;
    // digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  } else {
    // blink
    ledState = ((int)payload[0] - '0');
  }
}

int lightnumber = 1;
CRGB color = CRGB::Red;
void light_show() {
  leds[lightnumber] = color;
  lightnumber++;
  if (lightnumber >= NUM_LEDS_PER_STRIP) {
    lightnumber = 0;
    if (color == CRGB::Red) {
      color = CRGB::Blue;
    } else {
      color = CRGB::Red;
    }
  }
  FastLED.show();
  looper.delay(1000);
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void mqtt_maintenance() {
  if (!client.connected()) {
    Serial.println("Not yet connected");
    reconnect();
  }
  client.loop();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off (Note that LOW is the voltage level

  // tell FastLED to connect to D2 pin with NUM_LEDS_PER_STRIP NeoPixel LEDs
  FastLED.addLeds<WS2811, PIXEL_PIN, RGB>(leds, NUM_LEDS_PER_STRIP);
  for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  FastLED.show();
  digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off (Note that LOW is the voltage level

  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set static ip
  //block1 should be used for ESP8266 core 2.1.0 or newer, otherwise use block2

  //start-block1
  //IPAddress _ip,_gw,_sn;
  //_ip.fromString(static_ip);
  //_gw.fromString(static_gw);
  //_sn.fromString(static_sn);
  //end-block1

  //start-block2
  // IPAddress _ip = IPAddress(10, 0, 1, 78);
  // IPAddress _gw = IPAddress(10, 0, 1, 1);
  // IPAddress _sn = IPAddress(255, 255, 255, 0);
  //end-block2
  
  //wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  wifiManager.setAPCallback(configModeCallback);

  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  client.setServer("broker.hivemq.com", 1883);
  client.setCallback(callback);

  // looper.addLoop(light_show);
  looper.addLoop(mqtt_maintenance);
  // looper.addLoop(led_blink);
  looper.addLoop(pixel_pattern);
}

void loop() {
  looper.loop();
}

