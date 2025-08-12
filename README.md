# ESP32 ThingsBoard Temperature Monitor

Este projeto conecta um ESP32 ao ThingsBoard para monitoramento de temperatura e umidade.

## 🚀 Configuração Inicial

### 1. Configurar ThingsBoard

1. Acesse o ThingsBoard (https://thingsboard.cloud ou seu servidor)
2. Faça login na sua conta
3. Vá para **Devices** > **Add Device**
4. Crie um novo dispositivo (ex: "ESP32-Temperatura")
5. Copie o **Access Token** do dispositivo

### 2. Configurar WiFi e ThingsBoard

Edite o arquivo `include/config.h` e configure:

```cpp
#define WIFI_SSID "Seu_WiFi_Nome"
#define WIFI_PASSWORD "Sua_Senha_WiFi"
#define DEVICE_TOKEN "Seu_Token_ThingsBoard"
```

### 3. Compilar e Enviar

```bash
pio run -t upload
pio device monitor
```

## 📊 Dados Enviados

O dispositivo envia os seguintes dados para o ThingsBoard:

### Telemetria (a cada 5 segundos):
- `temperature`: Temperatura em °C
- `humidity`: Umidade relativa em %

### Atributos:
- `deviceModel`: Modelo do dispositivo
- `location`: Localização do dispositivo

## 🔧 Personalização

### Alterar Intervalo de Envio
No arquivo `config.h`, modifique:
```cpp
#define TELEMETRY_INTERVAL 10000  // 10 segundos
```

### Usar Sensor Real
Para usar um sensor DS18B20 ou DHT22, substitua a função `readTemperature()` no `main.cpp`:

#### Para DS18B20:
```cpp
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);

float readTemperature() {
    sensors.requestTemperatures();
    return sensors.getTempCByIndex(0);
}
```

#### Para DHT22:
```cpp
#include <DHT.h>

DHT dht(TEMP_SENSOR_PIN, DHT22);

float readTemperature() {
    return dht.readTemperature();
}
```

## 🎯 Dashboard ThingsBoard

No ThingsBoard, crie um dashboard com:

1. **Widget de Gráfico**: Para mostrar histórico de temperatura
2. **Widget de Gauge**: Para temperatura atual
3. **Widget de Cartão**: Para mostrar última leitura

### Exemplo de configuração de widget:
- **Datasource**: Seu dispositivo
- **Keys**: temperature, humidity
- **Timewindow**: Last 1 hour

## 🔍 Monitoramento

### Serial Monitor
O dispositivo imprime informações úteis no monitor serial:
- Status da conexão WiFi
- Status da conexão ThingsBoard  
- Valores de temperatura e umidade
- Confirmação de envio de dados

### Logs Importantes
```
WiFi conectado!
Conectando ao ThingsBoard... Conectado!
Temperatura: 25.3°C
Umidade: 68%
Dados enviados para o ThingsBoard!
```

## ⚠️ Solução de Problemas

### WiFi não conecta
- Verifique SSID e senha
- Certifique-se que o ESP32 está no alcance do WiFi

### ThingsBoard não conecta
- Verifique o token do dispositivo
- Confirme o endereço do servidor ThingsBoard
- Verifique a conexão com a internet

### Dados não aparecem no ThingsBoard
- Verifique se o dispositivo está "Ativo" no ThingsBoard
- Confirme que a telemetria está sendo enviada (monitor serial)
- Verifique as configurações do dashboard

## 📋 Dependências

As seguintes bibliotecas são instaladas automaticamente:
- ThingsBoard (v0.12.0)
- ArduinoJson (v7.0.4)
- WiFi (v1.2.7)

## 🔗 Links Úteis

- [ThingsBoard Documentation](https://thingsboard.io/docs/)
- [PlatformIO Docs](https://docs.platformio.org/)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
