/*
*  Exemplo referente a questão 1.3 de Atividades.txt
* 
*  - Lê a tensão da bateria em mV e transmite via serial.
* 
*  Objetivo: Utilizar conversor analógico-digital.
*
*/
/* ****************************************************************** */

#include <avr/io.h>
#include <avr/pgmspace.h>

/* ****************************************************************** */
/* *** Definições diversas ****************************************** */


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


/* ******************************************************************* */
/* *** Protótipos das funções **************************************** */

uint16_t get_volt_bat( void );


/* ******************************************************************* */
/* *** SETUP ********************************************************* */

void setup() {

    Serial.begin(115200);        // Inicialização da com. serial
    
    analogReference(INTERNAL);   // Referência dos ADCs (fundo de escala = 1.1V)
    
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

		// Envia tensão da bateria via serial
		Serial.print("Bateria: ");
		Serial.print( get_volt_bat() );
		Serial.println("mV");
    
    }
    // ******************************************************* //
}


/* ******************************************************************* */
/* *** FUNÇÕES (implementações) ************************************** */


uint16_t get_volt_bat( ) {

    uint16_t adc =  0;

    // Cada valor lido pelo ADC faz referência a 
    // 1/10 do valor de tensão real das baterias.
    for (int i = 0; i < 10; i++){
        adc += analogRead(VOLT_BAT);
        delayMicroseconds(100);
    }
    // Fundo de escala do ADC = 1.1V
    // Resolução do ADC = 10 bits (2^10)
    // Resultado em milivolts
    return (uint16_t)((adc*1.1/1023.0)*1000.0);

}

/* ****************************************************************** */
/* ****************************************************************** */
 
