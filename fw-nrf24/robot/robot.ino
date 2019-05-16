
#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

/* ****************************************************************** */
/* *** Definições diversas ****************************************** */


// Descomente para inverter a rotação (somente) do Motor A
// => Necessário com os motores montados simetricamente

#define SSPEED 115200   // Velocidade da interface serial

#define FULL_BAT 8000   // Valor em mV para bat. completamente carregada
#define DEAD_BAT 6000   // Valor em mV para bat. esgotada ( recarregar )


/* ****************************************************************** */
/* Limitações de PWM e velocidade para uso no controlador PID ******* */

#define PWM_MIN  0x0F   // PWM mín. p/ garantir movimento das duas rodas
#define PWM_MAX  0x9F   // PWM máx. para que os motores tenham aprox. 5V
#define SPD_MIN    50   // Vel. mín. em mm/s ( Condição: PWM > PWM_MIN )
#define SPD_MAX   500   // Vel. máx. em mm/s ( Condição: PWM < PWM_MAX )

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


/* ****************************************************************** */
/* Macros diversas que podem ser úteis ****************************** */

// Converte um número de 4 bits (hexadecimal) para caractere ascii. 
#define hex2asc(a) (( a < 10 )  ? ( (a) + 0x30 ) : ( (a) + ('a'-10) ))

// Converte um caractere ascii em um número de 4 bits (hexadecimal).
#define asc2hex(a) (((a) < 'a') ? ( (a) - '0' )  : ( ((a) - 'a')+10) )

// Lê um nibble de um inteiro de 32 bits dado seu índice (0 a 7).
#define Nibble(a, i) ((unsigned)(((a) >> (28 - 4 * (i))) & 0xF))


/* ****************************************************************** */
/* Definições de estruturas de dados (funcionais, status e controle)  */


typedef struct {
    uint32_t last_10ms   = 0; // Controle do grupo das tarefas de 10ms
    uint32_t last_100ms  = 0; // Controle do grupo das tarefas de 100ms
    uint32_t last_1000ms = 0; // Controle do grupo das tarefas de 1000ms
} TasksTCtr;


/* ******************************************************************* */
/* *** Variáveis globais e instanciações ***************************** */


TasksTCtr tasks;		// Contagem de tempo para execução de tarefas




/* ******************************************************************* */
/* *** Protótipos das funções **************************************** */
//
// Obs: Os protótipos não são necessários para compilação mas são úteis
//      como referência durante o processo de desenvolvimento.




/* ******************************************************************* */
/* *** SETUP ********************************************************* */

void setup() {

    pinMode(LED, OUTPUT);            // Inicialização do LED
    digitalWrite(LED, LOW);          
    


    Serial.begin(SSPEED);            // Inicializa a interface serial
    SPI.begin();                     // Inicializa a interface SPI
    radio.begin();                   // Inicializa o módulo de rádio 

    analogReference(INTERNAL);       // Referência dos ADCs -> 1.1V
    

}


/* ******************************************************************* */
/* *** LOOP PRINCIPAL ************************************************ */

void loop() {
    
    // Mantém a rede ativa
    network.update();
    
    // *****************************************************************
    // Tarefas que devem ser executadas em intervalos de 10ms
    if( (millis() - tasks.last_10ms) > 10 ){
        tasks.last_10ms = millis();



    }

    // *****************************************************************
    // Tarefas que devem ser executadas em intervalos de 100ms
   if( (millis() - tasks.last_100ms) > 100 ){
        tasks.last_100ms = millis();



    }

    // *****************************************************************
    // Tarefas que devem ser executadas em intervalos de 1000ms
    if( (millis() - tasks.last_1000ms) > 1000 ){
        tasks.last_1000ms = millis();


    
    }

}


/* ******************************************************************* */
/* *** FUNÇÕES (implementações) ************************************** */








/* ****************************************************************** */
/* ****************************************************************** */
