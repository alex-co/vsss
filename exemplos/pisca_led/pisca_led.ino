/*
*    Exemplo referente as questões 1.1a e 1.1b de Atividades.txt
* 
*  - Escalonador sem chamadas de funções.
*
*  - LED piscando com ciclos de 0.8s, 1.6s, 3.2s, 4s e 8s.
*    A contagem dos ciclos devem ser transmitida via interface serial.
* 
*    Objetivo: Trabalhar com multiplicações e divisões por potências 
*              de 2 usando deslocamentos de bits.
*
*/
/* ****************************************************************** */

#include <avr/io.h>
#include <avr/pgmspace.h>

/* ****************************************************************** */
/* *** Definições diversas ****************************************** */

// =================================================================== //
// Ref Ex 1.1b - Descomentar somente uma das definições abaixo por vez:

#define LED_08S	        // LED pisca com ciclo de 0.8s
//#define LED_16S         // LED pisca com ciclo de 1.6s
//#define LED_32S         // LED pisca com ciclo de 3.2s
//#define LED_20S         // LED pisca com ciclo de 2.0s
//#define LED_40S         // LED pisca com ciclo de 4.0s
//#define LED_80S         // LED pisca com ciclo de 8.0s

// =================================================================== //

#define SSPEED        115200   // Velocidade da interface serial


/* ****************************************************************** */
/* Estas são as conexões de hardware mapeadas aos pinos do Arduino ** */

#define LED         4      // Led conectado a uma saída digital

#define RADIO_CE    7      // Pino CE do módulo de rádio
#define RADIO_CS    8      // Pino CS do módulo do rádio
#define RADIO_A0   A4      // Bit 0 do end. do rádio (LOW = ligado)
#define RADIO_A1   A5      // Bit 1 do end. do rádio (LOW = ligado)

#define IRQ_ENC_A   2      // Pino de interrupção do Encoder A
#define IRQ_ENC_B   3      // Pino de interrupção do Encoder B
#define IRQ_RADIO   5      // Pino de interrupção do Rádio

#define HBRID_EN    6      // Habilita a ponte H (High)
#define MTR_AIN1   A2      // Bit 0 - Controle da ponte H do Motor A
#define MTR_AIN2   A3      // Bit 1 - Controle da ponte H do Motor A
#define MTR_BIN1   A1      // Bit 0 - Controle da ponte H do Motor B
#define MTR_BIN2   A0      // Bit 1 - Controle da ponte H do Motor B
#define MTR_PWMA    9      // Sinal de PWM para controle  do Motor A
#define MTR_PWMB   10      // Sinal de PWM para controle  do Motor B

#define VOLT_BAT   A7      // Tensão da bateria -> Vcc/10

/* ******************************************************************* */
/* Definições de estruturas de dados ( funcionais, status e controle ) */


typedef struct {
    uint32_t last_10ms   = 0;   // Controle das tarefas executadas a cada 10ms
    uint32_t last_100ms  = 0;   // Controle das tarefas executadas a cada 100ms
    uint32_t last_1000ms = 0;   // Controle das tarefas executadas a cada 1000ms
} TasksTCtr;

/* ******************************************************************* */
/* *** Variáveis globais e instanciações ***************************** */


TasksTCtr tasks;		// Contagem de tempo para execução de tarefas

uint32_t counter;		
uint32_t blinker;		


/* ******************************************************************* */
/* *** Protótipos das funções **************************************** */


/* ******************************************************************* */
/* *** SETUP ********************************************************* */

void setup() {

    Serial.begin(115200);               // Inicialização da com. serial

    // Inicialização do pino do LED
    pinMode(LED, OUTPUT);               // Pino do LED como saída digital
    digitalWrite(LED, LOW);
    
    counter = 0;
    blinker = 0;   
}


/* ******************************************************************* */
/* *** LOOP PRINCIPAL ************************************************ */

void loop() {
    
    // ******************************************************* //
    // Tarefas que devem ser executadas em intervalos de 10ms
    if( (millis() - tasks.last_10ms) > 10 ){
        tasks.last_10ms = millis();



    }
    // ******************************************************* //
    // Tarefas que devem ser executadas em intervalos de 100ms
    if( (millis() - tasks.last_100ms) > 100 ){
        tasks.last_100ms = millis();

        // Conta número de eventos de 100ms
        #if  defined(LED_08S) || defined(LED_16S) || defined(LED_32S)
        blinker++;
        #endif
        
        // Pisca o LED com ciclo de 0.8s
        #ifdef LED_08S
        digitalWrite(LED, bitRead(blinker, 2));
        if( !(blinker & B111) ){
            counter++;
            Serial.println(counter);
        }
        #endif
        
        // Pisca o LED com ciclo de 1.6s
        #ifdef LED_16S
        digitalWrite(LED, bitRead(blinker, 3));
        if( !(blinker & B1111) ){       // Incrementa contador e incrementa contador quando
            counter++;                  // os quatro últimos bits de 'blinker' forem zero.
            Serial.println(counter);
        }
        #endif

       // Pisca o LED com ciclo de 3.2s
       #ifdef LED_32S
       digitalWrite(LED, bitRead(blinker, 4));
       if( !(blinker & B11111) ){
           counter++;
           Serial.println(counter);
       }
       #endif


    }
    // ******************************************************* //
    // Tarefas que devem ser executadas em intervalos de 1000ms
    if( (millis() - tasks.last_1000ms) > 1000 ){
        tasks.last_1000ms = millis();

        // Conta número de eventos de 1000ms
        #if  defined(LED_20S) || defined(LED_40S) || defined(LED_80S)
        blinker++;
        #endif
        
        // Pisca o LED com ciclo de 2s
        #ifdef LED_20S
        digitalWrite(LED, bitRead(blinker, 0));
        if( !(blinker & B1) ){
            counter++;
            Serial.println(counter);
        }
        #endif

       // Pisca o LED com ciclo de 4s
       #ifdef LED_40S
       digitalWrite(LED, bitRead(blinker, 1));
       if( !(blinker & B11) ){
           counter++;
           Serial.println(counter);
       }
       #endif

       // Pisca o LED com ciclo de 8s
       #ifdef LED_80S
       digitalWrite(LED, bitRead(blinker, 2));
       if( !(blinker & B111) ){
           counter++;
           Serial.println(counter);
        }
        #endif
    
    }
    // ******************************************************* //
}


/* ******************************************************************* */
/* *** FUNÇÕES (implementações) ************************************** */



/* ****************************************************************** */
/* ****************************************************************** */
 
