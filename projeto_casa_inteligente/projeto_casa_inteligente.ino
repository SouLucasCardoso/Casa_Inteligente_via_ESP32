/**
 * Projeto: Casa Inteligente via ESP32
 * Autores: Lucas Cardoso Rodrigues e Bruno Sezar Marcelino Aiolfi
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <DHT.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#define LED_AR 4
#define LED_UMID 0
#define LED_LUZ 15
#define BOTAO 2

#define RGB_RED 27
#define RGB_GREEN 26
#define RGB_BLUE 25

#define DHT11_PIN 33
#define LDR 39
#define DHTTYPE DHT11

#define A 18
#define B 5
#define C 21
#define D 3
#define E 1
#define F 23
#define G 22
#define DP 19
#define DISPLAY1 16
#define DISPLAY2 17

const char* ssid = "Telecom";
const char* password = "cczd2011";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

const char* topico_pub = "unisatc/casa/sensores";
const char* topico_sub = "unisatc/casa/comandos";

DHT dht(DHT11_PIN, DHTTYPE);
WebServer server(80);
WiFiClient espClient;
PubSubClient mqtt(espClient);

EventGroupHandle_t sensorEventGroup;
#define SENSOR_READ_BIT (1 << 0)

float t = 0;
float h = 0;
int l = 0;
bool alarmeAtivo = false;

unsigned long lastBlinkTime = 0;
bool redState = false;

bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

const byte numbers[10][7] = {
  {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1}, {1,1,1,1,0,0,1}, 
  {0,1,1,0,0,1,1}, {1,0,1,1,0,1,1}, {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0}, 
  {1,1,1,1,1,1,1}, {1,1,1,1,0,1,1}  
};

void handleRoot() {
  server.send(200, "text/plain", "Casa Inteligente Ativa");
}

void handleDados() {
  String json = "{\"temperatura\":" + String(t) + ",\"umidade\":" + String(h) + ",\"luminosidade\":" + String(l) + ",\"alarme\":" + String(alarmeAtivo) + "}";
  server.send(200, "application/json", json);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  
  if (msg == "ar_on") digitalWrite(LED_AR, HIGH);
  else if (msg == "ar_off") digitalWrite(LED_AR, LOW);
  else if (msg == "umid_on") digitalWrite(LED_UMID, HIGH);
  else if (msg == "umid_off") digitalWrite(LED_UMID, LOW);
  else if (msg == "luz_on") digitalWrite(LED_LUZ, HIGH);
  else if (msg == "luz_off") digitalWrite(LED_LUZ, LOW);
  else if (msg == "alarme_on") alarmeAtivo = true; 
  else if (msg == "alarme_off") alarmeAtivo = false; 
}

void reconnect() {
  while (!mqtt.connected()) {
    String clientId = "Casa-ESP32-";
    clientId += String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str())) {
      mqtt.subscribe(topico_sub);
    } else {
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}

void taskLeituraSensores(void *pvParameters) {
  for(;;) {
    t = dht.readTemperature();
    h = dht.readHumidity();
    l = 4095 - analogRead(LDR);
    xEventGroupSetBits(sensorEventGroup, SENSOR_READ_BIT);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void taskPublicacaoMQTT(void *pvParameters) {
  for(;;) {
    xEventGroupWaitBits(sensorEventGroup, SENSOR_READ_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
    if (mqtt.connected() && !isnan(t) && !isnan(h)) {
      String payload = "{\"temperatura\":" + String(t) + ",\"umidade\":" + String(h) + ",\"luminosidade\":" + String(l) + ",\"alarme\":" + (alarmeAtivo ? String("true") : String("false")) + "}";
      mqtt.publish(topico_pub, payload.c_str());
    }
  }
}

void displayDigit(int digit, int value) {
  digitalWrite(DISPLAY1, HIGH);
  digitalWrite(DISPLAY2, HIGH);

  digitalWrite(A, numbers[value][0]);
  digitalWrite(B, numbers[value][1]);
  digitalWrite(C, numbers[value][2]);
  digitalWrite(D, numbers[value][3]);
  digitalWrite(E, numbers[value][4]);
  digitalWrite(F, numbers[value][5]);
  digitalWrite(G, numbers[value][6]);

  if (digit == 1) digitalWrite(DISPLAY1, LOW);
  if (digit == 2) digitalWrite(DISPLAY2, LOW);
}

void taskDisplay(void *pvParameters) {
  for(;;) {
    int tempInt = (int)t;
    if (tempInt < 0) tempInt = 0;
    if (tempInt > 99) tempInt = 99;

    int tens = tempInt / 10;
    int ones = tempInt % 10;

    displayDigit(1, ones);
    vTaskDelay(5 / portTICK_PERIOD_MS);
    displayDigit(2, tens);
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_AR, OUTPUT);
  pinMode(LED_UMID, OUTPUT);
  pinMode(LED_LUZ, OUTPUT);
  
  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);
  
  pinMode(BOTAO, INPUT_PULLUP); 
  
  pinMode(A, OUTPUT); pinMode(B, OUTPUT); pinMode(C, OUTPUT);
  pinMode(D, OUTPUT); pinMode(E, OUTPUT); pinMode(F, OUTPUT);
  pinMode(G, OUTPUT); pinMode(DP, OUTPUT);
  pinMode(DISPLAY1, OUTPUT); pinMode(DISPLAY2, OUTPUT);

  digitalWrite(LED_AR, LOW);
  digitalWrite(LED_UMID, LOW);
  digitalWrite(LED_LUZ, LOW);
  
  digitalWrite(RGB_RED, HIGH);   
  digitalWrite(RGB_GREEN, HIGH);  
  digitalWrite(RGB_BLUE, HIGH);  
  
  dht.begin();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.on("/", handleRoot);
  server.on("/dados", handleDados);
  server.begin();
  
  mqtt.setServer(mqtt_server, mqtt_port);
  mqtt.setCallback(callback);
  
  sensorEventGroup = xEventGroupCreate();
  
  xTaskCreatePinnedToCore(taskLeituraSensores, "Leitura", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(taskPublicacaoMQTT, "Publicacao", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskDisplay, "Display", 2048, NULL, 1, NULL, 1);
}

void loop() {
  server.handleClient();
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();

  if (alarmeAtivo) {
    digitalWrite(RGB_GREEN, HIGH); 
    digitalWrite(RGB_BLUE, HIGH);  
    if (millis() - lastBlinkTime > 300) { 
      redState = !redState;
      digitalWrite(RGB_RED, redState ? LOW : HIGH); 
      lastBlinkTime = millis();
    }
  } else {
    digitalWrite(RGB_RED, HIGH);   
    digitalWrite(RGB_GREEN, HIGH);  
    digitalWrite(RGB_BLUE, HIGH);  
  }

  bool reading = digitalRead(BOTAO);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;
      
      if (currentButtonState == LOW) { 
        alarmeAtivo = !alarmeAtivo;
        
        if (mqtt.connected()) {
          mqtt.publish(topico_sub, alarmeAtivo ? "alarme_on" : "alarme_off");
          String payload = "{\"temperatura\":" + String(t) + ",\"umidade\":" + String(h) + ",\"luminosidade\":" + String(l) + ",\"alarme\":" + (alarmeAtivo ? String("true") : String("false")) + "}";
          mqtt.publish(topico_pub, payload.c_str());
        }
      }
    }
  }
  lastButtonState = reading;
}