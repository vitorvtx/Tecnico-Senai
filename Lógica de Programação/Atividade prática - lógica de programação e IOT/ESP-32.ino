#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>

#define LED_VERDE 26
#define LED_VERMELHO 27

#define DHTPIN 4

#if SIMULADOR_WOKWI
  #define DHTTYPE DHT22
#else
  #define DHTTYPE DHT11
#endif

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
WebServer server(80);

bool verdeLigado = false;
bool vermelhoLigado = false;

const char* ap_ssid = "ESP32-Sensor";
const char* ap_password = "12345678";

void pagina() {
  float tempDHT = dht.readTemperature();
  float umidade = dht.readHumidity();

  float tempBMP = bmp.readTemperature();
  float pressao = bmp.readPressure() / 100.0;
  float altitude = bmp.readAltitude();

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<title>ESP32</title>";
  html += "<style>";
  html += "body{font-family:Arial;background:#111;color:white;text-align:center;padding:20px;}";
  html += ".card{background:#222;border-radius:15px;padding:15px;margin:15px auto;max-width:400px;}";
  html += "a{display:inline-block;padding:10px 15px;margin:5px;border-radius:8px;color:white;text-decoration:none;background:#444;}";
  html += ".on{background:green}.off{background:#b02020}";
  html += "</style></head><body>";

  html += "<h1>ESP32 Sensores</h1>";

  html += "<div class='card'>";
  html += "<h2>DHT</h2>";
  html += "<p>Temperatura: " + String(tempDHT) + " °C</p>";
  html += "<p>Umidade: " + String(umidade) + " %</p>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<h2>BMP180</h2>";
  html += "<p>Temperatura: " + String(tempBMP) + " °C</p>";
  html += "<p>Pressão: " + String(pressao) + " hPa</p>";
  html += "<p>Altitude: " + String(altitude) + " m</p>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<h2>LEDs</h2>";
  html += "<p>Verde: " + String(verdeLigado ? "Ligado" : "Desligado") + "</p>";
  html += "<a class='on' href='/verde/on'>Ligar verde</a>";
  html += "<a class='off' href='/verde/off'>Desligar verde</a>";

  html += "<p>Vermelho: " + String(vermelhoLigado ? "Ligado" : "Desligado") + "</p>";
  html += "<a class='on' href='/vermelho/on'>Ligar vermelho</a>";
  html += "<a class='off' href='/vermelho/off'>Desligar vermelho</a>";
  html += "</div>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  dht.begin();

  Wire.begin(21, 22);

  if (!bmp.begin()) {
    Serial.println("BMP180 não encontrado.");
  }

#if SIMULADOR_WOKWI
  WiFi.begin("Wokwi-GUEST", "", 6);
  Serial.print("Conectando no Wi-Fi do Wokwi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Conectado!");
  Serial.print("IP no Wokwi: ");
  Serial.println(WiFi.localIP());
#else
  WiFi.softAP(ap_ssid, ap_password);

  Serial.println("Access Point criado!");
  Serial.print("Nome da rede: ");
  Serial.println(ap_ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
#endif

  server.on("/", pagina);

  server.on("/verde/on", []() {
    verdeLigado = true;
    digitalWrite(LED_VERDE, HIGH);
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/verde/off", []() {
    verdeLigado = false;
    digitalWrite(LED_VERDE, LOW);
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/vermelho/on", []() {
    vermelhoLigado = true;
    digitalWrite(LED_VERMELHO, HIGH);
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/vermelho/off", []() {
    vermelhoLigado = false;
    digitalWrite(LED_VERMELHO, LOW);
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.begin();
}

void loop() {
  server.handleClient();
}