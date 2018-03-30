#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Wifi: SSID and password
const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

// MQTT: ID, server IP, port, username and password
const PROGMEM char* MQTT_CLIENT_ID = "";
const PROGMEM char* MQTT_SERVER_IP = "";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "";
const PROGMEM char* MQTT_PASSWORD = "";

// MQTT: topics
char* MQTT_LIGHT_STATE_TOPIC = "casa/Suite/ledBeta";
char* MQTT_LIGHT_COMMAND_TOPIC = "casa/Suite/ledBeta/set";

// payloads by default (on/off)
const char* LIGHT_ON = "ON";
const char* LIGHT_OFF = "OFF";

int LED_PIN1 = D1;
int LED_PIN2 = D1;

boolean m_light_state = false; // light is turned off by default

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// publica o estado da do led (on/off)
void publishLightState() {
  if (m_light_state) {
    client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_ON, true);
  } else {
    client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_OFF, true);
  }
}

// function called to turn on/off the light
void setLightState(int led) {
  if (m_light_state) {
    digitalWrite(led, HIGH);
    Serial.println("INFO: Turn light on...");
  } else {
    digitalWrite(led, LOW);
    Serial.println("INFO: Turn light off...");
  }
}

// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  // concat the payload into a string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }
  
  // handle message topic
  if (String(MQTT_LIGHT_COMMAND_TOPIC).equals(p_topic)) {
    // test if the payload is equal to "ON" or "OFF"
    if (payload.equals(String(LIGHT_ON))) {
      if (m_light_state != true) {
        m_light_state = true;
        setLightState(LED_PIN1);
        publishLightState();
      }
    } else if (payload.equals(String(LIGHT_OFF))) {
      if (m_light_state != false) {
        m_light_state = false;
        setLightState(LED_PIN1);
        publishLightState();
      }
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("INFO: Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("INFO: connected");
      // Once connected, publish an announcement...
      publishLightState();
      // ... and resubscribe
      client.subscribe(MQTT_LIGHT_COMMAND_TOPIC);
    } else {
      Serial.print("ERROR: failed, rc=");
      Serial.print(client.state());
      Serial.println("DEBUG: try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // init the serial
  Serial.begin(115200);

  // init the led
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  analogWriteRange(255);
  setLightState(LED_PIN1);

  // init the WiFi connection
  Serial.println();
  Serial.println();
  Serial.print("INFO: Connecting to ");
  WiFi.mode(WIFI_STA);
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("INFO: WiFi connected");
  Serial.print("INFO: IP address: ");
  Serial.println(WiFi.localIP());

  // init the MQTT connection
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
