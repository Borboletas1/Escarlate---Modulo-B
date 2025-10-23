// -------------------- Bibliotecas Necessárias --------------------
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// -------------------- Configuração do nRF24L01 --------------------
// CE e CSN DEVEM corresponder ao Transmissor (D7 e D8)
RF24 radio(7, 8); 
// O endereço DEVE ser o mesmo usado como 'writing pipe' no Transmissor
const byte address[6] = "00002"; 

// -------------------- Estrutura de Dados (Payload) --------------------
// A struct DEVE ser IDÊNTICA à do código do Transmissor!
struct DataPayload {
  int id;            // ID único do nó sensor (1, 2 ou 3)
  float temperatura; // Temperatura em Celsius
  float umidade;     // Umidade relativa do ar
};

DataPayload receivedData; // Objeto para armazenar os dados recebidos

// -------------------- Setup do Receptor --------------------
void setup() {
  Serial.begin(9600);

  // Inicializa o nRF24L01
  if (!radio.begin()) {
    // Se o erro de inicialização persistir, verifique a fiação SPI e o 3.3V
    Serial.println(F("ERRO: Falha ao inicializar o modulo radio. (Checar 3.3V, 11, 12, 13, SPI)"));
    while (1); 
  }
  
  // Configurações do Rádio
  // Abre o pipe (canal) de leitura no endereço 'address'
  radio.openReadingPipe(1, address); 
  
  // Nível de potência mínimo (mesmo do Transmissor)
  radio.setPALevel(RF24_PA_MIN);     
  
  // Importante: Configura o módulo para ESCUTAR (Receptor)
  radio.startListening();          
  
  Serial.println(F("=== Estacao Receptora Central Iniciada ==="));
  
  // Imprime o cabeçalho CSV estruturado
  Serial.println(F("Timestamp,Node_ID,Temperatura_C,Umidade_Porc"));
}

// -------------------- Loop do Receptor --------------------
void loop() {
  // 1. Verifica se há dados disponíveis
  if (radio.available()) {
    
    // 2. Lê e armazena os dados na struct 'receivedData'
    radio.read(&receivedData, sizeof(receivedData));

    // 3. Obtém o tempo atual (para o carimbo de data/hora)
    unsigned long now = millis();
    
    // 4. Exibe os dados no formato CSV (Comma Separated Values)
    Serial.print(now);
    Serial.print(F(","));
    Serial.print(receivedData.id);
    Serial.print(F(","));
    Serial.print(receivedData.temperatura, 2); // 2 casas decimais
    Serial.print(F(","));
    Serial.println(receivedData.umidade, 2);   // 2 casas decimais
    
    // Opcional: Pausa breve antes de verificar novamente
    delay(5);
  }
}
