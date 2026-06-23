# Casa Inteligente via ESP32

## Autores
- Lucas Cardoso Rodrigues
- Bruno Sezar Marcelino Aiolfi

---

## 📋 Descrição

Sistema de casa inteligente utilizando ESP32, com sensores de temperatura (DHT11), umidade (DHT11) e luminosidade (LDR), controle de dispositivos via MQTT, display de 7 segmentos para temperatura, e dashboard Node-RED para monitoramento.

---

## 🔧 Componentes Utilizados

| Componente            | GPIO |
|-----------------------|------|
| LED Ar Condicionado   | 4    |
| LED Umidificador      | 0    |
| LED Luz               | 15   |
| Botão (alarme)        | 2    |
| RGB Vermelho          | 27   |
| RGB Verde             | 26   |
| RGB Azul              | 25   |
| DHT11                 | 33   |
| LDR                   | 39   |
| Display 7 seg (A-G)   | 18,5,21,3,1,23,22 |
| Display 7 seg (DP)    | 19   |
| Display 7 seg (COM1)  | 16   |
| Display 7 seg (COM2)  | 17   |

---

## 🚀 Configuração do ESP32

### 1. Instalar a Arduino IDE
Baixe em https://www.arduino.cc/en/software

### 2. Configurar ESP32 na Arduino IDE
- Adicione a URL de placas ESP32 em **Arquivo > Preferências > URLs Adicionais para Gerenciadores de Placas**:
  ```
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  ```
- Vá em **Ferramentas > Placa > Gerenciador de Placas**, procure por "ESP32" e instale.

### 3. Instalar Bibliotecas
- **PubSubClient** (MQTT)
- **DHT sensor library** by Adafruit
- **WebServer** (já incluso no pacote ESP32)

### 4. Configurar WiFi e MQTT
No arquivo `projeto_casa_inteligente/projeto_casa_inteligente.ino`, altere:

```cpp
const char* ssid = "sua_rede_wifi";
const char* password = "sua_senha_wifi";
const char* mqtt_server = "broker.hivemq.com"; // ou outro broker
```

### 5. Carregar o código
Conecte o ESP32 ao computador, selecione a porta correta em **Ferramentas > Porta** e clique em **Upload**.

---

## 📡 Tópicos MQTT

| Tópico                    | Direção     | Descrição                         |
|---------------------------|-------------|-----------------------------------|
| `unisatc/casa/sensores`   | Publicação  | Envia dados dos sensores (JSON)   |
| `unisatc/casa/comandos`   | Inscrição   | Recebe comandos dos dispositivos  |

### Payload dos sensores (JSON)
```json
{
  "temperatura": 25.5,
  "umidade": 60.2,
  "luminosidade": 2048,
  "alarme": false
}
```

### Comandos disponíveis
| Comando       | Ação                        |
|---------------|-----------------------------|
| `ar_on`       | Liga ar condicionado        |
| `ar_off`      | Desliga ar condicionado     |
| `umid_on`     | Liga umidificador           |
| `umid_off`    | Desliga umidificador        |
| `luz_on`      | Liga luz                    |
| `luz_off`     | Desliga luz                 |
| `alarme_on`   | Ativa alarme                |
| `alarme_off`  | Desativa alarme             |

---

## 📊 Dashboard Node-RED

### 1. Instalar Node.js
Baixe e instale o Node.js em https://nodejs.org/ (versão LTS).

### 2. Instalar Node-RED
Abra o terminal (CMD/PowerShell) e execute:

```bash
npm install -g node-red
```

### 3. Instalar pacotes adicionais
Após instalar o Node-RED, instale os pacotes necessários para o dashboard:

```bash
npm install -g node-red-dashboard
```

### 4. Iniciar o Node-RED
```bash
node-red
```

O servidor será iniciado em: `http://127.0.0.1:1880`

### 5. Importar o flow do dashboard
1. Acesse `http://127.0.0.1:1880`
2. Vá em **Menu (☰) > Import > Clipboard**
3. Cole o fluxo JSON abaixo e clique em **Import**:

```json
[
  {
    "id": "mqtt_in",
    "type": "mqtt in",
    "topic": "unisatc/casa/sensores",
    "broker": "broker.hivemq.com",
    "port": "1883",
    "name": "Sensores",
    "datatype": "json",
    "x": 200,
    "y": 200,
    "wires": [["dashboard"]]
  },
  {
    "id": "dashboard",
    "type": "ui_group",
    "name": "Casa Inteligente",
    "tab": "tab_casa",
    "order": 1
  },
  {
    "id": "tab_casa",
    "type": "ui_tab",
    "name": "Casa Inteligente",
    "icon": "home",
    "order": 1
  }
]
```

> ⚠️ Para um dashboard completo, crie nós do tipo `ui_gauge` (temperatura, umidade), `ui_chart` (luminosidade) e `ui_switch` (comandos ar, umid, luz, alarme) conectados ao tópico `unisatc/casa/comandos`.

### 6. Acessar o Dashboard
Após importar e implantar o fluxo (botão **Deploy**), acesse:

```
http://127.0.0.1:1880/ui
```

---

## 🌐 Servidor Web (ESP32)

O ESP32 também disponibiliza um servidor web local:

- **Página inicial**: `http://<ip-do-esp32>/`
- **Dados JSON**: `http://<ip-do-esp32>/dados`

Exemplo de resposta:
```json
{"temperatura":25.5,"umidade":60.2,"luminosidade":2048,"alarme":false}
```

---

## 🔄 Funcionalidades

- ✅ Leitura de temperatura e umidade (DHT11)
- ✅ Leitura de luminosidade (LDR)
- ✅ Display de 7 segmentos mostrando a temperatura
- ✅ Controle remoto via MQTT (ar, umidificador, luz, alarme)
- ✅ Botão físico para alternar alarme
- ✅ Alarme com LED RGB piscando
- ✅ Publicação periódica de dados via MQTT
- ✅ Dashboard Node-RED para monitoramento
