/*
*    Exemplo referente a parte 2 de Atividades.txt
* 
*  - Escrita e leitura dos parâmetros da ponte H.
*  - Alteração dinâmica do valor de PWM máximo.
* 
*    Objetivos: Atuação dos motores em laço aberto.
*/
/* ****************************************************************** */

#include <avr/io.h>
#include <avr/pgmspace.h>


/* ****************************************************************** */
/* *** Definições diversas ****************************************** */

#define SSPEED        115200   // Velocidade da interface serial

#define PWM_MAX  0x9F   // PWM máx. p/ que os motores tenham aprox. 5v (Bat = 8.4v)
#define PWM_MIN  0x0F   // PWM mín. para garantir um movimento mínimo dos motores

// Transforma caractere ascii em um número de 4 bits em hexadecimal.
#define asc2hex(a) (((a) < 'a') ? ( (a) - '0' )  : ( ((a) - 'a')+10) )
    
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


typedef union {
    struct {
        uint8_t  pwm_motor_B;        // (bits 0-7)   8 bits: Valor do PWM do Motor B
        uint8_t  pwm_motor_A;        // (bits 8-15)  8 bits: Valor do PWM do Motor A
        uint8_t  dir_motor_B : 2,    // (bits 16-17) 2 bits: BIN1 e BIN2  da ponte H
                 dir_motor_A : 2,    // (bits 18-19) 2 bits: AIN1 e AIN2  da ponte H
                 ign1_4b     : 4;    // (bits 20-23) 4 bits não utilizados (padding)
        uint8_t  pwm_max;            // (bits 24-31) 8 bits: valor dinâmico p/ PWM máx.
    } config;
    uint32_t status = 0;             // Leitura/Escrita simuntânea do conjunto de variáveis.
} TMotCtrl;

/* ******************************************************************* */
/* *** Variáveis globais e instanciações ***************************** */


TasksTCtr tasks;   // Contagem de tempo para execução de tarefas

TMotCtrl  motor;   // Configuração dos motores (direção e PWM)

uint8_t  recv;
uint32_t buffer;

/* ******************************************************************* */
/* *** Protótipos das funções **************************************** */

void     set_motor_status( uint32_t );
uint32_t get_motor_status( void );
bool     is_motor_locked( uint8_t );
uint8_t  set_pwm_max( void );
uint16_t get_volt_bat( void );

/* ******************************************************************* */
/* *** SETUP ********************************************************* */

void setup() {

    Serial.begin(115200);      // Inicialização da com. serial
    
    analogReference(INTERNAL); // Referência dos ADCs (fundo de escala = 1.1V)
    
    // Inicialização dos pinos de controle da Ponte H
    pinMode(HBRID_EN, OUTPUT);    // Habilita ponte H
    digitalWrite(HBRID_EN, HIGH); // 
    
    pinMode(MTR_AIN1, OUTPUT);    // Bit 0 - Controle da ponte H do Motor A
    pinMode(MTR_AIN2, OUTPUT);    // Bit 1 - Controle da ponte H do Motor A
    pinMode(MTR_BIN1, OUTPUT);    // Bit 0 - Controle da ponte H do Motor B
    pinMode(MTR_BIN2, OUTPUT);    // Bit 1 - Controle da ponte H do Motor B
    pinMode(MTR_PWMA, OUTPUT);    // Sinal de PWM para controle  do Motor A
    pinMode(MTR_PWMB, OUTPUT);    // Sinal de PWM para controle  do Motor B
    
    motor.config.pwm_max = PWM_MAX; // PWM máximo para os motores
    set_motor_status( 0x0 );        // Motores parados e freio elétrico
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

        
        // Recebe string via interface serial com os 
        //   parâmetros dos motores.
        if( Serial.available() )
            buffer = 0;
        
        while( Serial.available() ){
        
            // Lê caracteres até receber 'quebra de linha'
            if( (recv = Serial.read()) != '\n' ){
                buffer  = buffer << 4;
                buffer |= asc2hex(recv);
            }
            else {
                // Se buffer completo => altera estado dos motores
                set_motor_status(buffer);
                
                // Envia estado do motor para interface serial
                Serial.print('PWM Máx.: ');
                Serial.println(motor.config.pwm_max,HEX);
                Serial.print('Dir MtrA: ');
                Serial.println(motor.config.dir_motor_A,BIN);
                Serial.print('PWM MtrA: ');
                Serial.println(motor.config.pwm_motor_A,HEX);
                Serial.print('Dir MtrB: ');
                Serial.println(motor.config.dir_motor_B,BIN);
                Serial.print('PWM MtrB: ');
                Serial.println(motor.config.pwm_motor_B,HEX);
            }
        }
        
        // *************************************************** //
        //  Valores para teste via Serial (em hexadecimal)
        //      02021F1F    Motor A = 10 31 | Motor B = 10 31
        //      01013F3F    Motor A = 01 63 | Motor B = 01 63
        //
        // *************************************************** //


    }
    
    // ******************************************************* //
    // Tarefas que devem ser executadas em intervalos de 1000ms
    if( (millis() - tasks.last_1000ms) > 1000 ){
        tasks.last_1000ms = millis();

        // Leitura da bateria
//        Serial.print("Bateria: ");
//        Serial.println(get_volt_bat());
        
        // Cálculo de PWM máximo
//        motor.config.pwm_max = set_pwm_max();
//        Serial.print("PWM max.: ");
//        Serial.println(motor.config.pwm_max);
    }
    // ******************************************************* //
}


