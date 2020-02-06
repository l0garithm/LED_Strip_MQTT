#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

#define LED_PIN     D2
#define NUM_LEDS    119

WiFiClient homeClient;
PubSubClient client(homeClient);

const char* ssid = "MySpectrumWiFib8-2G";
const char* password = "livelywhale398";

const char* mqtt_server = "192.168.1.241";

const int led = D1;

int brightness = 255;
int redValue = 255;
int blueValue = 255;
int greenValue = 255;
int refreshSpeed = 50;
String command = "";
int colorShift = 10;

String state = "";
StaticJsonDocument<256> jsonDoc;

CRGB leds[NUM_LEDS];


void setup(void){
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  setup_wifi();
  setup_OTA();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.publish("test","connected",true);
}

void loop(void){
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("Vert_Strip_1");
}

void setup_wifi(){
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
}

void setup_OTA(){
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("Logans_Vert_Strip_1");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void callback(String topic, byte* message, unsigned int length){
  
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.println("Deserializing JSON...");
  
  deserializeJson(jsonDoc, message, length);

  checkCommand(topic);
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Vert_Strip_1")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("/logansroom/vert_strip_1");
      client.publish("/logansroom/", "connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void checkCommand(String topic){
    if(topic == "/logansroom/vert_strip_1"){
    Serial.print("Changing LED Strip to: ");
    if(jsonDoc["command"] == "solidColor"){
      redValue = jsonDoc["red"];
      greenValue = jsonDoc["green"];
      blueValue = jsonDoc["blue"];
      brightness = jsonDoc["brightness"];
      
      Serial.print("Setting Strip to Solid: ");
      //Serial.print("(" + jsonDoc[String("red")] + ", "jsonDoc[String("green")] + ", " + jsonDoc[String("blue")] + ")");
      allSolid(redValue, greenValue, blueValue, brightness);
      Serial.println(brightness);
    }
    else if(jsonDoc["command"] == "fire"){
      refreshSpeed = jsonDoc["speed"];
      brightness = jsonDoc["brightness"];
      
      Serial.println("Setting Strip to Fire");
      while(1){
        if (!client.connected()) {
          reconnect();
        }
        if(!client.loop()){
           client.connect("Vert_Strip_1"); 
           break;
        }
        if(jsonDoc["command"] != "fire")
        {
          Serial.println("Command Changed");
          checkCommand(topic);
          break;
        }
      Fire(55, 120, refreshSpeed, brightness); 
      }
    }
    else if(jsonDoc["command"] == "fadeOneColor"){
      redValue = jsonDoc["red"];
      greenValue = jsonDoc["green"];
      blueValue = jsonDoc["blue"];

      Serial.println("Setting Strip to Fade");
      while(1){
        Serial.println("Loop");
        if (!client.connected()) {
          Serial.println("Reconnecting");
          reconnect();
        }
        if(!client.loop()){
           Serial.println("Client Connect");
           client.connect("Vert_Strip_1"); 
           break;
        }
        if(jsonDoc["command"] != "fadeOneColor")
        {
          Serial.println("Command Changed");
          checkCommand(topic);
          break;
        }
        Serial.println("RED");
        FadeInOut(255, 0, 0); // red
        Serial.println("WHITE");
        FadeInOut(255, 255, 255); // white
        Serial.println("BLUE");
        FadeInOut(0, 255, 255); // blue
      }
    }
    else if(jsonDoc["command"] == "meteorRain"){
      redValue = jsonDoc["red"];
      greenValue = jsonDoc["green"];
      blueValue = jsonDoc["blue"];

      Serial.println("Launching Meteors");
      while(checkCommandUpdate()){
        Serial.println("Loop");
        if (!client.connected()) {
          Serial.println("Reconnecting");
          reconnect();
        }
        if(!client.loop()){
           Serial.println("Client Connect");
           client.connect("Vert_Strip_1"); 
           break;
        }
        if(jsonDoc["command"] != "meteorRain")
        {
          Serial.println("Command Changed");
          checkCommand(topic);
          break;
        }
        meteorRain(150,55,255,10,64,true,30);
      }
    }
    else if(jsonDoc["command"] == "off"){
      Serial.println("Setting Strip to OFF");
      allSolid(0,0,0,0);
    }
  }
}

int checkCommandUpdate(){
  if (!client.connected()) {
    Serial.println("Reconnecting");
    reconnect();
  }
  if(!client.loop()){
     Serial.println("Client Connect");
     client.connect("Vert_Strip_1"); 
     return 0;
  }
  return 1;
}

void allSolid(int red, int green, int blue, int brightness){
  for(int x = 0; x < NUM_LEDS; x++){
    redValue = red;
    blueValue = blue;
    greenValue = green;
    leds[x] = CRGB(redValue, greenValue, blueValue);
  }
  FastLED.setBrightness(brightness);
  FastLED.show();
}

void Fire(int Cooling, int Sparking, int SpeedDelay, int brightness) {
  static byte heat[NUM_LEDS];
  int cooldown;
 
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS; i++) {
    cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);
   
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
 
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
   
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    setPixelHeatColor(j, heat[j] );
  }

  FastLED.setBrightness(brightness);
  FastLED.show();
  delay(SpeedDelay);
}

