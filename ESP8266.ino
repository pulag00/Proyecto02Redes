//coneccion
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//leds, pulsadores y buzzer
#define LED1 13
#define LED2 12
#define LED3 14
#define PULSADOR1 0
#define PULSADOR2 2
#define BUZZER 15

//pantalla
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32  // 0.91" tiene 32 de alto
#define OLED_RESET     -1  // No se usa un pin de reset físico
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi settings
const char *ssid = "richired1234";             // Replace with your WiFi name
const char *password = "hola1234";   // Replace with your WiFi password

//valor de moneda
bool bitcoin = 0;
double valor = 0;
double valorAnterior = 0;
double valorSuperar = 9999999999;
char* topic1 ;
byte* payload1 ;
unsigned int length1;
bool MensajeRecibido = false;

// configuracion broker mqtt
const char *mqtt_broker = "broker.emqx.io";  // EMQX broker endpoint
const char *mqtt_topic = "crypto/bitcoin";
const char *mqtt_topic2 = "crypto/solana";
const char *mqtt_topic3 = "crypto/config";
const char *mqtt_username = "emqx";  // MQTT username for authentication
const char *mqtt_password = "public";  // MQTT password for authentication
const int mqtt_port = 1883;  // MQTT port (TCP)
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

//funciones
void connectToWiFi();
void connectToMQTTBroker();
void mqttCallback(char *topic, byte *payload, unsigned int length);

//setup----------------------------------------
void setup() {

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);

  pinMode(PULSADOR1, INPUT_PULLUP);
  pinMode(PULSADOR2, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  //setup red
  Serial.begin(115200);
  connectToWiFi();
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setCallback(mqttCallback);
  connectToMQTTBroker();

  //setup pantalla
  Wire.begin(4, 5); // SDA = GPIO4 (D2), SCL = GPIO5 (D1)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Dirección I2C común
    Serial.println(F("No se detectó la pantalla OLED"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("display funcionando");
  display.display();

}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to the WiFi network");
}

void connectToMQTTBroker() {
  while (!mqtt_client.connected()) {
    String client_id = "esp8266-client-" + String(WiFi.macAddress());
    Serial.printf("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
    if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
      mqtt_client.subscribe(mqtt_topic);
      mqtt_client.subscribe(mqtt_topic2);
      mqtt_client.subscribe(mqtt_topic3);

    } else {
      Serial.print("Failed to connect to MQTT broker");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

//callback-------------------------
void mqttCallback(char *topic, byte *payload, unsigned int length) {

  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
  MensajeRecibido = true;
  if ((bitcoin && (strcmp(topic, mqtt_topic) == 0)) || (!bitcoin && (strcmp(topic, mqtt_topic2) == 0))) {

    valorAnterior = valor;
    //pasa el mensaje de byte* a double
    char buffer[64];
    if (length >= sizeof(buffer)) length = sizeof(buffer) - 1;
    memcpy(buffer, payload, length);
    buffer[length] = '\0'; // Agrega el terminador nulo
    valor = atof(buffer);

    //muestra mensaje en display
    display.clearDisplay();
    display.setCursor(0, 0);
    if (bitcoin) {
      display.println("Bitcoin");
    }
    else {
      display.println("Solana");
    }
    display.print(valor);
    display.display();
  }
  if (strcmp(topic, mqtt_topic3) == 0) {
    char buffer[64];
    if (length >= sizeof(buffer)) length = sizeof(buffer) - 1;
    memcpy(buffer, payload, length);
    buffer[length] = '\0'; // Agrega el terminador nulo
    valorSuperar = atof(buffer);
  }
}

//loop------------------------
void loop() {
  bool btn1;
  bool btn2;
  //verifica coneccion a mqtt
  if (!mqtt_client.connected()) {
    connectToMQTTBroker();
  }
  //coge mensaje de mqtt
  mqtt_client.loop();
  btn1 = digitalRead(PULSADOR1);
  btn2 = digitalRead(PULSADOR2);
  if (!btn1) {
    bitcoin = true;
    valorSuperar = 9999999999;
  }
  if (!btn2) {
    bitcoin = false;
    valorSuperar = 9999999999;
  }
  if (valor > valorAnterior) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
  }
  if (valor < valorAnterior) {
    digitalWrite(LED2, HIGH);
    digitalWrite(LED1, LOW);
    digitalWrite(LED3, LOW);
  }
  if (valor == valorAnterior) {
    digitalWrite(LED3, HIGH);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
  }
  if (valorSuperar < valor) {
    digitalWrite(BUZZER, HIGH);
  }
  else {
    digitalWrite(BUZZER, LOW);
  }
  delay(0);
}
