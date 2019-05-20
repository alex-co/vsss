## Suporte à comunicação via rádio

Nas pastas **base** e **robo** encontram-se modelos para os firmwares do nó base e dos robôs 
onde foram adicionados suporte à comunicação bilateral via rádio através do transceiver 
nRF24L01, bem como o gerenciamento de filas de recepção e transmissão em software.

A implementação do *nó base* conta com a funcionalidade de um gateway transparente 
entre o PC e os robôs, ou seja, toda comunicação entre PC e robôs passa pelo nó base sem 
que haja processamento do conteúdo da mensagem.

A implementação do *robô* faz somente a função de *eco*. Mais especificamente, toda mensagem 
recebida via rádio é armazenada na fila de entrada, posteriormente colocada na fila de saída e 
retransmitida ao nó base.

O *nó base* tem endereço de rádio "0" enquantos os robôs 1, 2 e 3 têm respectivamente os endereços 
de rádio "1", "2" e "3", estes setados fisicamente via *dipswitch* em suas placas eletrônicas. 
Uma representação da comunicação entre PC, nó base e robôs pode ser vista na figura abaixo.

<img src="https://github.com/alex-co/vsss/blob/master/img/pc-base-robos.png" width="450">

### Bibliotecas:

No modelo fornecido para a programação do nó *base* e dos nós *robôs* usa duas bibliotecas 
relacionadas ao módulo de rádio:
- **RF24**: Responsável pela configuração do hardware e da comunicação do módulo de rádio com o Arduino.  
- **RF24Network**: Responsável pela configuração e manutenção da rede formada pelos módulos de rádio.  

Estas bibliotecas por padrão não se encontram na instalação da IDE do Arduino. A partir da versão 1.6.3 a 
IDE do Arduino conta com a funcionalidade embarcada de instalação de bibliotecas, sendo que estas podem ser 
encontradas nos repositórios já configurados por padrão na IDE. 

**Instalação via gerenciador de bibliotecas da IDE do Arduino:**  

1. Na IDE do Arduino, acesse o gerenciador de bibliotecas através do menu:  
*Sketch* > *Incluir Biblioteca* > *Gerenciar Bibliotecas*  

2. No campo apropriado, faça a busca por *RF24*  

3. As bibliotecas que deverão ser instaladas são *RF24 by TMRh20* e *RF24Network by TMRh20*.

- <a href="http://tmrh20.github.io/RF24/" target="_blank">Documentação</a> da Classe <a href="http://tmrh20.github.io/RF24/classRF24.html" target="_blank">RF24</a> e seu <a href="https://github.com/nRF24/RF24" target="_blank">código fonte</a>.
- <a href="http://tmrh20.github.io/RF24Network/" target="_blank">Documentação</a> da Classe <a href="http://tmrh20.github.io/RF24Network/classRF24Network.html" target="_blank">RF24Network</a> e seu <a href="https://github.com/nRF24/RF24Network" target="_blank">código fonte</a>.

### Formato das mensagens:

As mensagens transmitidas/recebidas pelo PC devem ter um formato específico para que sejam tratadas
pelo *nó base* e transformadas em mensagens de rádio. As mensagens enviadas pelos robôs, quando
têm como destino o PC são transmitidas pelo nó base no mesmo formato.

O string que representa a mensagem deve ser codificado em ASCII e ter o seguinte formato:

[A-Z] ; [0-3] ; [1 a 8 carateres (hexadecimal)] '\n'
 
|     |      |
| --- | ---  |
| [ A-Z ] | Tipo da mensagem |
| [ 0-3 ] | Endereço de rádio (destino ou origem) |
| [ 1 a 8 caracteres ] | Dado da mensagem (em hexadecimal) |
| ;  | Separador de campos da mensagem |
| '\n' | Sinalizador do final da mensagem |

Obs: No *Monitor Serial* da IDE do Arduino ative a opção "Nova-linha" para que o caractere '\n' seja transmitido.
