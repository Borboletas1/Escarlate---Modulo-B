import serial
import csv
import time
from datetime import datetime

# --- CONFIGURAÇÕES DE CONEXÃO E ARQUIVO ---
# 1. Ajuste a porta serial. (COM3, COM4, /dev/ttyACM0, etc.)
#    Verifique qual porta o seu Arduino Receptor está usando!
PORTA_SERIAL = 'COM4' 
# 2. A taxa de baud (DEVE ser a mesma do Arduino: 9600)
BAUD_RATE = 9600 
# 3. Nome do arquivo de saída
NOME_ARQUIVO = 'dados_sensores.csv' 
# 4. Cabecalho do arquivo CSV (DEVE ser igual ao que o Arduino imprime!)
HEADER = ["Timestamp_Python", "Node_ID", "Temperatura_C", "Umidade_Porc"]

# -------------------------------------------

def inicializar_serial():
    """Tenta inicializar a conexão serial com o Arduino."""
    try:
        ser = serial.Serial(PORTA_SERIAL, BAUD_RATE, timeout=1)
        # Espera um pouco para garantir que a conexão serial foi estabelecida
        time.sleep(2) 
        print(f"Conexão estabelecida com sucesso na porta {PORTA_SERIAL}.")
        return ser
    except serial.SerialException as e:
        print(f"ERRO: Não foi possível conectar à porta serial {PORTA_SERIAL}.")
        print("Verifique se o Arduino está conectado e se a porta e o baud rate estão corretos.")
        print(f"Detalhes do erro: {e}")
        return None

def main():
    # Inicializa a conexão serial
    serial_conn = inicializar_serial()
    if not serial_conn:
        return

    # Abre o arquivo CSV no modo append (a)
    with open(NOME_ARQUIVO, mode='a', newline='') as file:
        writer = csv.writer(file)
        
        # Se o arquivo estiver vazio, escreve o cabeçalho
        if file.tell() == 0:
            writer.writerow(["Timestamp_Python"] + HEADER) 
        
        print(f"Coletando dados e salvando em {NOME_ARQUIVO}. Pressione Ctrl+C para sair.")

        while True:
            try:
                # Lê uma linha da serial
                line = serial_conn.readline().decode('utf-8').strip()
                
                # O Arduino imprime o cabeçalho e outras mensagens na inicialização.
                # Ignoramos qualquer linha que não comece com um número (Timestamp do Arduino).
                if not line or not line[0].isdigit():
                    if line:
                        print(f"[Arduino Info]: {line}")
                    continue
                
                # Processamento da linha CSV:
                # O formato esperado do Arduino é: [Timestamp_Arduino],[Node_ID],[Temperatura_C],[Umidade_Porc]
                data_list = line.split(',')

                # O Arduino envia 4 campos. Se o rádio falhar na leitura, pode ter menos.
                if len(data_list) == 4:
                    
                    # 1. Captura o timestamp do Python (para precisão do sistema)
                    python_timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                    
                    # 2. Os dados brutos do Arduino (já no formato CSV)
                    node_id = data_list[1]
                    temperatura = data_list[2]
                    umidade = data_list[3]
                    
                    # Linha completa a ser salva no arquivo CSV
                    row = [python_timestamp, data_list[0], node_id, temperatura, umidade]

                    # Escreve a linha no arquivo e exibe no console
                    writer.writerow(row)
                    print(f"Dados salvos: {row}")

                else:
                    # Trata linhas com formato inválido (ex: rádio falhando)
                    print(f"[ATENÇÃO] Formato de dados inesperado: {line}")
                
            except KeyboardInterrupt:
                print("\nInterrompido pelo usuário. Fechando conexão...")
                break
            except Exception as e:
                print(f"Erro durante a leitura/processamento: {e}")
                time.sleep(1) # Pequena pausa para evitar loops de erro

    # Fecha a conexão serial ao sair do loop
    serial_conn.close()

if __name__ == "__main__":
    main()