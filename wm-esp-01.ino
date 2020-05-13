#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NewPing.h>

#define HCSR04_PIN_TRIG 2
#define HCSR04_PIN_ECHO 0

const char* ssid = "SAYANI_WIFI";
const char* password = "00011101";
const char* mqtt_server = "192.168.8.10";

WiFiClient espClient;
PubSubClient client(espClient);
NewPing hcsr04(HCSR04_PIN_TRIG,HCSR04_PIN_ECHO);

char msg[50];

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "WM-ESP-01";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  client.setServer(mqtt_server, 1883);
}

void loop() {

  WiFi.forceSleepWake();
  
  setup_wifi();

  if (!client.connected()) {
    reconnect();
  }
    
  int hcsr04Dist = hcsr04.ping_cm();
  delay(10);
  snprintf (msg, 50, "%ld", hcsr04Dist);
  Serial.print(F("Distance: ")); Serial.print(hcsr04Dist); Serial.println(F("[cm]"));
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("home/terrace/tank/water/level", msg);
  client.disconnect();
  Serial.println("Disconnected MQTT");

  delay(500);
  WiFi.disconnect();
  Serial.println("Disconnected WiFi");

  WiFi.forceSleepBegin();

  delay(1000 * 60 * 5); // five minutes
}
