# PetFood — Sistema de Alimentação Automática para Animais

## Sobre o projeto

O **PetFood** é um projeto acadêmico de Engenharia da Computação que propõe automatizar a alimentação de animais utilizando um ESP32, um servo motor e um sensor ultrassônico.

O sistema possui uma interface web desenvolvida com HTML, CSS e JavaScript, permitindo interagir com o alimentador por meio de um navegador.

## Objetivo

Facilitar a rotina de alimentação dos animais por meio da automação e do controle de liberação de ração.

## Componentes utilizados

- ESP32
- Servo motor
- Sensor ultrassônico HC-SR04
- Jumpers e alimentação elétrica adequada

## Ligações dos componentes

| Componente | Pino do ESP32 |
|---|---|
| Servo motor — sinal | GPIO 18 |
| HC-SR04 — TRIG | GPIO 23 |
| HC-SR04 — ECHO | GPIO 19, com divisor de tensão |

**Atenção:** o ESP32 utiliza sinais de 3,3 V. O pino ECHO do HC-SR04 pode fornecer 5 V, portanto utilize um divisor de tensão ou conversor de nível lógico. O servo deve receber alimentação adequada às suas especificações.

## Como executar

1. Instale a Arduino IDE.
2. Instale o suporte para placas ESP32.
3. Instale a biblioteca `ESP32Servo`.
4. Abra `PetFood_Completo_Arduino_IDE.ino`.
5. Selecione a placa e a porta serial corretas.
6. Compile e grave o código no ESP32.
7. Conecte seu celular ou computador à rede Wi-Fi `PetFood`, se o ponto de acesso estiver ativo.
8. Acesse `http://192.168.4.1` no navegador.

A senha e as configurações de rede devem ser confirmadas no código gravado no dispositivo.

## Funcionalidades

- Controle do servo motor pela interface web.
- Leitura do sensor ultrassônico.
- Interface web integrada ao ESP32.
- Recursos de agendamento e configuração de rede, conforme implementados no firmware.

## Contexto acadêmico

Projeto de Trabalho de Conclusão de Curso (TCC) em Engenharia da Computação, com foco em microcontroladores e automação da alimentação de animais.

## Segurança

- Teste o funcionamento do mecanismo antes de depender dele para alimentar o animal.
- Verifique se o servo não trava e se a liberação de ração funciona corretamente.
- Não publique senhas pessoais ou credenciais no repositório.

## Licença

A licença do projeto ainda não foi definida.
