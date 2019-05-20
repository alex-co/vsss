## Firmware do nó Base  

O nó chamado *base* é aquele que controla e gerencia a comunicação sem fio com os robôs, funcionando como um *gateway* entre o PC e a rede formada pelos robôs. O hardware é composto por um Arduino Pro Mini, um adaptador USB-Serial e uma interface sem fio baseada no CI nRF24L01. O Arduino comunica-se com o PC através da interface Serial e com o módulo sem fio através da interface SPI.


#### 1. Placa eletrônica do nó base:

<img src="https://github.com/alex-co/vsss/blob/master/img/base_placa_eletronica.png" width="400">

#### 2. Conexões de hardware mapeadas aos pinos do Arduino:

| Arduino | Função | | Arduino | Função |
| --- | --- | --- | --- | --- |
| A0 | Pino CE do módulo de rádio | | 4 | Led 1 - verde |
| A1 | Pino CS do módulo do rádio | | 5 | Led 1 - vermelho |
| A2 | Liga/Desl. módulo de rádio | | 7 | Led 2 - verde |
|    |                            | | 6 | Led 2 - vermelho |
|    |                            | | 8 | Led 3 - verde |
|    |                            | | 9 | Led 3 - vermelho |

#### 3. Links de interesse:

Datasheets:

| Microcontrolador | Conv. USB-Serial | Rádio |
| --- | --- | --- |
| [Atmega328](http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf "Atmega328") | [CP2102](https://www.silabs.com/documents/public/data-sheets/CP2102-9.pdf "CP2102") | [nRF24L01](https://infocenter.nordicsemi.com/pdf/nRF24L01P_PS_v1.0.pdf "nRF24L01") |

Bibliotecas:

* [RF24](http://tmrh20.github.io/RF24/classRF24.html): Responsável pela configuração do hardware e da comunicação do módulo de rádio com o Arduino.
* [RF24Network](http://tmrh20.github.io/RF24Network/classRF24Network.html): Responsável pela configuração e manutenção da rede formada pelos módulos de rádio.