void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    setPixel(Pixel, (255), (255), (heatramp));
  } else if( t192 > 0x40 ) {             // middle
    setPixel(Pixel, (255), (heatramp), 0);
  } else {                               // coolest
    setPixel(Pixel, (heatramp), 0, 0);
  }
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
   // FastLED
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
}

void FadeInOut(int red, int green, int blue){
  float r, g, b;
     
  for(int k = 0; k < 256; k=k+1) {
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    delay(10);
    allSolid(r,g,b,255);
    if (!client.connected()) {
      Serial.println("Reconnecting");
      reconnect();
    }
    if(!client.loop()){
       Serial.println("Client Connect");
       client.connect("Vert_Strip_1"); 
       break;
    }
    if(jsonDoc["command"] != "fadeOneColor")
    {
      Serial.println("Command Changed");
      break;
    }
  }
     
  for(int k = 255; k >= 0; k=k-2) {
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    delay(10);
    allSolid(r,g,b,255);
    if (!client.connected()) {
      Serial.println("Reconnecting");
      reconnect();
    }
    if(!client.loop()){
       Serial.println("Client Connect");
       client.connect("Vert_Strip_1"); 
       break;
    }
    if(jsonDoc["command"] != "fadeOneColor")
    {
      Serial.println("Command Changed");
      break;
    }
  }
}

void meteorRain(int red, int green, int blue, int meteorSize, int meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  allSolid(0,0,0, brightness);
 
  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {
   
   
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
   
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        setPixel(i-j, red, green, blue);
      }
    }
   
    FastLED.show();
    delay(SpeedDelay);
    if(!checkCommandUpdate()){
      break;
    }
  }
}

void fadeToBlack(int ledNo, int fadeValue) {
// #ifdef ADAFRUIT_NEOPIXEL_H
//    // NeoPixel
//    uint32_t oldColor;
//    uint8_t r, g, b;
//    int value;
//   
//    oldColor = strip.getPixelColor(ledNo);
//    r = (oldColor & 0x00ff0000UL) >> 16;
//    g = (oldColor & 0x0000ff00UL) >> 8;
//    b = (oldColor & 0x000000ffUL);
//
//    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
//    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
//    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
//   
//    strip.setPixelColor(ledNo, r,g,b);
// #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   leds[ledNo].fadeToBlackBy( fadeValue );
 #endif  
}
//void rainbowCycle(int SpeedDelay) {
//  byte *c;
//  uint16_t i, j;
//
//  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
//    for(i=0; i< NUM_LEDS; i++) {
//      c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);
//      setPixel(i, *c, *(c+1), *(c+2));
//      checkCommandUpdate();
//    }
//    showStrip();
//    delay(SpeedDelay);
//  }
//}
//
//byte * Wheel(byte WheelPos) {
//  static byte c[3];
// 
//  if(WheelPos < 85) {
//   c[0]=WheelPos * 3;
//   c[1]=255 - WheelPos * 3;
//   c[2]=0;
//  } else if(WheelPos < 170) {
//   WheelPos -= 85;
//   c[0]=255 - WheelPos * 3;
//   c[1]=0;
//   c[2]=WheelPos * 3;
//  } else {
//   WheelPos -= 170;
//   c[0]=0;
//   c[1]=WheelPos * 3;
//   c[2]=255 - WheelPos * 3;
//  }
//
//  return c;
//}
