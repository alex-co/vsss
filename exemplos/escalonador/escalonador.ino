
#include <avr/io.h>
#include <avr/pgmspace.h>

/* ****************************************************************** */
/* *** Definições diversas ****************************************** */

// Descomente para inverter a rotação (somente) do Motor A
// => Necessário com os motores montados simetricamente

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

uint32_t  blinker;		// Contagem de alterações de estado do LED


/* ******************************************************************* */
/* *** Protótipos das funções **************************************** */
//
// Obs: Este bloco não é necessário para compilação mas é útil como
//      referência durante o processo de desenvolvimento.

void     tasks_10ms( void );
void     tasks_100ms( void );
void     tasks_1000ms( void );


/* ******************************************************************* */
/* *** SETUP ********************************************************* */

void setup() {

    Serial.begin(115200);               // Inicialização da com. serial

    // Inicialização do pino do LED
    pinMode(LED, OUTPUT);               // Pino do LED como saída digital
    digitalWrite(LED, LOW);
    
    blinker = 0;                       // Inicialização da variável global
}


/* ******************************************************************* */
/* *** LOOP PRINCIPAL ************************************************ */

void loop() {
    
    tasks_10ms();                // Tarefas executadas a cada 10ms
    tasks_100ms();               // Tarefas executadas a cada 100ms
    tasks_1000ms();              // Tarefas executadas a cada 1000ms

}


/* ******************************************************************* */
/* *** FUNÇÕES (implementações) ************************************** */

/* *********************************************************************
 * Tarefas que devem ser executadas em intervalos de 10ms
 */
void tasks_10ms( void ) {

     if( (millis() - tasks.last_10ms) > 10 ){
        tasks.last_10ms = millis();



    }
}

/* *********************************************************************
 * Tarefas que devem ser executadas em intervalos de 100ms
 */
void tasks_100ms( void ) {

    if( (millis() - tasks.last_100ms) > 100 ){
        tasks.last_100ms = millis();



    }
}

/* *********************************************************************
 * Tarefas que devem ser executadas em intervalos de 1000ms
 */
void tasks_1000ms( void ) {

    if( (millis() - tasks.last_1000ms) > 1000 ){
        tasks.last_1000ms = millis();

		// Conta execuções
        blinker++;
        // Escreve "bit 0" de "blinker" para o LED 
        digitalWrite(LED, bitRead(blinker, 0));
        
        // Serial.write(blinker);  // Escreve dados binários na porta serial
        // Serial.print(blinker);  // Imprime dados na porta serial como texto ASCII
        Serial.println(blinker);   // Idem ao "print", adicionando EOL ao string
    
    }
}


/* ****************************************************************** */
/* ****************************************************************** */
