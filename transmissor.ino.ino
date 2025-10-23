// -------------------- Bibliotecas Necessárias --------------------
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Servo.h> 


// -------------------- DECLARAÇÕES DO SISTEMA --------------------

// 1. Definição do ID Único do Nó Sensor e Limites
#define NODE_ID 1 
// LIMITE OFICIAL: Voltando ao valor original de 30.0°C.
#define CRITICAL_TEMP 30.0 

// PINAGEM DE COMPONENTES ADICIONAIS
#define SERVO_PIN 5      // Pino Digital 5 para o Servo Motor (Esteira)
#define BUZZER_PIN 6     // Pino Digital 6 para o Buzzer (Alarme Sonoro)
const int ESTEIRA_LIGADA_ANGULO = 90; 
const int ESTEIRA_PARADA_ANGULO = 0;  


// 2. Definição da Estrutura de Dados (Payload)
struct DataPayload {
  int id;            // ID único do nó sensor (1, 2 ou 3)
  float temperatura; // Temperatura em Celsius
  float umidade;     // Umidade relativa do ar
};

// 3. Declaração da Variável de Envio e Objetos
DataPayload dataEnvio; 
Servo servoMotor; 

// Variáveis de controle para o alarme repetitivo
unsigned long buzzerStartTime = 0;
const long buzzerDuration = 3000; // Alarme toca por 3 segundos


// -------------------- CONFIGURAÇÃO DHT E NRF24L01 --------------------
#define DHTPIN 2// Pino digital 2 conectado ao DHT11 (VERIFIQUE RESISTOR PULL-UP!)
#define DHTTYPE DHT11// DHT 
DHT dht(DHTPIN, DHTTYPE);


// nRF24L01: CE=D7 e CSN=D8 
// *** VERIFIQUE DE NOVO ESSAS CONEXÕES, ALÉM DO VCC (3.3V) E O CAPACITOR! ***
RF24 radio(7, 8); 
const byte address[6] = "00002"; // Endereço de comunicação (DEVE ser igual ao Receptor)


void setup() {

  Serial.begin(9600); 
  dht.begin();

  // Configuração dos Componentes Adicionais
  servoMotor.attach(SERVO_PIN);
  servoMotor.write(ESTEIRA_LIGADA_ANGULO); // Inicia a esteira funcionando
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Inicialização do Rádio
  if (!radio.begin()) {
    Serial.println(F("ERRO CRÍTICO: Falha ao inicializar o modulo radio."));
    Serial.println(F(">>> SOLUÇÃO DE HARDWARE: Adicione capacitor 10uF/100uF entre 3.3V e GND do nRF."));
    Serial.println(F(">>> VERIFIQUE: Fiacao CE=D7, CSN=D8 e SPI (11, 12, 13)."));
    while (1); 
  }

  radio.openWritingPipe(address); 
  radio.setPALevel(RF24_PA_MIN); 
  radio.stopListening(); 
  
  Serial.print(F("Node ID: "));
  Serial.println(NODE_ID);
  Serial.print(F("Limite Critico Oficial: "));
  Serial.print(CRITICAL_TEMP);
  Serial.println(F("°C"));

}


void loop() {

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // 1. Verificação de Erro de Leitura do Sensor
  if (isnan(h) || isnan(t)) {
    Serial.println(F("ERRO: Falha ao ler do sensor DHT! Tentando novamente."));
    // Se a leitura falhar, não tenta enviar dados inválidos, apenas espera.
    delay(5000); 
    return;
  }
  
  // -------------------- LÓGICA DE CONTROLE DA ESTEIRA E ALARME --------------------
  if (t > CRITICAL_TEMP) {
    // --- TEMPERATURA CRÍTICA (T > 30.0°C) ---
    
    // a) O motor (servo) deverá parar a esteira.
    servoMotor.write(ESTEIRA_PARADA_ANGULO);
    
    // b) O buzzer será acionado REPETITIVAMENTE.
    if (buzzerStartTime == 0) {
      tone(BUZZER_PIN, 1000); // Toca um tom de 1000Hz
      buzzerStartTime = millis(); // Marca o tempo de início
      Serial.println(F(">>> ALERTA: Temperatura Critica! Esteira PARADA."));
    }

  } else {
    // --- TEMPERATURA NORMAL (T <= 30.0°C) ---
    
    // a) A esteira voltará a funcionar normalmente.
    servoMotor.write(ESTEIRA_LIGADA_ANGULO); 
    
    // b) O alarme será desligado automaticamente.
    if (buzzerStartTime != 0) {
      noTone(BUZZER_PIN); // Desliga o som 
      buzzerStartTime = 0; // Reseta o timer do alarme
      Serial.println(F(">>> SEGURANÇA RESTABELECIDA. Esteira RETOMADA."));
    }
  }

  // GERENCIAMENTO DO ALARME REPETITIVO (Desliga o som e PERMITE novo acionamento)
  if (buzzerStartTime != 0 && millis() - buzzerStartTime >= buzzerDuration) {
    noTone(BUZZER_PIN);
    buzzerStartTime = 0; 
  }
  // -------------------- FIM DA LÓGICA DE CONTROLE --------------------

  // 2. Empacotamento dos Dados
  dataEnvio.id = NODE_ID;
  dataEnvio.umidade = h;
  dataEnvio.temperatura = t;

  // 3. Envio dos Dados
  bool sucesso = radio.write(&dataEnvio, sizeof(dataEnvio)); 

  // 4. Feedback no Serial Monitor
  Serial.print(F("Node "));
  Serial.print(dataEnvio.id);
  Serial.print(F(" -> Temp: "));
  Serial.print(dataEnvio.temperatura);
  Serial.print(F("°C | Umid: "));
  Serial.print(dataEnvio.umidade);
  Serial.print(F("% | Status: "));
  
  if (sucesso) {
    Serial.println(F("ENVIADO com sucesso."));
  } else {
    Serial.println(F("FALHA no envio ou sem ACK do Receptor."));
  }

  // 5. Intervalo de Transmissão (5 segundos)
  delay(5000); 
}
