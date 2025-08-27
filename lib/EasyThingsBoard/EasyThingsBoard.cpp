#include "EasyThingsBoard.h"

// Instância global para callbacks (como no main original)
EasyThingsBoard *EasyThingsBoard::instance = nullptr;

EasyThingsBoard::EasyThingsBoard() : wifiClient(nullptr), mqttClient(nullptr), rpc(nullptr), apis(nullptr), tb(nullptr),
                                     configured(false), rpcConfigured(false), ledPin(-1), ledState(false),
                                     lastTelemetry(0), telemetryInterval(5000), autoTelemetryEnabled(false), callbackCount(0)
{

    // Configurar instância global
    instance = this;

    // Inicializar objetos EXATAMENTE como no main
    wifiClient = new WiFiClient();
    mqttClient = new Arduino_MQTT_Client(*wifiClient);
    rpc = new Server_Side_RPC<8, 16>();

    // Array de APIs
    apis = new IAPI_Implementation *[2];
    apis[0] = rpc;
    apis[1] = nullptr;

    // ThingsBoard com mesmos parâmetros do main
    tb = new ThingsBoard(*mqttClient, 256, 256, Default_Max_Stack_Size, apis + 0U, apis + 1U);
}

EasyThingsBoard::~EasyThingsBoard()
{
    delete tb;
    delete rpc;
    delete[] apis;
    delete mqttClient;
    delete wifiClient;
    instance = nullptr;
}

bool EasyThingsBoard::connect(const char *ssid, const char *password, const char *token,
                              const char *server, int port)
{
    // Salvar configurações
    wifiSSID = ssid;
    wifiPassword = password;
    tbToken = token;
    tbServer = server;
    tbPort = port;

    Serial.println("🚀 EasyThingsBoard iniciando...");
    Serial.printf("WiFi: %s\n", ssid);
    Serial.printf("ThingsBoard: %s:%d\n", server, port);

    // Conectar WiFi (método igual ao main)
    conectarWiFi();

    configured = true;
    Serial.println("✅ EasyThingsBoard configurado!");
    return true;
}

void EasyThingsBoard::conectarWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
        return;

    Serial.print("Conectando WiFi");
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ WiFi conectado!");
}

bool EasyThingsBoard::conectarThingsBoard()
{
    if (tb->connected())
        return true;

    Serial.println("🔗 Conectando ThingsBoard...");
    if (tb->connect(tbServer.c_str(), tbToken.c_str(), tbPort))
    {
        Serial.println("✅ ThingsBoard conectado!");
        return true;
    }

    Serial.println("❌ Falha conexão ThingsBoard");
    return false;
}

void EasyThingsBoard::setupLED(int pin)
{
    ledPin = pin;
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    ledState = false;

    Serial.printf("💡 LED configurado no pino %d\n", pin);

    // Adicionar callbacks automáticos do LED (igual ao main)
    addRPC("setState", onSetState);
    addRPC("getState", onGetState);
}

bool EasyThingsBoard::addRPC(const char *method, void (*callback)(const JsonVariantConst &data, JsonDocument &response))
{
    if (callbackCount >= MAX_CALLBACKS)
    {
        Serial.println("❌ Máximo de callbacks atingido!");
        return false;
    }

    // Adicionar ao array de callbacks
    registeredCallbacks[callbackCount] = {method, callback};
    callbackCount++;

    Serial.printf("✅ RPC '%s' registrado\n", method);

    // Marcar para reconfigurar RPCs
    rpcConfigured = false;

    return true;
}

bool EasyThingsBoard::registerRPCs(const RPC_Callback *callbacks, uint8_t count)
{
    Serial.printf("📋 Registrando %d callbacks...\n", count);

    for (uint8_t i = 0; i < count && i < MAX_CALLBACKS; i++)
    {
        if (callbackCount >= MAX_CALLBACKS)
            break;

        registeredCallbacks[callbackCount] = callbacks[i];
        callbackCount++;
        Serial.printf("✅ RPC '%s' registrado\n", callbacks[i].Get_Name());
    }

    // Marcar para reconfigurar RPCs
    rpcConfigured = false;

    return true;
}

