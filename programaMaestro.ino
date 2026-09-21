#include <WiFi.h>
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
    // Simulacion de setup
    //-------------------------------------
    static bool inicio = true;
    if(inicio){
        WiFi.begin(SSID,PASSWORD);
        // Serial.println("Comenzo la conexion a WiFi");
        inicio = false;
    }
    //-------------------------------------

    static uint16_t contadorWiFi;
    static bool conexionAWiFi = false;
    
    if(conexionAWiFi) return; // Si ya esta conectado salir a loop permanentemente

    if(contadorWiFi >= tiempoDeEsperaWiFi){ // Si ya paso tiempoDeEsperaWiFi
        if(WiFi.status() == WL_CONNECTED){ // Si ya esta conectado
            conexionAWiFi = true;
            // Serial.println("Conectado con exito ^^");
        }
        else{ // Si no se conecto
            contadorWiFi = 0;
            WiFi.begin(SSID,PASSWORD); // Reinenta conectarse
            // Serial.println("Fallo al conectarse al WiFi. Volviendo a intentar.");
        }
    }
    else{
        contadorWiFi++;
    }
}