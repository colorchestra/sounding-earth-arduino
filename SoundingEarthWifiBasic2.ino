#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>

WiFiMulti WiFiMulti;
WiFiClient espClient;
PubSubClient client(espClient);

// Wi-Fi credentials
const char* ssid = "soundingearth"; // add Wifi name here
const char* password = ""; // add Wifi password here

// MQTT broker
const char* mqtt_server = "192.168.8.100"; // add MQTT server IP here
const char* mqtt_user = "esp"; // add MQTT username here
const char* mqtt_pass = ""; // add MQTT password here

// uncomment the corresponding line for each ESP!
const char* mqtt_sub_topic = "india/#";
//const char* mqtt_sub_topic = "bhutan/#";
//const char* mqtt_sub_topic = "indonesia/#";

unsigned long lastWiFiCheck = 0;
unsigned long lastMQTTAttempt = 0;
const long reconnectInterval = 5000;

void setup() {
  Serial.begin(115200);

  // Wi-Fi
  WiFiMulti.addAP(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected, IP: " + WiFi.localIP().toString());

  // MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // Wi-Fi reconnect
  if ((WiFi.status() != WL_CONNECTED) && (millis() - lastWiFiCheck > reconnectInterval)) {
    Serial.println("WiFi disconnected, retrying...");
    lastWiFiCheck = millis();
    WiFi.disconnect();
    WiFiMulti.run();
  }

  // MQTT reconnect
  if (!client.connected() && millis() - lastMQTTAttempt > reconnectInterval) {
    Serial.println("Attempting MQTT reconnect...");
    lastMQTTAttempt = millis();
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("Connected to MQTT");
      client.subscribe(mqtt_sub_topic);
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.println(client.state());
    }
  }

  client.loop();
}

// --- Your custom functions ---

// add code for turning the light on or off
// payload will be "on" or "off"
void handleLightCommand(String payload) {
  Serial.println("Light was switched " + payload);
}

// add code for turning the stream on or off
// payload will be "on" or "off"
void handleStreamCommand(String payload) {
  Serial.println("Stream was switched " + payload);

}

// add code for turning the instrument on or off
// payload will be "on" or "off"
void handleInstrumentCommand(String payload) {
  Serial.println("Instrument was switched " + payload);
}

// MQTT callback
void callback(char* topic, byte* message, unsigned int length) {
  String topicStr = String(topic);
  String payload;

  for (int i = 0; i < length; i++) {
    payload += (char)message[i];
  }

  Serial.println("Message arrived:");
  Serial.println("Topic: " + topicStr);
  Serial.println("Payload: " + payload);

  // Extract the last part of the topic (after last '/')
  int lastSlash = topicStr.lastIndexOf('/');
  String command = topicStr.substring(lastSlash + 1);

  if (command == "light") {
    handleLightCommand(payload);
  } else if (command == "stream") {
    handleStreamCommand(payload);
  } else if (command == "instrument") {
    handleInstrumentCommand(payload);
  } else {
    Serial.println("Unhandled topic segment: " + command);
  }
}
