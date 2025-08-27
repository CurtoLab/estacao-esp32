// ESP32 ThingsBoard - EasyThingsBoard baseada no código que funciona!
// Agora é MUITO mais simples mas mantém todos os callbacks!

#include <Arduino.h>
#include <EasyThingsBoard.h>
#include <Wire.h>               // Chama a Wire.h para comunição I2C
#include <SH1106Wire.h>         // Chama a biblioteca responsável pelo controle do display
#include <Adafruit_AHTX0.h>     // Biblioteca responsável pelo AHT21
#include <Adafruit_CCS811.h>    // Biblioteca responsável pelo sensor CSS811
#include <Adafruit_BMP085.h>    // Biblioteca responsável pelo sensor BMP085
#include <Adafruit_TSL2561_U.h> // Biblioteca responsável pelo TSL2561_U

// ===== CONFIGURAÇÕES =====
const char *WIFI_SSID = "CURTOCIRCUITO";
const char *WIFI_PASSWORD = "Curto@1020";
const char *TB_TOKEN = "1A5M4leR8ttlBh0n30ZL";
const char *TB_SERVER = "lab.curtocircuito.com.br";
const int TB_PORT = 1883;
const int LED_PIN = 2;

// ===== BIBLIOTECA =====
EasyThingsBoard tb;

#define GUVA_OUT_PIN 34          // Pino analógico do sensor UV GUVA-S12S
#define SENSOR_CHUVA_AOUT_PIN 32 // Pino analógico do sensor de chuva
#define SENSOR_CHUVA_DOUT_PIN 33 // Pino digital do sensor de chuva
#define BOTAO_PIN 39             // Pino do botão para alternar entre as telas

SH1106Wire display(0x3C, 21, 22); // Definir o display OLED SH1106 no endereço 0x3C

Adafruit_AHTX0 aht;                                                                 // AHT10 no endereço 0x38
Adafruit_CCS811 ccs;                                                                // CCS811 no endereço 0x5A
Adafruit_BMP085 bmp;                                                                // BMP180 no endereço 0x77
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); // TSL2561 no endereço 0x39

int sensorAtual = 0;         // Variável para alternar entre os sensores
const int totalSensores = 6; // Total de sensores conectados

unsigned long tempoAtualizacao = 1000; // Intervalo de tempo para atualizar o display
unsigned long ultimoTempo = 0;         // Marca o último tempo que o display foi atualizado

void setup()
{
    pinMode(BOTAO_PIN, INPUT);             // Seleciona o pino do botão como entrada
    pinMode(GUVA_OUT_PIN, INPUT);          // Seleciona o pino do sensor GUVA como entrada
    pinMode(SENSOR_CHUVA_AOUT_PIN, INPUT); // Seleciona o pino analógico do sensor de chuva como entrada
    pinMode(SENSOR_CHUVA_DOUT_PIN, INPUT); // Seleciona o pino digital do sensor de chuva como entrada

    Serial.begin(115200);
    Serial.println("EasyThingsBoard - Super Simples!");

    // ===== CONECTAR =====
    tb.connect(WIFI_SSID, WIFI_PASSWORD, TB_TOKEN, TB_SERVER, TB_PORT);
    // tb.setupLED(LED_PIN); // Já adiciona setState/getState automaticamente!

    Serial.println("✅ Sistema pronto!");

    Wire.begin(21, 22);                                   // Inicializar comunicação I2C
    aht.begin();                                          // Inicializar o sensor AHT10
    ccs.begin();                                          // Inicializar o sensor CCS811
    bmp.begin();                                          // Inicializar o sensor BMP180
    tsl.begin();                                          // Inicializar o sensor TSL2561
    tsl.enableAutoRange(true);                            // Verifica se o TSL2561
    tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS); // define o tempo de integração do TSL2561

    display.init();                 // Inicializar o display SH1106
    display.flipScreenVertically(); // Inverte a tela do display
    display.clear();                // Limpa o display
    display.display();              // Atualiza a tela do display

    // Exibe mensagem inicial
    display.setFont(ArialMT_Plain_16);           // Seleciona a fonte como Arial e tamanho 16
    display.setTextAlignment(TEXT_ALIGN_CENTER); // alinha o texto no centro
    display.drawString(64, 14, "Estação");       // seleciona a posição e desenha no display Estação
    display.drawString(64, 34, "Meteorológica"); // Escreve meteorológica
    display.display();                           // atualiza o display, escrevendo a mensagem acima
    display.clear();                             // limpa a tela do display
    delay(1000);                                 // delay para limpar a tela
    display.setTextAlignment(TEXT_ALIGN_CENTER); // alinha o texto o centro
    display.drawString(64, 14, "Curto");         // escreve curto
    display.drawString(64, 34, "Circuito");      // escreve circuito
    display.display();                           // atualiza a tela do display, escrevendo a mensagem acima
    display.clear();                             // limpa a tela
    delay(1000);                                 // delay de 1 segundo
}

