#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN D3    // Pin D3 works best (D4 does not reset properly)
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

const char* ssid = "Skynet";
const char* password = "clydethedog";
const char* mqtt_server = "192.168.0.108";

const char* temp_topic = "office/temp";
const char* humidity_topic = "office/humidity";

const unsigned long sensor_rate = 60000;

const boolean led_enabled = true;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 9999999;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  Serial.begin(115200);
  delay(500);
  Serial.println("Begin!");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  unsigned long now = millis();
  if (now - lastMsg > sensor_rate) {
    lastMsg = now;

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    if (led_enabled) {
      for (int i = 0; i <= 9; i++) {
        digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
        delay(30);
        digitalWrite(LED_BUILTIN, LOW);  // Turn the LED on by making the voltage LOW
        delay(30);
      }
    }
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH

    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.print(f);
    Serial.print(F("°F  Heat index: "));
    Serial.print(hic);
    Serial.print(F("°C "));
    Serial.print(hif);
    Serial.println(F("°F"));

    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    //++value;
    //snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    //Serial.print("Publish message: ");
    //Serial.println(msg);
    //client.publish("outTopic", msg);
    snprintf (msg, MSG_BUFFER_SIZE, "%f", f);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(temp_topic, msg);
    snprintf (msg, MSG_BUFFER_SIZE, "%f", h);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(humidity_topic, msg);
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}
