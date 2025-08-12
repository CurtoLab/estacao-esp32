// ESP32 ThingsBoard - Controle de LED via Dashboard
// Versão simplificada com RPC setState/getState

#include <Arduino.h>
#include <WiFi.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>
#include <Server_Side_RPC.h>

// ===== CONFIGURAÇÕES =====
const char *WIFI_SSID = "CURTOCIRCUITO";
const char *WIFI_PASSWORD = "Curto@1020";
const char *TB_TOKEN = "cJDdWN8GNKSHOAVGLG1K";
const char *TB_SERVER = "demo.thingsboard.io";
const int TB_PORT = 1883;
const int LED_PIN = 2;

// ===== VARIÁVEIS GLOBAIS =====
bool ledState = false;
unsigned long lastTelemetry = 0;
const unsigned long TELEMETRY_INTERVAL = 5000; // 5 segundos

// ===== INICIALIZAÇÃO THINGSBOARD =====
WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
Server_Side_RPC<2, 8> rpc;
IAPI_Implementation *apis[] = {&rpc};
ThingsBoard tb(mqttClient, 256, 256, Default_Max_Stack_Size, apis + 0U, apis + 1U);

// ===== FUNÇÕES =====

// Conectar WiFi
void conectarWiFi()
{
  Serial.print("Conectando WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado!");
}

// Conectar ThingsBoard
bool conectarThingsBoard()
{
  if (tb.connected())
    return true;

  Serial.println("🔗 Conectando ThingsBoard...");
  if (tb.connect(TB_SERVER, TB_TOKEN, TB_PORT))
  {
    Serial.println("✅ ThingsBoard conectado!");
    return true;
  }

  Serial.println("❌ Falha conexão ThingsBoard");
  return false;
}

// RPC: Controlar LED
void onSetState(const JsonVariantConst &data, JsonDocument &response)
{
  Serial.println("📞 RPC setState recebido");

  // Aceita tanto boolean direto quanto {"enabled": true}
  bool novoEstado = data.is<bool>() ? data.as<bool>() : data["enabled"];

  ledState = novoEstado;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);

  response["state"] = ledState;
  response["success"] = true;

  Serial.printf("💡 LED %s\n", ledState ? "LIGADO" : "DESLIGADO");
}

// RPC: Obter estado do LED
void onGetState(const JsonVariantConst &data, JsonDocument &response)
{
  Serial.println("📞 RPC getState recebido");
  response["state"] = ledState;
  response["enabled"] = ledState;
}

// Enviar telemetria
void enviarTelemetria()
{
  if (millis() - lastTelemetry < TELEMETRY_INTERVAL)
    return;
  lastTelemetry = millis();

  // Simular sensores
  float temperatura = random(200, 300) / 10.0f; // 20.0-29.9°C
  float umidade = random(400, 800) / 10.0f;     // 40.0-79.9%

  // Enviar cada telemetria separadamente usando sendTelemetryData
  tb.sendTelemetryData("temperature", temperatura);
  tb.sendTelemetryData("humidity", umidade);
  tb.sendTelemetryData("getState", ledState);

  Serial.printf("📊 Temp: %.1f°C | Umidade: %.1f%% | LED: %s\n",
                temperatura, umidade, ledState ? "ON" : "OFF");
}

// Configurar RPCs
bool configurarRPCs()
{
  static bool rpcConfigurado = false;
  if (rpcConfigurado)
    return true;

  Serial.println("🔧 Configurando RPCs...");

  const RPC_Callback callbacks[] = {
      {"setState", onSetState},
      {"getState", onGetState}};

  if (rpc.RPC_Subscribe(callbacks + 0U, callbacks + 2U))
  {
    Serial.println("✅ RPCs configurados: setState, getState");
    rpcConfigurado = true;
    return true;
  }

  Serial.println("❌ Falha ao configurar RPCs");
  return false;
}

// ===== SETUP =====
void setup()
{
  Serial.begin(115200);
  Serial.println("🚀 ESP32 ThingsBoard iniciando...");

  // Configurar LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Conectar WiFi
  conectarWiFi();

  Serial.println("✅ Setup concluído!");
}

// ===== LOOP PRINCIPAL =====
void loop()
{
  // Manter conexões ativas
  if (WiFi.status() != WL_CONNECTED)
  {
    conectarWiFi();
    return;
  }

  if (!conectarThingsBoard())
  {
    delay(2000);
    return;
  }

  // Configurar RPCs
  if (!configurarRPCs())
  {
    delay(1000);
    return;
  }

  // Processar mensagens MQTT/RPC
  tb.loop();

  // Enviar telemetria
  enviarTelemetria();

  delay(100); // Pequena pausa para não sobrecarregar
}