void botao() // Variavel para controle do botão
{
    if (digitalRead(39) == 0) // Quando o botão do pino 39 for pressionado
    {
        sensorAtual++; // Adiciona o valor do sensorAtual
        delay(200);    // delay para adicionar o debounce
    }
    // Essa função reinicia a variavel sensorAtual para voltar a primeira tela
    if (sensorAtual >= 6) // Se o valor de sensorAtual for igual ou maior que 6
    {
        sensorAtual = 0; // sensorAtual se torna 0
    }
}

void mostrarAHT21() // Função que exibe o valor do sensor AHT21
{
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);

    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Sensor AHT21:");
    display.drawString(0, 20, "Temp: " + String(temp.temperature) + " C");
    display.drawString(0, 40, "Umid:  " + String(humidity.relative_humidity) + " %");
}

void mostrarCCS811() // Função que exibe o valor do sensor CCS811
{
    ccs.readData();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Sensor CCS811:");
    display.drawString(0, 20, "CO2: " + String(ccs.geteCO2()) + " ppm");
    display.drawString(0, 40, "TVOC: " + String(ccs.getTVOC()) + " ppb");
}

void mostrarBMP180() // Função que exibe o valor do senso BMP180
{
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Sensor BMP180:");
    display.drawString(0, 20, "Press: " + String(bmp.readPressure()) + " Pa");
    display.drawString(0, 40, "Altitude: " + String(bmp.readAltitude()) + " m");
}

void mostrarTSL2561() // Função que exibe o valor do sensor TSL2561
{
    sensors_event_t event;
    tsl.getEvent(&event);

    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Sensor TSL2561:");
    if (event.light)
    {
        display.drawString(0, 20, "Lux: " + String(event.light));
    }
    else
    {
        display.drawString(0, 20, "Falha na leitura!");
    }
}

float calcularIndiceUV(float voltage) // Função para controlar indice UV
{
    float UVIndex = voltage * 15.0;
    return UVIndex;
}

void mostrarGUVA() // Função que exibe o valor do sensor GUVA
{
    int UVLevel = analogRead(GUVA_OUT_PIN);
    float voltage = UVLevel * (3.3 / 4095.0);  // Converte a leitura para a tensão
    float UVIndex = calcularIndiceUV(voltage); // Calcula o índice UV

    String nivelUV;
    if (UVIndex <= 2)
    {
        nivelUV = "Baixo";
    }
    else if (UVIndex <= 5)
    {
        nivelUV = "Moderado";
    }
    else if (UVIndex <= 7)
    {
        nivelUV = "Alto";
    }
    else if (UVIndex <= 10)
    {
        nivelUV = "Muito Alto";
    }
    else
    {
        nivelUV = "Extremo";
    }

    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Sensor GUVA: ");
    display.drawString(0, 20, "UV Index: " + String(UVIndex));
    display.drawString(0, 40, "Nivel: " + nivelUV);
}

void mostrarSensorChuva() // Função para ler os valores do sensor de chuva e mostrar na tela
{
    int valorAnalogico = analogRead(SENSOR_CHUVA_AOUT_PIN);
    int valorDigital = digitalRead(SENSOR_CHUVA_DOUT_PIN);

    String nivelChuva;
    if (valorAnalogico > 3000)
    {
        nivelChuva = "Sem Chuva";
    }
    else if (valorAnalogico > 2000)
    {
        nivelChuva = "Chuva Leve";
    }
    else if (valorAnalogico > 1000)
    {
        nivelChuva = "Chuva Moderada";
    }
    else
    {
        nivelChuva = "Chuva Intensa";
    }

    // Indicação se está chovendo ou não com base na leitura digital
    String statusChuva = (valorDigital == HIGH) ? "Sem Chuva" : "Chovendo";

    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Sensor de Chuva:");
    display.drawString(0, 20, nivelChuva);
    display.drawString(0, 40, statusChuva);
}

