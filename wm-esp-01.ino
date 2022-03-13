#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NewPing.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NoDelay.h>

#define HCSR04_PIN_TRIG 2
#define HCSR04_PIN_ECHO 0

const char* ssid = "SAYANI_WIFI";
const char* password = "00011101";
const char* mqtt_server = "192.168.8.10"; //pi4

WiFiClient espClient;
PubSubClient mqttClient(espClient);
NewPing hcsr04(HCSR04_PIN_TRIG,HCSR04_PIN_ECHO);
noDelay hcsr04Time(1000);

char msg[50];

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println("Booting");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

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

  randomSeed(micros());

  Serial.println("");
  Serial.println("Ready");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void disconnect_wifi() {
  delay(10);

  Serial.println();
  Serial.print("Disconnecting from ");
  Serial.println(ssid);
  
  WiFi.disconnect(true);
  Serial.println("WiFi dsconnected");
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {

    ArduinoOTA.handle();
    
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 1 second before retrying
      delay(1000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  mqttClient.setServer(mqtt_server, 1883);
}

void loop() {

  ArduinoOTA.handle();

  if (!mqttClient.connected()) {
    reconnect();
  }

  mqttClient.loop();

  if(hcsr04Time.update()) {
    int dist = hcsr04.ping_cm();
    delay(10);
    snprintf (msg, 50, "%ld", dist);
    Serial.print(F("Distance: ")); Serial.print(dist); Serial.println(F("[cm]"));
    Serial.print("Publish message: ");
    Serial.println(msg);
    mqttClient.publish("home/terrace/tank/water/level", msg);
  }

}
