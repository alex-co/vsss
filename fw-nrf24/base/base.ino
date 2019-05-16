
#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

/* ****************************************************************** */
/* *** Definições diversas ****************************************** */


#define SSPEED      115200   // Velocidade da interface serial
#define SERIAL_ECHO      0   // Eco da msg recebida (Prod: 0 | Debug: 1) 

#define TAM_BUFFER      16   // Buffer de SW para o rádio
#define BASE_ADDRESS    00   // Base tem o endereço 0 (em octal)
#define NETW_CHANNEL   100   // Canal padrão de operação do rádio


/* ****************************************************************** */
/* Estas são as conexões de hardware mapeadas aos pinos do Arduino ** */

// Pinos de controle do módulo de rádio (independentes da SPI)

#define RADIO_CE    A0   // Pino CE do módulo de rádio
#define RADIO_CS    A1   // Pino CS do módulo do rádio
#define RADIO_PWR   A2   // Liga/Desl. módulo de rádio
#define RADIO_IRQ    2   // IRQ do módulo de rádio

// LEDs de sinalização. 
// Obs: Os LEDs vermelhos estão conectados a pinos com PWM.

#define L1GR         4   // Led 1 verde
#define L1RD         5   // Led 1 vermelho
#define L2GR         7   // Led 2 verde
#define L2RD         6   // Led 2 vermelho
#define L3GR         8   // Led 3 verde
#define L3RD         9   // Led 3 vermelho


/* ****************************************************************** */
/* Macros diversas que podem ser úteis ****************************** */

// Converte um número de 4 bits (hexadecimal) para caractere ascii. 
#define hex2asc(a) (( a < 10 )  ? ( (a) + 0x30 ) : ( (a) + ('a'-10) ))

// Converte um caractere ascii em um número de 4 bits (hexadecimal).
#define asc2hex(a) (((a) < 'a') ? ( (a) - '0' )  : ( ((a) - 'a')+10) )

// Lê um nibble de um inteiro de 32 bits dado seu índice (0 a 7).
#define Nibble(a, i) ((unsigned)(((a) >> (28 - 4 * (i))) & 0xF))

/* ****************************************************************** */
/* *** Definições de estruturas de dados **************************** */


typedef struct {
    uint8_t  robot;   // 1 byte  - Endereço do robô (destino) 
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
    uint32_t last_10ms  = 0;   // Controle do grupo das tarefas de 10ms
    uint32_t last_500ms = 0;   // Controle do grupo das tarefas de 500ms
} TasksTCtr;


/* ****************************************************************** */
/* *** Variáveis globais e instanciações **************************** */


RF24 radio(RADIO_CE, RADIO_CS);  // Instância do rádio
RF24Network network(radio);      // Instância da rede

uint8_t   netw_channel;          // Canal de rádio da rede
TRadioBuf rx_buffer, tx_buffer;  // Buffers de SW do rádio (Rx & Tx)
TasksTCtr tasks;                 // Para o controle de tempo das tarefas


/* ****************************************************************** */
/* *** Protótipos das funções *************************************** */
//
// Obs: Os protótipos não são necessários para compilação mas são úteis
//      como referência durante o processo de desenvolvimento.


//  Lê mensagens do buffer de HW e as armazena no buffer de SW
//  Retorna: Número de mensagens recebidas.
    uint8_t radio_rx( void );

//  Encaminha mensagens do buffer de SW para transmissão (rádio)
//  Retorna: Número de mensagens transmitidas.
    uint8_t radio_tx( void );

//  Lê um string recebido pela interface serial, transforma no padrão de
//  uma mensagem de rádio e a coloca no buffer de transmissão do rádio.
//  Retorna: Número de carateres recebidos.
    int8_t serial_rx( void );

//  Lê as mensagens armazenadas no buffer de recepção do rádio, as 
//  converte em string no formato CSV e as transmite pela int. serial.
//  Retorna: Número de carateres transmitidos.
    int8_t serial_tx( void );

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


/* ****************************************************************** */
/* *** SETUP ******************************************************** */

void setup() {

    pinMode(RADIO_PWR, OUTPUT);   // Liga alimentação do módulo de rádio
    digitalWrite(RADIO_PWR, HIGH);

    for( int i=4; i < 10; i++ )   // Define pinos dos LEDs como saída
        pinMode( i, OUTPUT );

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

    network.begin(BASE_ADDRESS);     // Inicialização do nó base na rede

    flush_radio_buffer(&rx_buffer);  // Inicialização dos buffers
    flush_radio_buffer(&tx_buffer);  //

}

/* ****************************************************************** */
/* *** LOOP PRINCIPAL *********************************************** */

void loop() {
    
    // Mantém a rede ativa
    network.update();
    
    // *****************************************************************
    // Tarefas a serem executadas em intervalos de 10ms
     if( (millis() - tasks.last_10ms) > 10 ){
        tasks.last_10ms = millis();
        
        radio_rx();     //  Rádio     =>  rx_buffer
        serial_tx();    //  rx_buffer =>  Serial
        serial_rx();    //  Serial    =>  tx_buffer
        radio_tx();     //  tx_buffer =>  Rádio

    }
    // *****************************************************************
    // Tarefas a serem executadas em intervalos de 500ms
    if( (millis() - tasks.last_500ms) > 500 ){
        tasks.last_500ms = millis();



    }

}

