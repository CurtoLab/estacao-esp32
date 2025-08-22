// ESP32 ThingsBoard - EasyThingsBoard baseada no código que funciona!
// Agora é MUITO mais simples mas mantém todos os callbacks!

#include <Arduino.h>
#include <EasyThingsBoard.h>

// ===== CONFIGURAÇÕES =====
const char *WIFI_SSID = "CURTOCIRCUITO";
const char *WIFI_PASSWORD = "Curto@1020";
const char *TB_TOKEN = "odiDQPrSXYfpZ9Lc6lXz";
const char *TB_SERVER = "lab.curtocircuito.com.br";
const int TB_PORT = 1883;
const int LED_PIN = 2;

// ===== BIBLIOTECA =====
EasyThingsBoard tb;

// ===== CALLBACKS CUSTOMIZADOS =====

// Callback para reiniciar ESP32
void onReset(const JsonVariantConst &data, JsonDocument &response) {
    Serial.println("🔄 RPC: Reiniciando ESP32...");
    response["message"] = "ESP32 será reiniciado";
    response["success"] = true;
    
    delay(1000);
    ESP.restart();
}

// Callback para obter status do sistema
void onStatus(const JsonVariantConst &data, JsonDocument &response) {
    Serial.println("📊 RPC: Status do sistema");
    
    response["uptime"] = millis() / 1000;
    response["freeHeap"] = ESP.getFreeHeap();
    response["wifiStrength"] = tb.getWiFiStrength();
    response["localIP"] = tb.getLocalIP();
    response["success"] = true;
}

void setup() {
    Serial.begin(115200);
    Serial.println("🚀 EasyThingsBoard - Super Simples!");
    
    // ===== CONECTAR (1 linha!) =====
    tb.connect(WIFI_SSID, WIFI_PASSWORD, TB_TOKEN, TB_SERVER, TB_PORT);
    
    // ===== CONFIGURAR LED (1 linha!) =====
    tb.setupLED(LED_PIN);  // Já adiciona setState/getState automaticamente!
    
    // ===== ADICIONAR CALLBACKS CUSTOMIZADOS =====
    tb.addRPC("reset", onReset);
    tb.addRPC("status", onStatus);
    
    // OU registrar múltiplos de uma vez (igual ao main original):
    /*
    const RPC_Callback callbacks[] = {
        {"reset", onReset},
        {"status", onStatus}
    };
    tb.registerRPCs(callbacks, 2);
    */
    
    // ===== CONFIGURAR TELEMETRIA =====
    tb.setTelemetryInterval(3000);  // 3 segundos
    // tb.enableAutoTelemetry(true);  // Habilitar se quiser telemetria automática
    
    Serial.println("✅ Sistema pronto!");
    Serial.println("📋 RPCs disponíveis:");
    Serial.println("   • setState - Liga/desliga LED");
    Serial.println("   • getState - Estado do LED"); 
    Serial.println("   • reset - Reinicia ESP32");
    Serial.println("   • status - Status do sistema");
}

void loop() {
    tb.loop();  
    tb.sendTelemetry("temperatura", 25.5f);  // 'f' especifica float
    tb.sendTelemetry("meuSensor", 123);
    delay(3000);
}
