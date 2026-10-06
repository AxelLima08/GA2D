#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
/*
    SERVIDOR
    Se utiliza una maquina de estados dentro de iniciarWiFi() para simular un setup
    sin bloquear el programa. La funcion se ejecuta cada 1 ms gracias al timer.

    Estados:
      0 -> inicia la conexion WiFi
      1 -> espera tiempoDeEsperaWiFi ms
      2 -> verifica si se conecto (si no, vuelve al 1)
      3 -> inicia mDNS con el nombre "esp32-GA2D"
      4 -> inicia el servidor TCP en el puerto 80
      5 -> atiende clientes
*/

// Variables para conectarse a WiFi
//-----------------------------------------
constexpr const char* SSID = "Mahiro <3";
constexpr const char* PASSWORD = "Hola1234";
// tiempos (en ms, porque la funcion se ejecuta cada 1 ms)
constexpr uint16_t tiempoDeEsperaWiFi = 5000; // maximo 65.535 por ser uint16_t
//-----------------------------------------


// Variables y funcion de la interrupcion ISR
//-----------------------------------------
volatile bool banderaTimer = false;
hw_timer_t *My_timer = NULL;
// Rutina de Interrupción (ISR)
void IRAM_ATTR onTimer() {
    banderaTimer = true;
}
//-----------------------------------------


// Prototipos de funciones
//-----------------------------------------
void iniciarWiFi();
//-----------------------------------------

WiFiServer server(80);


void setup(){
    Serial.begin(115200);

    // Cosas de la interrupcion ISR
    //-------------------------------------
    My_timer = timerBegin(1000000);           // Frecuencia del reloj (1 MHz, 1 tick = 1 microsegundo)
    timerAttachInterrupt(My_timer, &onTimer); // Cuando llegue a su limite ejecuta la funcion 'onTimer'
    timerAlarm(My_timer, 1000, true, 0);      // cada 1000 ticks (1 ms), autoreinicio, 0 = indefinidamente
    //-------------------------------------
}

void loop(){
    if(banderaTimer){ // si ya paso 1 ms
        banderaTimer = false;
        iniciarWiFi();
    }
}

void iniciarWiFi(){

    static uint16_t contador = 0;
    static uint8_t estado = 0;

    switch(estado){
        case 0:
            WiFi.begin(SSID, PASSWORD);
            Serial.println("Comenzo a conectarse a WiFi");
            contador = 0;
            estado = 1;
            break;

        case 1:
            if(contador >= tiempoDeEsperaWiFi){
                contador = 0;
                estado = 2;
            }
            else{
                contador++;
            }
            break;

        case 2:
            if(WiFi.status() == WL_CONNECTED){
                Serial.println("Se conecto a WiFi ^^");
                Serial.println("IP: " + WiFi.localIP().toString());
                estado = 3;
            }
            else{
                Serial.println("Fallo la conexion a WiFi, volviendo a intentar");
                estado = 1;
            }
            break;

        case 3:
            if(MDNS.begin("esp32-GA2D")){
                MDNS.addService("http", "tcp", 80);
                Serial.println("MDNS iniciado correctamente");
                estado = 4;
            }
            else{
                Serial.println("Fallo al iniciar MDNS, reintentando");
            }
            break;

        case 4:
            server.begin();
            Serial.println("Servidor TCP escuchando en el puerto 80");
            estado = 5;
            break;

        case 5: {
            static WiFiClient cliente; // static: no se destruye al salir de la funcion

            if(!cliente || !cliente.connected()){
                cliente = server.accept(); // en core 2.x usar server.available()
                if(cliente) Serial.println("Cliente conectado");
            }
            else if(cliente.available()){
                String linea = cliente.readStringUntil('\n');
                Serial.println("Recibido: " + linea);
                cliente.println("Hola desde el servidor");
            }
            break;
        }
    }
}
