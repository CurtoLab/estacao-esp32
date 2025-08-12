// ESP32 / ESP32-S3 + ThingsBoard 0.15.0
// Server-side RPC com API Server_Side_RPC: setState / getState
// LED controlado pelo dashboard (Switch)

#include <Arduino.h>
#include <WiFi.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>
#include <Server_Side_RPC.h>

// ======== Configurações ========
constexpr char WIFI_SSID[] = "CURTOCIRCUITO";
constexpr char WIFI_PASSWORD[] = "Curto@1020";
constexpr char TOKEN[] = "cJDdWN8GNKSHOAVGLG1K";
constexpr char THINGSBOARD_SERVER[] = "demo.thingsboard.io";
constexpr uint16_t THINGSBOARD_PORT = 1883;

// Buffers MQTT (aumente se precisar de payloads maiores)
constexpr uint16_t MAX_MESSAGE_SEND_SIZE = 256;
constexpr uint16_t MAX_MESSAGE_RECEIVE_SIZE = 256;

// LED onboard: em muitos ESP32 é GPIO 2.
// Em ESP32-S3, às vezes é 48. Se o LED não acender, troque aqui.
constexpr int LED_PIN = 2;

// ======== Estado ========
bool deviceState = false;

// ======== Stack ThingsBoard ========
WiFiClient net;
Arduino_MQTT_Client mqttClient(net);

// API Server-side RPC (igual ao exemplo)
constexpr uint8_t MAX_RPC_SUBSCRIPTIONS = 3U;
constexpr uint8_t MAX_RPC_RESPONSE = 6U;
Server_Side_RPC<MAX_RPC_SUBSCRIPTIONS, MAX_RPC_RESPONSE> rpc;
IAPI_Implementation *apis[] = {&rpc};

// Instância ThingsBoard com buffers e APIs
ThingsBoard tb(mqttClient,
               MAX_MESSAGE_RECEIVE_SIZE,
               MAX_MESSAGE_SEND_SIZE,
               Default_Max_Stack_Size,
               apis + 0U, apis + 1U);

bool subscribed = false;

// ======== Wi-Fi ========
void InitWiFi()
{
  Serial.print("Conectando ao WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

bool reconnect()
{
  if (WiFi.status() == WL_CONNECTED)
    return true;
  InitWiFi();
  return true;
}

// ======== RPC handlers ========
// setState: params pode vir {"enabled":true} ou bool direto
void processSetState(const JsonVariantConst &data, JsonDocument &response)
{
  Serial.println("RPC setState recebido");

  bool newState = false;
  if (data.is<bool>())
  {
    newState = data.as<bool>();
  }
  else if (data.containsKey("enabled"))
  {
    newState = data["enabled"].as<bool>();
  }

  deviceState = newState;
  digitalWrite(LED_PIN, deviceState ? HIGH : LOW);

  response["state"] = deviceState;
  response["enabled"] = deviceState;
}

// getState: retorna estado atual
void processGetState(const JsonVariantConst &data, JsonDocument &response)
{
  Serial.println("RPC getState recebido");
  response["state"] = deviceState;
  response["enabled"] = deviceState;
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  InitWiFi();
}

void loop()
{
  // Mantenha Wi-Fi
  if (!reconnect())
    return;

  // Conecta TB se necessário
  if (!tb.connected())
  {
    Serial.printf("Conectando ao ThingsBoard (%s) com token...\n", THINGSBOARD_SERVER);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT))
    {
      Serial.println("Falha ao conectar no TB, tentando depois...");
      delay(2000);
      return;
    }
    Serial.println("Conectado ao ThingsBoard!");
  }

  // Assina RPCs uma única vez
  if (!subscribed)
  {
    Serial.println("Assinando RPCs...");
    const RPC_Callback callbacks[MAX_RPC_SUBSCRIPTIONS] = {
        {"setState", processSetState},
        {"getState", processGetState}
        // você pode adicionar mais aqui
    };
    if (!rpc.RPC_Subscribe(callbacks + 0U, callbacks + 2U))
    {
      Serial.println("Falha ao assinar RPCs");
      delay(1000);
      return;
    }
    Serial.println("RPCs assinados");
    subscribed = true;
  }

  // Processa MQTT/RPC
  tb.loop();

  // Telemetria de exemplo (2 casas) a cada 5s
  static uint32_t lastSend = 0;
  if (millis() - lastSend >= 5000)
  {
    lastSend = millis();

    float temp = random(200, 310) / 10.0f;           // 20.0–30.9
    float hum = 60.0f + (random(-200, 200) / 10.0f); // ~40.0–79.9

    // Criar StaticJsonDocument para telemetria
    StaticJsonDocument<200> telemetryDoc;
    telemetryDoc["temperature"] = temp;
    telemetryDoc["humidity"] = hum;
    telemetryDoc["state"] = deviceState; // Incluir estado do LED na telemetria

    // Enviar telemetria com JsonDocument
    tb.sendTelemetryJson(telemetryDoc, measureJson(telemetryDoc));

    Serial.printf("TX -> temp=%.2f, hum=%.2f, led=%s\n",
                  temp, hum, deviceState ? "ON" : "OFF");
  }
}
