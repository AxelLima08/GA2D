#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
/*
    CLIENTE
    Se utiliza una maquina de estados dentro de iniciarWiFi() para simular un setup
    sin bloquear el programa. La funcion se ejecuta cada 1 ms gracias al timer.

    Estados:
      0 -> inicia la conexion WiFi
      1 -> espera tiempoDeEsperaWiFi ms
      2 -> verifica si se conecto (si no, vuelve al 1)
      3 -> inicia mDNS con el nombre "esp-loro"
      4 -> busca "esp32-GA2D.local" (queryHost)
      5 -> abre la conexion TCP al puerto 80 y envia un mensaje
      6 -> terminado
      7 -> espera tiempoDeEsperaMDNS ms antes de reintentar la busqueda
*/

// Variables para conectarse a WiFi
//-----------------------------------------
constexpr const char* SSID = "Mahiro <3";
constexpr const char* PASSWORD = "Hola1234";
// tiempos (en ms, porque la funcion se ejecuta cada 1 ms)
constexpr uint16_t tiempoDeEsperaWiFi = 5000; // maximo 65.535 por ser uint16_t
constexpr uint16_t tiempoDeEsperaMDNS = 5000; // espera entre reintentos de busqueda/conexion
//-----------------------------------------

IPAddress servidorIP; // IP del servidor encontrada por mDNS


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
            if(MDNS.begin("esp-loro")){
                Serial.println("MDNS cliente iniciado");
                estado = 4;
            }
            else{
                Serial.println("Fallo MDNS.begin, reintentando");
            }
            break;

        case 4: {
            IPAddress ip = MDNS.queryHost("esp32-GA2D"); // bloquea hasta ~2 s
            if(ip == IPAddress(0, 0, 0, 0)){
                Serial.println("No se encontro el host, reintentando en unos segundos");
                contador = 0;
                estado = 7;
                break;
            }
            servidorIP = ip;
            Serial.println("Servidor en " + ip.toString());
            estado = 5;
            break;
        }

        case 5: {
            WiFiClient cliente;
            if(cliente.connect(servidorIP, 80)){
                Serial.println("Conectado al servidor ^^");
                cliente.println("Hola desde el cliente");

                // Espera la respuesta (maximo 2 s)
                unsigned long t0 = millis();
                while(!cliente.available() && millis() - t0 < 2000){
                    delay(10);
                }
                if(cliente.available()){
                    String respuesta = cliente.readStringUntil('\n');
                    Serial.println("Respuesta: " + respuesta);
                }
                cliente.stop();
                estado = 6;
            }
            else{
                Serial.println("Fallo la conexion TCP, reintentando en unos segundos");
                contador = 0;
                estado = 7;
            }
            break;
        }

        case 6:
            // Terminado, no hace nada mas
            break;

        case 7:
            if(contador >= tiempoDeEsperaMDNS){
                contador = 0;
                estado = 4; // vuelve a buscar el host
            }
            else{
                contador++;
            }
            break;
    }
}
