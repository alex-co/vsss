/*
*	Exemplo referente a questão 1.5 de Atividades.txt
* 
*  - Contagem de interrupções na barreira óptica dos encoders.
*  - O LED da placa sinaliza o estado dos encoders.
* 
*    Objetivo: Detecção de eventos usando interrupção externa.
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

uint16_t count_enc_a = 0;
uint16_t count_enc_b = 0;
uint16_t prev_enc_a = 0;
uint16_t prev_enc_b = 0;

/* ******************************************************************* */
/* *** Protótipos das funções **************************************** */

void task_irq_encoder_a( void );
void task_irq_encoder_b( void );


/* ******************************************************************* */
/* *** SETUP ********************************************************* */

void setup() {

    Serial.begin(115200); // Inicialização da com. serial
    
    pinMode(LED, OUTPUT); // Pino do LED como saída digital
    
    // Habilita int para o Encoder A no pino 2 do Arduino
    pinMode(IRQ_ENC_A, INPUT_PULLUP);
    attachInterrupt(0, task_irq_encoder_a, FALLING);

    // Habilita int para o Encoder B no pino 3 do Arduino
    pinMode(IRQ_ENC_B, INPUT_PULLUP);
    attachInterrupt(1, task_irq_encoder_b, FALLING); 
    
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

        if( digitalRead(IRQ_ENC_A) || digitalRead(IRQ_ENC_B))
             digitalWrite(LED, HIGH);
        else digitalWrite(LED, LOW);

        if( count_enc_a > prev_enc_a ){
            prev_enc_a = count_enc_a;
            Serial.print("Enc. A: ");
            Serial.println(count_enc_a);
        }
        
        if( count_enc_b > prev_enc_b ){
            prev_enc_b = count_enc_b;
            Serial.print("Enc. B: ");
            Serial.println(count_enc_b);
        }

    }
    // ******************************************************* //
    // Tarefas que devem ser executadas em intervalos de 1000ms
    if( (millis() - tasks.last_1000ms) > 1000 ){
        tasks.last_1000ms = millis();


    
    }
    // ******************************************************* //
}


/* ******************************************************************* */
/* *** FUNÇÕES (implementações) ************************************** */

void task_irq_encoder_a(){
    
    noInterrupts();
    count_enc_a++;
    interrupts();
}

void task_irq_encoder_b(){
    
    noInterrupts();
    count_enc_b++;
    interrupts();
}

/* ****************************************************************** */
/* ****************************************************************** */
 