/* ******************************************************************* */
/* *** FUNÇÕES (implementações) ************************************** */


/* *********************************************************************
 * Altera a configuração de direção e PWM dos motores
*/
void set_motor_status( uint32_t status ) {
    
    // Desabilita ponte H
    digitalWrite(HBRID_EN, LOW);
    
    // Altera configuração dos motores
    motor.status = status;
    // Direção do Motor A
    digitalWrite(MTR_AIN1, bitRead(motor.config.dir_motor_A, 0));
    digitalWrite(MTR_AIN2, bitRead(motor.config.dir_motor_A, 1));
    // Direção do Motor B
    digitalWrite(MTR_BIN1, bitRead(motor.config.dir_motor_B, 0));
    digitalWrite(MTR_BIN2, bitRead(motor.config.dir_motor_B, 1));
    
    // Ajusta PWM máx. caso o valor atual seja menor que
    //   o equivalente para as baterias com carga total.
    if( motor.config.pwm_max < PWM_MAX )
        motor.config.pwm_max = set_pwm_max();
    
    // Limita PWM máximo do motor A
    if( motor.config.pwm_motor_A > motor.config.pwm_max )
        motor.config.pwm_motor_A = motor.config.pwm_max;

    // Limita PWM máximo do motor B
    if( motor.config.pwm_motor_B > motor.config.pwm_max )
        motor.config.pwm_motor_B = motor.config.pwm_max;
        
    // Escreve bits de PWM na ponte H
    analogWrite(MTR_PWMA, motor.config.pwm_motor_A);
    analogWrite(MTR_PWMB, motor.config.pwm_motor_B);
    
    // Habilita ponte H
    digitalWrite(HBRID_EN, HIGH);
}

/* *********************************************************************
 * Leitura da configuração dos motores
*/
uint32_t get_motor_status() {
    
    return motor.status;
}

/* *********************************************************************
 * Informa se algum dos motores está com o freio elétrico ativado
 *   mtr = 0 => Motor A
 *   mtr = 1 => Motor B
 */
bool is_motor_locked( uint8_t mtr ) {
   
    uint8_t dir = (uint8_t)(motor.status >> 16);
    
    return mtr ? !(bitRead(dir,0) ^ bitRead(dir,1))
               : !(bitRead(dir,2) ^ bitRead(dir,3));
}

/* *********************************************************************
 * Ajusta o valor de PWM máximo baseado na da tensão da bateria para
 * que o motor trabalhe com 5 volts.
 */
uint8_t set_pwm_max() {
    
    return (uint8_t)((255.0 * 5.0 * 1000.0)/get_volt_bat());
}

/* *********************************************************************
 * Leitura da tensão da bateria
 */
uint16_t get_volt_bat() {

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
    return (uint16_t)((adc * 1.1/1023.0) * 1000.0);
}


/* ****************************************************************** */
/* ****************************************************************** */
 
