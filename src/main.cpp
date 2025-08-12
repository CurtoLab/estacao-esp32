#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Configurações
const char *wifi_ssid = "CURTOCIRCUITO";
const char *wifi_password = "Curto@1020";
const char *token = "cJDdWN8GNKSHOAVGLG1K";
const char *thingsboard_server = "demo.thingsboard.io";
const int thingsboard_port = 1883;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Estado do dispositivo
bool deviceState = false;
const int LED_PIN = 2;

// --- RPC (API 0.15.x usa RPC_Request / RPC_Request_Callback) ---
// Callback para mensagens MQTT recebidas
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("📞 Mensagem recebida no tópico: ");
  Serial.println(topic);
  
  // Converter payload para string
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println("� Payload: " + message);
  
  // Verificar se é um RPC
  String topicStr = String(topic);
  if (topicStr.startsWith("v1/devices/me/rpc/request/")) {
    // Extrair ID da requisição
    int lastSlash = topicStr.lastIndexOf('/');
    String requestId = topicStr.substring(lastSlash + 1);
    
    Serial.println("🔧 RPC Request ID: " + requestId);
    
    // Parse do JSON simples para extrair o método
    if (message.indexOf("\"setState\"") >= 0) {
      // Extrair valor do parâmetro
      bool newState = false;
      if (message.indexOf("true") >= 0) {
        newState = true;
      }
      
      deviceState = newState;
      digitalWrite(LED_PIN, deviceState ? HIGH : LOW);
      
      Serial.println("💡 LED alterado para: " + String(deviceState ? "ON" : "OFF"));
      
      // Responder ao RPC
      String responseTopic = "v1/devices/me/rpc/response/" + requestId;
      String response = String(deviceState ? "true" : "false");
      
      client.publish(responseTopic.c_str(), response.c_str());
      Serial.println("� Resposta enviada: " + response);
      
    } else if (message.indexOf("\"getState\"") >= 0) {
      Serial.println("📊 Estado atual: " + String(deviceState ? "ON" : "OFF"));
      
      // Responder com estado atual
      String responseTopic = "v1/devices/me/rpc/response/" + requestId;
      String response = String(deviceState ? "true" : "false");
      
      client.publish(responseTopic.c_str(), response.c_str());
      Serial.println("📤 Resposta enviada: " + response);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Configurar LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("🚀 Iniciando ESP32-S3...");
  
  // Conectar WiFi
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("📶 Conectando WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("✅ WiFi conectado!");
  Serial.println("📍 IP: " + WiFi.localIP().toString());
  
  // Configurar MQTT
  client.setServer(thingsboard_server, thingsboard_port);
  client.setCallback(callback);
  
  Serial.println("🔧 Cliente MQTT configurado!");
}

void connectToThingsBoard() {
  while (!client.connected()) {
    Serial.println("🔌 Conectando ao ThingsBoard...");
    
    if (client.connect("ESP32Device", token, "")) {
      Serial.println("✅ Conectado ao ThingsBoard!");
      
      // Subscrever aos RPCs
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("📡 Subscrito aos RPCs!");
      
    } else {
      Serial.print("❌ Falha na conexão, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5s...");
      delay(5000);
    }
  }
}

void loop() {
  // Verificar conexão WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("📶 Reconectando WiFi...");
    WiFi.begin(wifi_ssid, wifi_password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\n✅ WiFi reconectado!");
  }
  
  // Conectar ao ThingsBoard se necessário
  if (!client.connected()) {
    connectToThingsBoard();
  }
  
  // Processar mensagens MQTT
  client.loop();
  
  // Enviar telemetria a cada 10 segundos
  static unsigned long lastTelemetry = 0;
  if (millis() - lastTelemetry > 10000) {
    // Dados de exemplo
    float temperature = 25.5 + random(-50, 50) / 10.0;
    float humidity = 60.0 + random(-200, 200) / 10.0;
    
    // Criar JSON de telemetria
    String telemetry = "{\"temperature\":" + String(temperature) + 
                      ",\"humidity\":" + String(humidity) + 
                      ",\"ledState\":" + String(deviceState ? "true" : "false") + "}";
    
    if (client.publish("v1/devices/me/telemetry", telemetry.c_str())) {
      Serial.println("📊 Telemetria enviada: " + telemetry);
    } else {
      Serial.println("❌ Erro ao enviar telemetria");
    }
    
    lastTelemetry = millis();
  }
  
  delay(100);
}