bool EasyThingsBoard::configurarRPCs()
{
    if (rpcConfigured || callbackCount == 0)
        return rpcConfigured;

    Serial.printf("🔧 Configurando %d RPCs...\n", callbackCount);

    // Usar EXATAMENTE o mesmo método do main
    if (rpc->RPC_Subscribe(registeredCallbacks + 0U, registeredCallbacks + callbackCount))
    {
        Serial.println("✅ RPCs configurados!");
        rpcConfigured = true;
        return true;
    }

    Serial.println("❌ Falha ao configurar RPCs");
    return false;
}

void EasyThingsBoard::loop()
{
    if (!configured)
        return;

    // Manter conexões ativas (igual ao main)
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

    // Configurar RPCs (igual ao main)
    if (!configurarRPCs())
    {
        delay(1000);
        return;
    }

    // Processar mensagens MQTT/RPC (igual ao main)
    tb->loop();

    // Telemetria automática se habilitada
    if (autoTelemetryEnabled)
    {
        enviarTelemetria();
    }
}

void EasyThingsBoard::enviarTelemetria()
{
    if (millis() - lastTelemetry < telemetryInterval)
        return;
    lastTelemetry = millis();

    // Simular sensores (igual ao main)
    float temperatura = random(200, 300) / 10.0f; // 20.0-29.9°C
    float umidade = random(400, 800) / 10.0f;     // 40.0-79.9%

    // Enviar telemetrias (igual ao main)
    tb->sendTelemetryData("temperature", temperatura);
    tb->sendTelemetryData("humidity", umidade);
    tb->sendTelemetryData("ledState", ledState);

    Serial.printf("📊 Temp: %.1f°C | Umidade: %.1f%% | LED: %s\n",
                  temperatura, umidade, ledState ? "ON" : "OFF");
}

void EasyThingsBoard::setLED(bool state)
{
    if (ledPin == -1)
        return;

    ledState = state;
    digitalWrite(ledPin, state ? HIGH : LOW);
    Serial.printf("💡 LED %s\n", state ? "LIGADO" : "DESLIGADO");
}

void EasyThingsBoard::sendTelemetry(const char *key, float value)
{
    if (isConnected())
    {
        tb->sendTelemetryData(key, value);
    }
}

void EasyThingsBoard::sendTelemetry(const char *key, int value)
{
    if (isConnected())
    {
        tb->sendTelemetryData(key, value);
    }
}

void EasyThingsBoard::sendTelemetry(const char *key, bool value)
{
    if (isConnected())
    {
        tb->sendTelemetryData(key, value);
    }
}

void EasyThingsBoard::sendTelemetry(const char *key, const char *value)
{
    if (isConnected())
    {
        tb->sendTelemetryData(key, value);
    }
}

void EasyThingsBoard::setTelemetryInterval(unsigned long intervalMs)
{
    telemetryInterval = intervalMs;
    Serial.printf("⏱️ Intervalo telemetria: %lu ms\n", intervalMs);
}

void EasyThingsBoard::enableAutoTelemetry(bool enable)
{
    autoTelemetryEnabled = enable;
    Serial.printf("📊 Telemetria automática: %s\n", enable ? "HABILITADA" : "DESABILITADA");
}

bool EasyThingsBoard::isConnected() const
{
    return WiFi.status() == WL_CONNECTED && tb->connected();
}

String EasyThingsBoard::getLocalIP() const
{
    return WiFi.localIP().toString();
}

int EasyThingsBoard::getWiFiStrength() const
{
    return WiFi.RSSI();
}

// ===== CALLBACKS PADRÃO DO LED (baseados no main) =====

void EasyThingsBoard::onSetState(const JsonVariantConst &data, JsonDocument &response)
{
    if (!instance)
        return;

    Serial.println("📞 RPC setState recebido");

    // Aceita tanto boolean direto quanto {"enabled": true} (igual ao main)
    bool novoEstado = data.is<bool>() ? data.as<bool>() : data["enabled"];

    instance->setLED(novoEstado);

    response["state"] = instance->ledState;
    response["success"] = true;
}

void EasyThingsBoard::onGetState(const JsonVariantConst &data, JsonDocument &response)
{
    if (!instance)
        return;

    Serial.println("📞 RPC getState recebido");
    response["state"] = instance->ledState;
    response["enabled"] = instance->ledState;
}
