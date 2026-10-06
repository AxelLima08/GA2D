#include <WiFi.h>
#include <ESPmDNS.h>
/*
    Se utiliza la variable 'inicio' en cada funcion para simular un setup
    *agregar otros comentarios*

*/




// Variables para conectarse a WiFi
//-----------------------------------------
constexpr const char* SSID = "SSID";
constexpr const char* PASSWORD = "PASSWORD";
// tiempos
constexpr uint16_t tiempoDeEsperaWiFi = 30000; // Tiempo que tarda en volver a intentarlo, maximo 65.535 por ser uint16_t, 
constexpr uint16_t tiempoDeEsperaMDNS = 10000;
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
    // Cosas de la interrupcion ISR
    //-------------------------------------
    My_timer = timerBegin(1000000);           // Frecuencia del reloj (1 MHz, 1 tick = 1 microsegundo)
    timerAttachInterrupt(My_timer, &onTimer); // Cuando llegue a su limite ejecuta la funcion 'onTimer'
    timerAlarm(My_timer, 1000, true, 0);      // el segundo argumento marca cada cuantos ticks dispara 'onTimer',
    // el tercer argumento marca si se autoreinicia el contador de My_timer
    // y el cuarto cuantas veces se dispara la interrupcion, en ese caso 0 significa que lo hara indefinidamente
    //-------------------------------------

    // Serial.begin(115000);

}

void loop(){
    if(banderaTimer){ // si ya paso 1ms
        banderaTimer = false;
        iniciarWiFi();
    }
}

void iniciarWiFi(){

    static uint16_t contador;
    static uint8_t estado = 0;
    
    if(conexionAWiFi) return; // Si ya esta conectado salir a loop permanentemente
    switch(estado){
        case 0:
            WiFi.begin(SSID,PASSWORD);
            // Serial.println("Comenzo a conectarse a WiFi");
            estado = 1;
            break;
        case 1:
            if(contador >= tiempoDeEsperaWiFi){
                estado = 2;
            }
            contador++;
            break;
        case 2:
            if(WiFi.status() == WL_CONNECTED){
                //Serial.println("Se conceto a WiFi ^^")
                estado = 3;
            }
            else{
                estado = 1;
                //Serial.println("Fallo la conexion a WiFi volviendo a conectar")
                contador = 0;
            }
            break;
        case 3:
            if(MDNS.begin("esp32-GA2D")){
                //Serial.println("MDNS iniciado correctamente");
                estado = 4;
            }
            else{
            //Serial.println("fallo aliniciar MDNS");
            //while(true) delay(1000);
            }
            break;
        case 4:
            server.begin;

    }
