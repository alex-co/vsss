
#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

/* ****************************************************************** */
/* *** Definições diversas ****************************************** */


#define SSPEED    115200 // Velocidade da interface serial

#define TAM_BUFFER    16 // Buffer de SW para o rádio
#define BASE_ADDRESS  00 // Base tem o endereço 0 (em octal)
#define NETW_CHANNEL 100 // Canal padrão de operação do rádio

#define FULL_BAT    8000 // Valor de referência (em mV) p/ bat carregada
#define DEAD_BAT    6000 // Valor de referência (em mV) p/ bat esgotada

#define PWM_MIN     0x0F // PWM mín p/ garantir movimento das duas rodas
#define PWM_MAX     0x9F // PWM máx para que os motores tenham aprox. 5V

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
    uint8_t  type;    // 1 byte  - Tipo da mensagem
    uint32_t data;    // 4 bytes - Dado da mensagem
} TRadioMsg;


typedef struct {
    uint8_t   head;              // Índice do primeiro elemento da fila
    uint8_t   tail;              // Índice do último elemento da fila
    uint8_t   tam;               // Número de mensagens na fila
    uint32_t  last_t;            // Última transmissão ou recepção
    TRadioMsg msg[TAM_BUFFER];   // Fila de mensagens
} TRadioBuf;


typedef struct {
    uint32_t last_10ms   = 0; // Controle do grupo das tarefas de 10ms
    uint32_t last_100ms  = 0; // Controle do grupo das tarefas de 100ms
    uint32_t last_1000ms = 0; // Controle do grupo das tarefas de 1000ms
} TasksTCtr;


/* ******************************************************************* */
/* *** Variáveis globais e instanciações ***************************** */

RF24 radio(RADIO_CE, RADIO_CS);  // Instância do rádio
RF24Network network(radio);      // Instância da rede 

uint8_t   netw_channel;          // Canal de rádio da rede
uint16_t  base_node, this_node;  // End. do nó remoto (base) e deste nó
TRadioBuf rx_buffer, tx_buffer;  // Buffers para mensagens (Rx & Tx)
TasksTCtr tasks;                 // Para o controle de tempo das tarefas

/* ******************************************************************* */
/* *** Protótipos das funções **************************************** */
//
// Obs: Os protótipos não são necessários para compilação mas são úteis
//      como referência durante o processo de desenvolvimento.


//  Lê endereço de rede do dispositivo (robô) configurado via dip switch
//  Retorna: Endereço de rede do robô [00..03]
    uint8_t  get_node_addr( void );
    
//  Processa a primeira mensagem da fila do buffer de SW de entrada 
//  recebida do rádio e a encaminha seu tratador.
    void dispatch_msg( void );

//  Lê mensagens do buffer de HW e as armazena no buffer de SW
//  Retorna: Número de mensagens recebidas.
    uint8_t radio_rx( void );

//  Encaminha mensagens do buffer de SW para transmissão (rádio)
//  Retorna: Número de mensagens transmitidas.
    uint8_t radio_tx( void );

//  Insere uma mensagem na fila do buffer de SW de rádio
//  Retorna: Espaço livre.
    int8_t write_msg_radio_buffer( TRadioBuf&, TRadioMsg& );

//  Lê (e remove) uma mensagem da fila do buffer de SW de rádio
//  Retorna: Espaço ocupado.
    int8_t read_msg_radio_buffer( TRadioBuf&, TRadioMsg& );

//  (Re)Inicializa buffer de SW de rádio
//  Retorna: Nada
    void flush_radio_buffer( TRadioBuf& ); 

//  Verifica se o buffer de SW de rádio está cheio
//  Retorna: 1 => buffer cheio | 0 => caso contrário
    bool is_radio_buffer_full( TRadioBuf& );

//  Verifica se o buffer de SW de rádio está vazio
//  Retorna: 1 => buffer vazio | 0 => caso contrário
    bool is_radio_buffer_empty( TRadioBuf& );


/* ******************************************************************* */
/* *** SETUP ********************************************************* */

