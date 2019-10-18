/*
*  MAZZEMATIC 
*  This is my ESP32 Sketch with a OLED display and an MQTT function
* 
*/

#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <credentials.h>
#include <iplist.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WiFiMulti WiFiMulti1;
WiFiClient espClient;
PubSubClient mqttclient(espClient);

const char* mqtt_server = INC_MQTTBROKERIP;
const unsigned long periodmqtt = 5000;
const int moisturesens = 34;
const int relais = 25;
unsigned long startMillis;
unsigned long currentMillis;
int adcValue = 0;
bool relais1status = 0;

void displaymqttzeug(const String& myString) {
  display.clearDisplay();
  display.setTextSize(2);            
  display.setTextColor(WHITE);        
  display.setCursor(0,0);            
  display.println(myString);
  display.display();
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
    displaymqttzeug(messageTemp);
  }
  if(relais1status == 0){
    digitalWrite(relais, HIGH);
    relais1status = 1;
  } else {
    digitalWrite(relais, LOW);
    relais1status = 0;
  }
  delay(3000);
}

void displaytexdraw(const String& myString) {
  display.clearDisplay();
  display.setTextSize(2);            
  display.setTextColor(WHITE);        
  display.setCursor(0,0);            
  display.println(F(String(myString).c_str()));
  display.setTextSize(1); 
  display.setCursor(0,20); 
  display.println(WiFi.localIP());
  display.display();
}

void setup()
{
    Serial.begin(115200);
    
    pinMode(relais, OUTPUT);
    digitalWrite(relais, LOW);
    delay(100);

    WiFiMulti1.addAP(INC_SSID, INC_PASSWORD);
    mqttclient.setServer(mqtt_server, 1883);
    mqttclient.setCallback(callback);
    Serial.print("Waiting for WiFi... ");
    while(WiFiMulti1.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(500);
    
    startMillis = millis();

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); 
    }

    display.clearDisplay();
    display.display();
    delay(500);
    display.clearDisplay();
    display.setTextSize(2);            
    display.setTextColor(WHITE);
    display.setCursor(0,0);    
    display.println(F("Mazzematic"));       
    display.setCursor(0,20);            
    display.println(F("IP:"));
    display.setTextSize(1); 
    display.setCursor(50,20); 
    display.println(WiFi.localIP());
    display.display();
    delay(5000);  
}

void mqttclientconnect() {
  if (!mqttclient.connected()) {
    Serial.println("Attempting MQTT connection...");
    mqttclient.connect("ESP32");
    mqttclient.subscribe("esp32/test");
  }  
}

void firsttimer(){
  if (currentMillis - startMillis >= periodmqtt)
  {
    adcValue = analogRead(moisturesens);
    mqttclient.publish("esp32/node1/pin34", String(adcValue).c_str(), true);
    String string1 = "Value: "; 
    Serial.println(string1 + adcValue);
    displaytexdraw(String(adcValue).c_str());
    startMillis = currentMillis;
    Serial.println(WiFi.localIP());
  } 
}

void loop()
{
  if (!mqttclient.connected()) {
    mqttclientconnect();
  }
  mqttclient.loop();
  currentMillis = millis();
  firsttimer();
}