/* ****************************************************************** */
/* *** FUNÇÕES (implementações) ************************************* */


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
        msg.robot = header.from_node;
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
        RF24NetworkHeader header(msg.robot, msg.type);
        network.write(header, &msg.data, sizeof(msg.data));
        m++;
    }
    tx_buffer.last_t = millis();
    return m;   // Número de mensagens transmitidas.
}



/* *********************************************************************
 * Lê um string recebido pela interface serial, transforma no padrão
 * de uma mensagem de rádio e coloca no buffer de transmissão.
 * 
 * ATENÇÃO: 
 * No Monitor Serial, ative a opção "Nova-linha" para transmitir o '\n'
 * 
 * FORMATO DA MENSAGEM:
 * [A-Z];[0-3];[1 a 8 carateres de um número em hexadecimal]'\n'
 * 
 * [A-Z] => 1 caractere: Tipo da mensagem;
 * [0-3] => 1 caractere: Endereço do robô de destino;
 * [1 a 8 caracteres] => Dado a ser enviado;
 *  ;    => separador de campos da mensagem
 *  '\n' => sinaliza o final da mensagem.
 */
int8_t serial_rx( void ){

    if( !Serial.available() ) 
        return -1;
    
    uint8_t  h, i, c = '\0'; 
    uint32_t m;
    
    uint8_t str[TAM_BUFFER];      // Buffer de entrada
    memset(str,'\0',TAM_BUFFER);  // Limpeza do buffer

    // Lê o string enviado via interface serial
    i = 0;
    while( Serial.available() && i < (TAM_BUFFER-1) ){
        c = Serial.read();
        if( c != '\n' ) 
             str[i++] = c;
        else break;
        // Compensação pela baixa vel. da serial
        delayMicroseconds(100);  
    }
    
    // A mensagem deve ter tamanho entre 5 e 12 caracteres
    if( i < 5 || i > 12 ) 
        return -1;  // Erro: Tam. da msg fora do intervalo [5..12]

    TRadioMsg msg;
    
    if( c == '\n' ){    // O caractere '\n' sinaliza o fim de recepção
            
        // Processa o tipo da mensagem
        str[0] = toupper( str[0] );
        if( (str[0] >= 'A' && str[0] <= 'Z') && str[1] == ';' )
             msg.type = str[0];
        else return -1;  // Erro: Tipo da msg fora do intervalo [A..Z]
            
        // Processa o endereço de destino
        h = asc2hex( str[2] );
        if( (h >= 00 && h <= 03) && str[3] == ';' )
             msg.robot = h;
        else return -1; // Erro: Endereço fora do intervalo [00..03]

        // Processa o conteúdo da mensagem
        i = 4; m = 0;
        while( str[i] != '\0' ){
            m = m << 4;
            m |= ( asc2hex(tolower(str[i])) & 0xF );
            i++;
        }
        msg.data = m;

        // Retransmite msg recebida para a serial (eco)
        #if SERIAL_ECHO
        i = 0;
        while( str[i] != '\0' )
            Serial.write(str[i++]);
        Serial.write('\n');
        #endif

        // Escreve para o buffer de transmissão de rádio
        write_msg_radio_buffer(&tx_buffer, &msg);
    }
    return i;   // Número de carateres recebidos
}


/* *********************************************************************
 * Lê uma mensagem armazenada no buffer de recepção do rádio, a 
 * converte em string no formato CSV e transmite viarda int. serial.
 * 
 * FORMATO DO STRING:
 * [A-Z];[0-3];[1 a 8 carateres de um número em hexadecimal]'\n'
 * 
 * [A-Z] => 1 caractere: Tipo da mensagem;
 * [1-3] => 1 caractere: Endereço do robô de origem;
 * [1 a 8 caracteres] => Dado recebido;
 *  ;    => separador de campos da mensagem
 *  '\n' => quebra de linha sinaliza final da mensagem.
 */
int8_t serial_tx( void ){
    
    uint8_t i = 0;
    TRadioMsg msg;
    
    if( !is_radio_buffer_empty(&rx_buffer) ){
        
        read_msg_radio_buffer( &rx_buffer, &msg );
        
        i += Serial.print((char)msg.type);
        i += Serial.print(';');
        i += Serial.print(msg.robot);
        i += Serial.print(';');
        i += Serial.print(msg.data, HEX);
        i += Serial.print('\n');
    }
    return i;   // Número de caracteres transmitidos
}


/* *********************************************************************
 * Insere uma mensagem na fila do buffer de SW do rádio
 */
int8_t write_msg_radio_buffer( TRadioBuf *buf, TRadioMsg *msg ){

    if( buf->tam == TAM_BUFFER )    // Erro, buffer cheio
        return -1;
    
    buf->msg[buf->tail].robot = msg->robot;
    buf->msg[buf->tail].type  = msg->type;
    buf->msg[buf->tail].data  = msg->data;
    
    if( ++buf->tail == TAM_BUFFER ) 
        buf->tail = 0;
    buf->tam++;
    return ( TAM_BUFFER - buf->tam );   // Espaço restante
}


/* *********************************************************************
 * Lê e apaga uma mensagem da fila do buffer de SW de rádio
 */
int8_t read_msg_radio_buffer( TRadioBuf *buf, TRadioMsg *msg ){

    if( buf->tam == 0 )   // Erro, buffer vazio
        return -1;
    msg->robot = buf->msg[buf->head].robot;
    msg->type  = buf->msg[buf->head].type;
    msg->data  = buf->msg[buf->head].data;
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