void setup() {

    pinMode(LED, OUTPUT);            // Inicialização do LED
    digitalWrite(LED, LOW);          
    
    base_node = BASE_ADDRESS;        // Endereço do nó base
    this_node = get_node_addr();     // Lê endereço deste robô

    Serial.begin(SSPEED);            // Inicializa a interface serial
    SPI.begin();                     // Inicializa a interface SPI
    radio.begin();                   // Inicializa o módulo de rádio 

    netw_channel = NETW_CHANNEL;     //
    radio.setChannel(netw_channel);  // Canal da rede de rádio (0-125);
    radio.setPALevel(RF24_PA_MAX);   // Potência de tx em 0dB (1mW)
//  radio.setDataRate(RF24_250KBPS); // O padrão é 1Mbps
    radio.setCRCLength(RF24_CRC_16); // Tamanho do CRC: 8 ou 16 bits
    radio.enableDynamicPayloads();   // Habilita msgs de tam dinâmico
    radio.setRetries(4,10);          // Reenvios (HW):4*250us=1ms;count:10
    radio.setAutoAck(true);          // AutoAck habilitado (feito em HW)
    radio.maskIRQ(1,1,0);            // Interrupção ao receber pacotes

    network.begin(this_node);        // Inicialização do robô na rede

    flush_radio_buffer(&rx_buffer);  // Inicialização dos buffers
    flush_radio_buffer(&tx_buffer);  //

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
        
    //  Função de gateway PC <=> robôs
        radio_rx();      //  Rádio     =>  rx_buffer
        dispatch_msg();  //  rx_buffer =>  tx_buffer
        radio_tx();      //  tx_buffer =>  Rádio

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


/* *********************************************************************
 * Lê endereço de rede do dispositivo (robô) configurado via dip switch
 */
uint8_t get_node_addr( void ){

    // Usar implementação da atividade 1.2

    return 01;
}


/* *********************************************************************
 * Processa a primeira mensagem da fila do buffer de SW de entrada 
 * recebida do rádio e a encaminha seu tratador.
 */
void dispatch_msg( void ){
    
    // Remove uma mensagem da fila de entrada (se existir) 
    //  e a coloca na fila de saída.
    
    TRadioMsg msg;
    read_msg_radio_buffer(&rx_buffer, &msg);
    write_msg_radio_buffer(&tx_buffer, &msg);
}


/* *********************************************************************
 * Lê mensagens do buffer de HW (rádio) e as armazena no buffer de SW
 */
uint8_t radio_rx( void ){

    uint8_t m = 0;
    TRadioMsg msg;
    RF24NetworkHeader header;
    
    while( network.available() && !is_radio_buffer_full(&rx_buffer) ) {

        network.read(header, &msg.data, sizeof(msg.data));
        msg.type  = header.type;
        write_msg_radio_buffer(&rx_buffer, &msg);
        m++;
    }
    rx_buffer.last_t = millis();
    return m;   // Número de mensagens recebidas.
}


/* *********************************************************************
 * Encaminha mensagens do buffer de SW para transmissão (rádio)
 */
uint8_t radio_tx( void ){

    uint8_t m = 0;
    TRadioMsg msg;
    
    while ( !is_radio_buffer_empty(&tx_buffer) ){

        read_msg_radio_buffer( &tx_buffer, &msg );
        RF24NetworkHeader header(base_node, msg.type);
        network.write(header, &msg.data, sizeof(msg.data));
        m++;
    }
    tx_buffer.last_t = millis();
    return m;   // Número de mensagens transmitidas.
}


/* *********************************************************************
 * Insere uma mensagem na fila do buffer de SW do rádio
 */
int8_t write_msg_radio_buffer( TRadioBuf *buf, TRadioMsg *msg ){

    if( is_radio_buffer_full(buf) )    // Erro, buffer cheio
        return -1;
    buf->msg[buf->tail].type = msg->type;
    buf->msg[buf->tail].data = msg->data;
    if( ++buf->tail == TAM_BUFFER ) 
        buf->tail = 0;
    buf->tam++;
    return ( TAM_BUFFER - buf->tam );  // Espaço restante
}


/* *********************************************************************
 * Lê e apaga uma mensagem da fila do buffer de SW de rádio
 */
int8_t read_msg_radio_buffer( TRadioBuf *buf, TRadioMsg *msg ){

    if( is_radio_buffer_empty(buf) )   // Erro, buffer vazio
        return -1;
    msg->type = buf->msg[buf->head].type;
    msg->data = buf->msg[buf->head].data;
    if( ++buf->head == TAM_BUFFER ) 
        buf->head = 0;
    buf->tam--;
    return buf->tam;    // Espaço ocupado
}


/* *********************************************************************
 * (Re)Inicializa buffer de SW de rádio
 */
void flush_radio_buffer( TRadioBuf *buf ) {

    buf->head   = 0;
    buf->tail   = 0;
    buf->tam    = 0;
    buf->last_t = 0;
}


/* *********************************************************************
 * Verifica se o buffer de SW de rádio está cheio
 */
bool is_radio_buffer_full( TRadioBuf *buf ){

    return buf->tam == TAM_BUFFER ? true : false ;
}


/* *********************************************************************
 * Verifica se o buffer de SW de rádio está vazio
 */
bool is_radio_buffer_empty( TRadioBuf *buf ){

    return buf->tam == 0 ? true : false ;
}


/* ****************************************************************** */
/* ****************************************************************** */
