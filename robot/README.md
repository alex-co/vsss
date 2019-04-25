## Firmware do Robô  

O arquivo *robot.ino* é um modelo baseado em sketches do Arduino para as equipes desenvolverem seus firmwares.


#### 1. Placa eletrônica do robô:

<img src="https://github.com/alex-co/vsss/blob/master/img/robo_placa_eletronica.png" width="500">


#### 2. Conexões de hardware mapeadas aos pinos do Arduino:

| Arduino | Função | | Arduino | Função |
| --- | --- | --- | --- | --- |
| 4 | Led conectado a uma saída digital | | 6 | Habilita a ponte H (High) |
| 7 | Pino CE do módulo de rádio | | A2 | Bit 0 - Controle da ponte H do Motor A |
| 8 | Pino CS do módulo do rádio | | A3 | Bit 1 - Controle da ponte H do Motor A |
| 5 | Pino IRQ do módulo de rádio | | A1 | Bit 0 - Controle da ponte H do Motor B |
| A4 | Bit 0 do end. do rádio (LOW = ligado) | | A0 | Bit 1 - Controle da ponte H do Motor B |
| A5 | Bit 1 do end. do rádio (LOW = ligado) | | 9 | Sinal de PWM para controle  do Motor A |
| 2 | Pino de interrupção do Encoder A | | 10 | Sinal de PWM para controle  do Motor B |
| 3 | Pino de interrupção do Encoder B | | A7 | Leitura da tensão da bateria (Vcc/10)|


#### 3. Links de interesse:

Datasheets:

| Microcontrolador | Ponte H | Rádio |
| --- | --- | --- |
| [Atmega328](http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf "Atmega328") | [TB6612FNG](https://www.pololu.com/file/0J86/TB6612FNG.pdf "TB6612FNG") | [nRF24L01](https://infocenter.nordicsemi.com/pdf/nRF24L01P_PS_v1.0.pdf "nRF24L01") |

Bibliotecas:

* [RF24](http://tmrh20.github.io/RF24/classRF24.html): Responsável pela configuração do hardware e da comunicação do módulo de rádio com o Arduino.
* [RF24Network](http://tmrh20.github.io/RF24Network/classRF24Network.html): Responsável pela configuração e manutenção da rede formada pelos módulos de rádio.
