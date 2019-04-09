## Firmware do nó Base  

O nó chamado *base* é aquele que controla e gerencia a comunicação sem fio com os robôs, funcionando como um *gateway* entre o PC e a rede formada pelos robôs. O hardware é composto por um Arduino Pro Mini, um adaptador USB-Serial e uma interface sem fio baseada no CI nRF24L01. O Arduino comunica-se com o PC através da interface Serial e com o módulo sem fio através da interface SPI.


### **Circuito eletrônico do nó base**

<img src="https://github.com/alex-co/vsss/blob/master/img/base_placa_eletronica.png" width="400">
