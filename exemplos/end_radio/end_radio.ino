/*
*  Exemplo referente a questão 1.2 de Atividades.txt
* 
*  - Lê o endereço de rádio, definido pelo dipswitch
* 
*    Objetivo: Trabalhar com pinos individuais de I/O.
*
*/
/* ****************************************************************** */

#include <avr/io.h>
#include <avr/pgmspace.h>

/* ****************************************************************** */
/* *** Definições diversas ****************************************** */

// =================================================================== //
// Ref Ex 1.2 - Descomentar somente uma das definições abaixo por vez:

//#define RD_PINS      // Lê pinos de I/O individualmente
#define RD_PORT      // Lê PORTC ( PINC ) do Atmega328

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


TasksTCtr tasks;        // Contagem de tempo para execução de tarefas


/* ******************************************************************* */
/* *** Protótipos das funções **************************************** */

uint8_t get_node_addr( void );


/* ******************************************************************* */
/* *** SETUP ********************************************************* */

void setup() {

    Serial.begin(115200);   // Inicialização da com. serial
    
    // Inicialização dos pinos de endereçamento do rádio
    pinMode(RADIO_A0, INPUT_PULLUP);    // Endereço - bit 0
    pinMode(RADIO_A1, INPUT_PULLUP);    // Endereço - bit 1
    
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




    }
    // ******************************************************* //
    // Tarefas que devem ser executadas em intervalos de 1000ms
    if( (millis() - tasks.last_1000ms) > 1000 ){
        tasks.last_1000ms = millis();

        // Envia valor do end. de rário via serial
        Serial.print("End. de rádio: ");
        Serial.println( get_node_addr() );

    
    }
    // ******************************************************* //
}


/* ******************************************************************* */
/* *** FUNÇÕES (implementações) ************************************** */

uint8_t get_node_addr( void ){
    
   
    // Abordagem: Leitura individual dos pinos
    #ifdef RD_PINS
    uint8_t addr = 0xFF;
    addr  = addr << 1;
    addr |= digitalRead(RADIO_A1);
    addr  = addr << 1;
    addr |= digitalRead(RADIO_A0);
    addr = ~addr;
    #endif

    // Abordagem: Leitura do PORTC (PINC) do uC
    // RADIO_A0 = Arduino A4 = uC pino 27 = PC4 (bit 4)
    // RADIO_A1 = Arduino A5 = uC pino 28 = PC5 (bit 5)
    #ifdef RD_PORT
    uint8_t addr = 0;
    addr |= PINC >> 4;    // Lê todos os bits de PORTC e ajusta posição
    addr |= B11111100;    // Todos os bits que não são de interesse tornam-se 1
    addr  = ~addr;        // Inverte o byte (chave posição ON = nível lógico 0)
    #endif

    return addr;
}

/* ****************************************************************** */
/* ****************************************************************** */
 
