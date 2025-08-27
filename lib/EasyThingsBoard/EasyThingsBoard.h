#ifndef EASYTHINGSBOARD_H
#define EASYTHINGSBOARD_H

#include <Arduino.h>
#include <WiFi.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>
#include <Server_Side_RPC.h>

/**
 * EasyThingsBoard - Abstração do código que funciona!
 *
 * Baseado no main.cpp que já está funcionando,
 * mantendo callbacks reais e toda flexibilidade.
 */

class EasyThingsBoard
{
public:
    EasyThingsBoard();
    ~EasyThingsBoard();

    /**
     * Conectar WiFi e ThingsBoard
     */
    bool connect(const char *ssid, const char *password, const char *token,
                 const char *server = "demo.thingsboard.io", int port = 1883);

    /**
     * Configurar LED com callbacks automáticos setState/getState
     */
    void setupLED(int pin);

    /**
     * Adicionar callback RPC customizado
     */
    bool addRPC(const char *method, void (*callback)(const JsonVariantConst &data, JsonDocument &response));

    /**
     * Registrar múltiplos callbacks de uma vez (igual ao main!)
     */
    bool registerRPCs(const RPC_Callback *callbacks, uint8_t count);

    /**
     * Manter tudo funcionando - chame no loop()
     */
    void loop();

    /**
     * Controle manual do LED
     */
    void setLED(bool state);
    bool getLEDState() const { return ledState; }

    /**
     * Enviar telemetria individual
     */
    void sendTelemetry(const char *key, float value);
    void sendTelemetry(const char *key, int value);
    void sendTelemetry(const char *key, bool value);
    void sendTelemetry(const char *key, const char *value);

    /**
     * Configurar intervalo de telemetria automática
     */
    void setTelemetryInterval(unsigned long intervalMs);

    /**
     * Habilitar/desabilitar telemetria automática com dados simulados
     */
    void enableAutoTelemetry(bool enable = true);

    /**
     * Utilitários
     */
    bool isConnected() const;
    String getLocalIP() const;
    int getWiFiStrength() const;

private:
    // ===== CONFIGURAÇÕES =====
    String wifiSSID;
    String wifiPassword;
    String tbToken;
    String tbServer;
    int tbPort;

    // ===== OBJETOS THINGSBOARD (igual ao main) =====
    WiFiClient *wifiClient;
    Arduino_MQTT_Client *mqttClient;
    Server_Side_RPC<8, 16> *rpc; // Mais slots para callbacks
    IAPI_Implementation **apis;
    ThingsBoard *tb;

    // ===== ESTADO =====
    bool configured;
    bool rpcConfigured;
    int ledPin;
    bool ledState;

    // ===== TELEMETRIA AUTOMÁTICA =====
    unsigned long lastTelemetry;
    unsigned long telemetryInterval;
    bool autoTelemetryEnabled;

    // ===== CALLBACKS =====
    static constexpr uint8_t MAX_CALLBACKS = 8;
    RPC_Callback registeredCallbacks[MAX_CALLBACKS];
    uint8_t callbackCount;

    // ===== MÉTODOS INTERNOS (baseados no main) =====
    void conectarWiFi();
    bool conectarThingsBoard();
    bool configurarRPCs();
    void enviarTelemetria();

    // ===== CALLBACKS PADRÃO DO LED =====
    static void onSetState(const JsonVariantConst &data, JsonDocument &response);
    static void onGetState(const JsonVariantConst &data, JsonDocument &response);

    // Instância global para callbacks
    static EasyThingsBoard *instance;
};

#endif
