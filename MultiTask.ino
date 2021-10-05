#include <WiFi.h>
#include <PubSubClient.h>
#define mqtt_port 1883

TaskHandle_t t0;
TaskHandle_t t1;

const char* ssid = "HengHeng_5G";
const char* password = "bank1234";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient *client = NULL; // pointer

long lastMsg = 0;
char msg[50];
int value = 0;

long lastReconnectAttempt = 0;
String STATE = "None";

boolean reconnect() {
  if (client->connect("arduinoClient")) {
    Serial.println("MQTT connected");
    client->publish("device/outTopic", "Complete");
    client->subscribe("device/inTopic");
  }
  return client->connected();
}

void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
}

void callb(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}



void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  
 
  xTaskCreatePinnedToCore(
    tLedAndSerialFunc,  
    "LedAndSerial",     
    10000,              
    NULL,               
    1,                  
    &t1,                
    0);                 
  delay(500);
  
  
  xTaskCreatePinnedToCore(
    tNetworkFunc,       
    "Network",          
    10000,              
    NULL,               
    1,                  
    &t0,                
    1);                 
  delay(500);
}




void tNetworkFunc(void *params) {

  Serial.print("tNetworkFunc running on core ");
  Serial.println(xPortGetCoreID());
  setup_wifi();
  
  
  while (true) {
    if (WiFi.status() != WL_CONNECTED) { 
      STATE = "None";
    } else if (WiFi.status() == WL_CONNECTED && STATE == "None") {\
     
      Serial.println("WiFi connected");
      STATE = "WiFi-Connect";
      client = new PubSubClient(espClient);
      client->setServer(mqtt_server, mqtt_port);
      client->setCallback(callback);
    } else if (WiFi.status() != WL_CONNECTED && STATE == "MQTT-Connect") {
     
      delete client;
      STATE = "None";
    } else { 
      if (!client->connected()) {
        long now = millis();
        if (now - lastReconnectAttempt > 5000) {
          lastReconnectAttempt = now;
         
          if (reconnect()) {
            lastReconnectAttempt = 0;
          }
        }
        STATE = "WiFi-Connect";
      } else {
       
        STATE = "MQTT-Connect";
        client->loop();

        long now = millis();
        if (now - lastMsg > 2000) {
          lastMsg = now;
          ++value;
          snprintf (msg, 50, "Hello World #%ld", value);
          Serial.print("Publish message: ");
          Serial.println(msg);
          client->publish("device/outTopic", msg);
        }
      }
    }
    delay(10);
  }
}

void loop() {

}
