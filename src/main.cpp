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

// ===== VARIÁVEIS PARA ARMAZENAR DADOS =====
int velocidadeSalva = 0;  // Velocidade atual (0-100%)

// Callback para configurar velocidade
void onSetVelocidade(const JsonVariantConst &data, JsonDocument &response) {
    Serial.println("⚡ RPC: Configurando velocidade..."); 
    int novaVelocidade = data.as<int>();
    
    if (novaVelocidade >= 0 && novaVelocidade <= 100) {
        velocidadeSalva = novaVelocidade;
        Serial.printf("Velocidade configurada: %d%% \n", velocidadeSalva);
    }
}

// Callback para consultar velocidade atual
void onGetVelocidade(const JsonVariantConst &data, JsonDocument &response) {
    Serial.println("RPC: Consultando velocidade atual...");
    Serial.printf("Velocidade atual: %d%\n", velocidadeSalva);
}

void setup() {
    Serial.begin(115200);
    Serial.println("EasyThingsBoard - Super Simples!");
    
    // ===== CONECTAR =====
    tb.connect(WIFI_SSID, WIFI_PASSWORD, TB_TOKEN, TB_SERVER, TB_PORT);
    tb.setupLED(LED_PIN);  // Já adiciona setState/getState automaticamente!
    
    // ===== CONFIGURAR CALLBACKS CUSTOMIZADOS =====
    tb.addRPC("setVelocidade", onSetVelocidade);  // ← RPC para configurar velocidade
    tb.addRPC("getVelocidade", onGetVelocidade);  // ← RPC para consultar velocidade
    Serial.println("✅ Sistema pronto!");
}

void loop() {
    tb.loop();  
    tb.sendTelemetry("temperatura", 25.5f);  // 'f' especifica float
    tb.sendTelemetry("velocidade", velocidadeSalva);  // ← Envia velocidade atual
    delay(3000);
}
