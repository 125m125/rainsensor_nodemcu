#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/*
 *static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
 */

static const uint8_t RED1   = 5;
static const uint8_t RED2   = 4;
static const uint8_t RED3   = 0;
static const uint8_t BLUE   = 14;
static const uint8_t GREEN   = 12;

static const uint8_t RAINSENSOR = 13;

static const uint8_t Analog = 17;

// Wifi configuration
const char* ssid = "<wlan>";
const char* password = "<password>";

// mqtt configuration
const char* server = "192.168.178.32";
const char* raintopic = "rainsensor";
const char* lighttopic = "lightsensor";
const char* clientName = "nodemcuRain";

const int pwmIntervals = 1024;
float R;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

 void wifiConnect() {
   Serial.println();
   Serial.print("Connecting to ");
   Serial.println(ssid);

   WiFi.begin(ssid, password);

   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   Serial.println("");
   Serial.print("WiFi connected.");
   Serial.print("IP address: ");
   Serial.println(WiFi.localIP());

   if (client.connect(clientName)) {
     Serial.print("Connected to MQTT broker at ");
     Serial.print(server);
     Serial.print(" as ");
     Serial.println(clientName);
   }
   else {
     Serial.println("MQTT connect failed");
     Serial.println("Will reset and try again...");
     abort();
   }
 }

 void mqttReConnect() {
   while (!client.connected()) {
     Serial.print("Attempting MQTT connection...");
     // Attempt to connect
     if (client.connect(clientName)) {
       Serial.println("connected");
     } else {
       Serial.print("failed, rc=");
       Serial.print(client.state());
       Serial.println(" try again in 5 seconds");
       delay(5000);
     }
   }
 }

void setup() {
  R = (pwmIntervals * log10(2))/(log10(255));

  pinMode(RED1, OUTPUT);
  pinMode(RED2, OUTPUT);
  pinMode(RED3, OUTPUT);

  pinMode(RAINSENSOR, INPUT);
  // pinMode(BLUE, OUTPUT);
  // pinMode(GREEN, OUTPUT);
  Serial.begin(9600);
  client.setServer(server, 1883);
  wifiConnect();
  delay(1000);
}
String payload;
String lightPayload;
String rain;
String light;
uint8_t red = 0;
void loop() {
  int lightReading = 1023 - analogRead(Analog);
  int rainReading = digitalRead(RAINSENSOR);
  int brightness = brightness = pow (2, (lightReading / R)) - 1;
  light = (String) lightReading;
  rain = (String) rainReading;
  Serial.print(lightReading);
  Serial.print("     ");
  Serial.print(rainReading);
  Serial.print("     ");
  Serial.println(brightness);

  if (rainReading==HIGH) {
    digitalWrite(RED1, LOW);
    digitalWrite(RED2, LOW);
    digitalWrite(RED3, LOW);
    analogWrite(GREEN, brightness);
    payload = "{ok: true, value: "+ rain +"}";
    delay(4000);
  } else {
    analogWrite(GREEN, 0);
    payload = "{ok: false, value: "+ rain +"}";
    switch(red) {
      case 0:
        digitalWrite(RED1, HIGH);
        digitalWrite(RED3, HIGH);
        digitalWrite(RED2, LOW);
        red = 1;
        break;
      case 1:
        digitalWrite(RED2, HIGH);
        digitalWrite(RED1, LOW);
        digitalWrite(RED3, LOW);
        red = 0;
        break;
      /* case 2:
        digitalWrite(RED3, HIGH);
        digitalWrite(RED2, LOW);
        red = 0;
        break; */
    }
  }
  analogWrite(BLUE, brightness);
  delay(50);
  analogWrite(BLUE, 0);
  delay(950);
  if (client.connected()) {
    lightPayload = "{value: "+ light +"}";
    if (client.publish(raintopic, (char*) payload.c_str())) {
      Serial.print("Rain publish ok (");
      Serial.print(payload);
      Serial.println(")");
      if (client.publish(lighttopic, (char*) lightPayload.c_str())) {
        Serial.print("Light publish ok (");
        Serial.print(lightPayload);
        Serial.println(")");

      } else {
        Serial.println("Light publish failed");
      }
    } else {
      Serial.println("Rain publish failed");
    }
  } else {
    mqttReConnect();
  }
  /*digitalWrite(RED1, HIGH);
  delay(1000);
  digitalWrite(RED2, HIGH);
  delay(1000);
  digitalWrite(RED3, HIGH);
  delay(1000);
  digitalWrite(BLUE, HIGH);
  delay(1000);
  digitalWrite(GREEN, HIGH);
  delay(1000);


  digitalWrite(RED1, LOW);
  delay(1000);
  digitalWrite(RED2, LOW);
  delay(1000);
  digitalWrite(RED3, LOW);
  delay(1000);
  digitalWrite(BLUE, LOW);
  delay(1000);
  digitalWrite(GREEN, LOW);
  delay(1000);*/
}
