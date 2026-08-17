#include <DHT.h>

#define PINO_DHT 4
#define TIPO_DHT DHT11

#define PINO_BOTAO 2
#define LED_VERDE 18

DHT dht(PINO_DHT, TIPO_DHT);

// Tempo entre cada leitura
// 5000 ms = 5 segundos
const unsigned long INTERVALO_LEITURA = 5000;
unsigned long tempoAnterior = 0;

// Matriz com 20 linhas e 2 colunas
// Coluna 0 = Temperatura
// Coluna 1 = Umidade
const int TOTAL_LEITURAS = 20;
float matrizDados[TOTAL_LEITURAS][2];

// Controle do buffer circular
int indiceAtual = 0;
int leiturasSalvas = 0;

// Controle do botão
bool botaoJaFoiPressionado = false;

void setup() {
  Serial.begin(115200);

  dht.begin();

  pinMode(LED_VERDE, OUTPUT);
  pinMode(PINO_BOTAO, INPUT_PULLUP);

  digitalWrite(LED_VERDE, LOW);

  Serial.println("Sistema de Monitoramento com ESP32 e DHT11 iniciado.");
  Serial.println("DHT11 no GPIO 4");
  Serial.println("Botao no GPIO 2");
  Serial.println("LED verde no GPIO 18");
  Serial.println("Pressione o botao para exibir o historico.");
  Serial.println("----------------------------------------------");
}

void loop() {
  verificarBotao();

  unsigned long tempoAtual = millis();

  if (tempoAtual - tempoAnterior >= INTERVALO_LEITURA) {
    tempoAnterior = tempoAtual;
    fazerLeitura();
  }
}

void fazerLeitura() {
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Erro ao ler o sensor DHT11.");
    return;
  }

  // Salva os dados na matriz
  matrizDados[indiceAtual][0] = temperatura;
  matrizDados[indiceAtual][1] = umidade;

  Serial.print("Leitura salva na posicao ");
  Serial.print(indiceAtual);
  Serial.print(": ");
  Serial.print(temperatura, 1);
  Serial.print(" C | ");
  Serial.print(umidade, 1);
  Serial.println(" %");

  // Vai para a proxima posicao da matriz
  indiceAtual++;

  // Se chegar na posicao 20, volta para 0
  // Isso faz o buffer circular
  if (indiceAtual >= TOTAL_LEITURAS) {
    indiceAtual = 0;
  }

  // Conta quantas leituras ja foram salvas
  if (leiturasSalvas < TOTAL_LEITURAS) {
    leiturasSalvas++;
  }

  piscarLed();

  // Quando a matriz estiver cheia, o LED verde fica aceso
  if (leiturasSalvas >= TOTAL_LEITURAS) {
    digitalWrite(LED_VERDE, HIGH);
  }
}

void verificarBotao() {
  int estadoBotao = digitalRead(PINO_BOTAO);

  // Com INPUT_PULLUP:
  // Botao solto = HIGH
  // Botao apertado = LOW
  if (estadoBotao == LOW && botaoJaFoiPressionado == false) {
    delay(50); // debounce simples

    if (digitalRead(PINO_BOTAO) == LOW) {
      botaoJaFoiPressionado = true;
      mostrarHistorico();
    }
  }

  if (estadoBotao == HIGH) {
    botaoJaFoiPressionado = false;
  }
}

void mostrarHistorico() {
  Serial.println();
  Serial.println("============ HISTORICO DE MEDICOES ============");
  Serial.println("Posicao | Temperatura (C) | Umidade (%)");
  Serial.println("-----------------------------------------------");

  if (leiturasSalvas == 0) {
    Serial.println("Nenhuma leitura salva ainda.");
  } else {
    for (int i = 0; i < leiturasSalvas; i++) {
      Serial.print(i);
      Serial.print("       | ");

      Serial.print(matrizDados[i][0], 1);
      Serial.print("             | ");

      Serial.print(matrizDados[i][1], 1);
      Serial.println();
    }
  }

  Serial.println("===============================================");
  Serial.println();
}

void piscarLed() {
  // Antes de completar as 20 leituras, o LED pisca a cada leitura salva
  if (leiturasSalvas < TOTAL_LEITURAS) {
    digitalWrite(LED_VERDE, HIGH);
    delay(100);
    digitalWrite(LED_VERDE, LOW);
  } else {
    // Depois que completou as 20 leituras, ele pisca rapidinho e volta aceso
    digitalWrite(LED_VERDE, LOW);
    delay(100);
    digitalWrite(LED_VERDE, HIGH);
  }
}