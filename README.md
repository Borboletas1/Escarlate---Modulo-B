Monitoramento de Temperatura e Umidade com nRF24L01 e Arduino:
Este projeto implementa um sistema de monitoramento de ambiente sem fio utilizando módulos nRF24L01, sensores DHT11 e microcontroladores Arduino. O sistema é composto por um Nó Sensor (Transmissor) que coleta dados e um Receptor Central que exibe e analisa esses dados no PC.
1. Nó Sensor (Transmissor)
O Transmissor coleta a temperatura e umidade, controla uma esteira/servo e um alarme (buzzer) com base em um limite crítico, e envia os dados via rádio.
1.1 Funções Principais
   Coleta de Dados: Lê temperatura e umidade do sensor DHT11.
   Controle Local:Se a temperatura for maior que $30.0^{\circ}C$, a esteira (Servo Motor) é parada e o Buzzer é acionado repetidamente.Se a temperatura retornar ao normal, a esteira é retomada e o alarme é desligado.
   Transmissão: Envia os dados coletados (ID, Temperatura, Umidade) via nRF24L01 para o Receptor.
   1.2 Hardware Requerido (Transmissor):
- Arduino UNO
- nRF24L01 = Conexao: CE: D7, CSN: D8, SPI: D11, D12, D13, alimentação: 3.3V / GND (OBS: não passou para o recepctor, provavelmente problema no hardware)
- DHT11 = Conexão: D2, alimentação:5V / GND
- Servo Motor = Conexão: D5, alimentação: 5V / GND (OBS: problema no hardware);
- Buzzer = Conexão: D6, alimentação:  5V / GND.
2. Receptor Central (Arduino)
O Receptor escuta em um endereço fixo e envia os dados recebidos para o Monitor Serial no formato CSV.
2.1 Funções Principais
Escuta: Configura o nRF24L01 para o modo de escuta (radio.startListening()) no endereço "00002".
Recepção: Recebe o pacote de dados (DataPayload) do Transmissor.
Saída CSV: Imprime os dados (Timestamp, ID do Nó, Temperatura, Umidade) no Monitor Serial para processamento por um script Python.
  2.2 Hardware Requerido(Receptor):
  - Arduino UNO
  - nRF24L01: Conexao: CE: D7, CSN: D8, SPI: D11, D12, D13, alimentação: 3.3V / GND (OBS: não recebeu, provavelmente problema no hardware)
3. Script de Coleta e Registro de Dados (Datalogger Python)
é responsável por ler os dados CSV enviados pelo Receptor Central (Arduino) via porta serial e salvá-los em um arquivo.

3.1 Funções Principais
Conexão Serial: Utiliza a biblioteca pyserial para estabelecer a comunicação com a porta COM onde o Arduino Receptor está conectado.

Decodificação: Lê as linhas de dados enviadas pelo Arduino, garantindo a decodificação correta em formato de texto (utf-8).

Marcação de Tempo (Timestamping): Adiciona um timestamp preciso do computador (Timestamp_Python) a cada linha de dados, o que é mais confiável do que o millis() do Arduino.

Registro CSV: Salva cada linha de dados recebida em um arquivo CSV (dados_sensores.csv) com um cabeçalho estruturado.

Tratamento de Erros: Ignora mensagens de inicialização do Arduino e trata formatos de dados inesperados ou interrupções do usuário (Ctrl+C).

