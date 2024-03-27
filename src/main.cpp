#include <Arduino.h>

#include "string.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <pwmWrite.h>
#include "PageHtml.h"

#define STASSID "igo"
#define STAPSK "tiopaulo123"
#define LED     2

const char *ssid = STASSID;
const char *password = STAPSK;


const byte pwmPin[3] = {21, 19, 18};
const uint16_t duty = 341;
const byte deadtime = 21;
const uint16_t phase[3] = {0, 341, 682};
const uint16_t shift[3] = {452, 452, 0};
const uint32_t frequency = 40000;
const byte resolution = 10;

uint8_t data_array[1000];

char *text;
int i = 0;

Pwm pwm = Pwm();
unsigned long inter;

int PWM_data[1000];
int PWM_data_pos[1000];
int PWM_data_neg[1000];

AsyncWebServer server(80);

bool data_uploaded = false;

int data_points = 0;

String trataWeb1(const String& var){
    if(var == "01") return String(duty);
    return String('%');
}

void handleRoot(AsyncWebServerRequest *request) {
    String Pag=FPSTR(PgWeb);
    request->send_P(200, "text/html", PgWeb, trataWeb1);
}

int atoi_modificado(char *str){
    int res = 0;
    bool negativo = false;
    for (int i = 0; str[i] != '\0' && str[i] != '\n'; ++i){
        if(str[i]=='-'){
            negativo = true;
            continue;
        }
        if(str[i] < '0' || str[i] > '9')
            continue;

        res = res*10 + str[i] - '0';
    }
    if(negativo){
        res = -res;
    }
    return res;
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
//    esp_task_wdt_init(30, false);
    Serial.println("Recebendo arquivo");
    Serial.println(filename);
    Serial.printf("The len is %u\n", len);
    Serial.printf("index = %d\n", index);

    for (int k = 0; k < len; k++) {
        data_array[index + k] = data[k];
    }

    char *token = (char *)data_array;

    int aux =0;
    data_points = 0;
    while (true) {
        PWM_data[aux] = atoi_modificado(token);
        data_points++;

        if(PWM_data[aux]>0){
            PWM_data_pos[aux] = PWM_data[aux];
            PWM_data_neg[aux] = 0;
        } else{
            PWM_data_pos[aux] = 0;
            PWM_data_neg[aux] = - PWM_data[aux];
        }

        Serial.printf("%d\n", PWM_data[aux]);

        while(*token != '\n' && *token != '\0'){
            token++;
        }
        if (*token == '\n') {
            token++;
        }
        if (*token == '\0') {
            break;
        }

        aux++;
    }
    data_uploaded = true;
}

void notFound(AsyncWebServerRequest *request) {
    String message = "File Not Found\n\n";
    request->send(404, "text/plain", message);
    Serial.println("Pagina não encontrada "+request->url());
}



void setup_wifi(){
    Serial.print(F("Conectando a Rede "));
    Serial.println(STASSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(F("."));
    }

    Serial.println();
    Serial.println(F("WiFi Conectado"));
    Serial.println(F("Endereço IP: ")); Serial.println(WiFi.localIP());
}

void setup_pwm(){
    pwm.pause();
    pwm.resume();
    pwm.printDebug();
    inter=micros();
}


void setup() {
    Serial.begin(9600);
    pinMode(LED, OUTPUT);
    analogWriteFrequency(15000);

    setup_wifi();
    setup_pwm();

    server.on("/", HTTP_GET, handleRoot);

    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
        Serial.println("Chegou aqui");
        request->send(200); }, handleUpload);

    server.onNotFound(notFound);
    server.begin();
    Serial.println("Servidor Iniciado!");


}

void loop() {
    if (data_uploaded){
        if (micros()-inter>=75)
        {
            if(i<data_points) {
                pwm.write(14, PWM_data_pos[i], frequency, resolution, 0);
                pwm.write(27, PWM_data_neg[i], frequency, resolution, 0);
                i++;
            }
            else {i=0;}
            inter=micros();
        }

    }
}