void loop()
{
    tb.loop();

    int UVLevel = analogRead(GUVA_OUT_PIN);
    float voltage = UVLevel * (3.3 / 4095.0);  // Converte a leitura para a tensão
    float UVIndex = calcularIndiceUV(voltage); // Calcula o índice UV

    String nivelUV;
    if (UVIndex <= 2)
    {
        nivelUV = "Baixo";
    }
    else if (UVIndex <= 5)
    {
        nivelUV = "Moderado";
    }
    else if (UVIndex <= 7)
    {
        nivelUV = "Alto";
    }
    else if (UVIndex <= 10)
    {
        nivelUV = "Muito Alto";
    }
    else
    {
        nivelUV = "Extremo";
    }

    int valorAnalogico = analogRead(SENSOR_CHUVA_AOUT_PIN);
    int valorDigital = digitalRead(SENSOR_CHUVA_DOUT_PIN);

    String nivelChuva;
    if (valorAnalogico > 2500)
    {
        nivelChuva = "Sem Chuva";
    }
    else if (valorAnalogico > 1500)
    {
        nivelChuva = "Chuva Leve";
    }
    else if (valorAnalogico > 500)
    {
        nivelChuva = "Chuva Moderada";
    }
    else
    {
        nivelChuva = "Chuva Intensa";
    }

    String statusChuva = (valorDigital == HIGH) ? "Sem Chuva" : "Chovendo";

    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);

    sensors_event_t event;
    tsl.getEvent(&event);
    ccs.readData();

    tb.sendTelemetry("AHT21 - Temperatura", temp.temperature); // 'f' especifica float
    tb.sendTelemetry("AHT21 - Umidade", humidity.relative_humidity);
    tb.sendTelemetry("CCS811 - CO2", ccs.geteCO2());
    tb.sendTelemetry("CCS811 - TVOC", ccs.getTVOC());
    tb.sendTelemetry("BMP180 - Pressão", bmp.readPressure());
    tb.sendTelemetry("BMP180 - Altitude", bmp.readAltitude());
    tb.sendTelemetry("TSL2561 - Lux", event.light);
    tb.sendTelemetry("GUVA - UV Index", UVIndex);
    tb.sendTelemetry("GUVA - Nível UV", nivelUV.c_str());
    tb.sendTelemetry("Sensor de Chuva", nivelChuva.c_str());
    tb.sendTelemetry("Está Chovendo?", statusChuva.c_str());

    botao();

    unsigned long tempoAtual = millis(); // Tempo atual

    if (tempoAtual - ultimoTempo >= tempoAtualizacao)
    {
        // Atualiza o display com o sensor selecionado
        switch (sensorAtual) // Switch com a variavel sensorAtual, definindo qual sensor é mostrado na tela
        {
        case 0: // Em caso 0, chama a função mostrarAHT21, mostrando no display, as informações do sensor AHT21
            mostrarAHT21();
            break;
        case 1: // Em caso 1, chama a função mostrarCCS811, mostrando no display, as informações do sensor CCS811
            mostrarCCS811();
            break;
        case 2: // Em caso 2, chama a função mostrarBMP180, mostrando no display, as informações do sensor BMP180
            mostrarBMP180();
            break;
        case 3: // Em caso 3, chama a função mostrarTSL2561, mostrando no display, as informações do sensor TSL2561
            mostrarTSL2561();
            break;
        case 4: // Em caso 4, chama a função mostrarGUVA, mostrando no display, as informações do sensor mostrarGUVA
            mostrarGUVA();
            break;
        case 5: // Em caso 5, chama a função mostrarSensorChuva, mostrando no display, as informações do sensor Sensor Chuva
            mostrarSensorChuva();
            break;
        }

        display.display(); // Mostra os dados no display
        display.clear();   // Limpa o display para a próxima atualização

        // Atualiza o último tempo em que o display foi atualizado
        ultimoTempo = tempoAtual;
    }
}
