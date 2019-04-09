
#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

/* ************************************************************************* */
/* *** Definições diversas ************************************************* */


#define SSPEED        115200   // Velocidade da interface serial

// Pinos de controle do módulo de rádio (independentes da SPI)

#define RADIO_CE          A0   // Pino CE do módulo de rádio
#define RADIO_CS          A1   // Pino CS do módulo do rádio
#define RADIO_PWR         A2   // Liga/Desl. módulo de rádio

// LEDs de sinalização. 
// Obs: Os LEDs vermelhos estão conectados a pinos com PWM.

#define L1G                4   // Led 1 verde
#define L1R                5   // Led 1 vermelho
#define L2G                7   // Led 2 verde
#define L2R                6   // Led 2 vermelho
#define L3G                8   // Led 3 verde
#define L3R                9   // Led 3 vermelho


// Transforma um número de 4 bits em hexadecimal para caractere ascii. 
#define hex2asc(a) (( a < 10 )  ? ( (a) + 0x30 ) : ( (a) + ('a'-10) ))

// Transforma caractere ascii em um número de 4 bits em hexadecimal.
#define asc2hex(a) (((a) < 'a') ? ( (a) - '0' )  : ( ((a) - 'a')+10) )

// Lê um nibble de um inteiro de 32 bits dado seu índice (0 a 7).
#define Nibble(a, i) ((unsigned)(((a) >> (28 - 4 * (i))) & 0xF))

/* ************************************************************************* */
/* *** Definições de estruturas de dados *********************************** */


typedef struct {
    uint32_t last_10ms   = 0;   // Controle das tarefas executadas a cada 10ms
    uint32_t last_500ms  = 0;   // Controle das tarefas executadas a cada 500ms
} TasksTCtr;

/* ************************************************************************* */
/* *** Variáveis globais e instanciações *********************************** */

TasksTCtr tasks;       // Contagem de tempo para execução de tarefas



/* ************************************************************************* */
/* *** Protótipos das funções ********************************************** */

void    tasks_10ms( void );
void    tasks_500ms( void );


/* ************************************************************************* */
/* *** SETUP *************************************************************** */

void setup() {

    // Liga alimentação do módulo de rádio
    pinMode(RADIO_PWR, OUTPUT);
    digitalWrite(RADIO_PWR, HIGH);

    // Define pinos dos LEDs como saída
    for( int i=4; i < 10; i++ )
        pinMode( i, OUTPUT );

    Serial.begin(SSPEED);               // Inicializa a interface serial
    
    SPI.begin();                        // Inicializa a interface SPI


}

/* ************************************************************************* */
/* *** LOOP PRINCIPAL ****************************************************** */

void loop() {
    
    tasks_10ms();         // Tarefas executadas a cada 10ms
    tasks_500ms();        // Tarefas executadas a cada 500ms
}

/* ************************************************************************* */
/* *** FUNÇÕES (implementações) ******************************************** */


/* *********************************************************************
 * Tarefas que devem ser executadas em intervalos de 10ms
 */
void tasks_10ms( void ) {

     if( (millis() - tasks.last_10ms) > 10 ){
        tasks.last_10ms = millis();
        


    }
}


/* *********************************************************************
 * Tarefas que devem ser executadas em intervalos de 500ms
 */
void tasks_500ms( void ) {

    if( (millis() - tasks.last_500ms) > 500 ){
        tasks.last_500ms = millis();



    }
}



/* ****************************************************************** */
/* ****************************************************************** */
