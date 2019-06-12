#include <WiFi.h>
#include <PubSubClient.h>

#include "DHTesp.h"

#define WIFISSID "Hola" 
#define PASSWORD "123456789"
#define TOKEN "A1E-1pAi8BG2CWNxOyBDT0ovjlYZC6IeVm" 
#define MQTT_CLIENT_NAME "LIFEOFRAM96"

#define TEMP_LABEL "temperature"
#define HUM_LABEL "humidity"
#define HI_LABEL "heatindex"
#define DEVICE_LABEL "daksh19"

char mqttBroker[]  = "app.ubidots.com";
char payload[100];
char topic[150];
char str_sensor[10];

WiFiClient ubidots;
PubSubClient client(ubidots);
DHTesp dht;
void getTemperature();
char temperature[10],humidity[10],heatindex[10];
int dhtPin = 17;

void getTemperature() {
  TempAndHumidity newValues = dht.getTempAndHumidity();
  if (dht.getStatus() != 0) {
    Serial.println("DHT11 error status: " + String(dht.getStatusString()));
  }

  float heatIndex = dht.computeHeatIndex(newValues.temperature, newValues.humidity);
  
  Serial.println(" T:" + String(newValues.temperature) + " H:" + String(newValues.humidity) + " I:" + String(heatIndex) );

  String temp =String(newValues.temperature);
  temp.toCharArray(temperature,10);

  String hum =String(newValues.humidity);
  hum.toCharArray(humidity,10);

  String hin =String(heatIndex);
  hin.toCharArray(heatindex,10); 
}

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFISSID, PASSWORD);
  Serial.println();
  Serial.print("Wait for WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);  


  Serial.println("DHT ESP32 example with tasks");
  dht.setup(dhtPin, DHTesp::DHT11);
  Serial.println("DHT initiated");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  getTemperature();
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", TEMP_LABEL); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s}}", payload,temperature) ; // Adds the value
  
  Serial.write(payload);
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);

  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", HUM_LABEL); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s}}", payload,humidity) ; // Adds the value
  
  Serial.write(payload);
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);

  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", HI_LABEL); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s}}", payload,heatindex) ; // Adds the value
  
  Serial.write(payload);
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  client.loop();
  delay(3000);
}
