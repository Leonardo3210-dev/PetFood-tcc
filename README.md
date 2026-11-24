# 🐾 PetFood — Sistema Inteligente de Alimentação Automática para Animais

<p align="center">
  <strong>Automatizando o cuidado e a alimentação dos nossos animais.</strong>
</p>

## 📌 Sobre o projeto

O **PetFood** é um sistema de alimentação automática para animais domésticos, desenvolvido como Trabalho de Conclusão de Curso (TCC) de Engenharia da Computação. O projeto utiliza um ESP32 para controlar a liberação de ração e uma interface web para acompanhamento e operação do equipamento.

## 🎯 Objetivo

Desenvolver um dispositivo automatizado que auxilie os tutores na alimentação de seus animais, permitindo programar refeições e realizar acionamentos manuais por uma página web.

## ⚙️ Funcionalidades

- Programação de horários de alimentação.
- Acionamento do mecanismo dispensador por servo motor.
- Monitoramento do nível de ração com sensor ultrassônico.
- Interface web acessível pelo celular ou computador.
- Alimentação manual pela interface.
- Comunicação local por Wi-Fi com o ESP32.

## 🖥️ Interface do sistema

A interface web do PetFood pode ser acessada quando o celular ou computador estiver conectado à rede Wi-Fi criada pelo equipamento. No protótipo demonstrado, o endereço local é `http://192.168.4.1`.

![Painel de controle do PetFood](imagens/interface-petfood.jpg)

*Figura 1 — Painel web do PetFood aberto em um celular, exibindo o nível de ração e o estado da máquina.*

## 🔧 Montagem eletrônica

O protótipo utiliza o ESP32 como unidade de controle, um sensor ultrassônico HC-SR04 para estimar o nível de ração e um servo motor para movimentar o mecanismo de liberação.

![Montagem eletrônica do PetFood](imagens/montagem-eletronica.jpg)

*Figura 2 — Montagem experimental dos componentes utilizados no protótipo.*

## 🔩 Componentes utilizados

| Componente | Função |
|---|---|
| ESP32 | Unidade principal de controle e comunicação Wi-Fi |
| Servo motor | Acionamento do mecanismo de liberação de ração |
| Sensor ultrassônico HC-SR04 | Medição da distância até a ração |
| Conversor lógico de nível | Adequação do sinal ECHO do sensor ao nível lógico do ESP32 |
| Protoboard e jumpers | Montagem e conexão do circuito |

## 🔌 Ligações do protótipo

| Componente | Pino do ESP32 |
|---|---|
| Sinal do servo motor | GPIO 18 |
| TRIG do HC-SR04 | GPIO 23 |
| ECHO do HC-SR04 | GPIO 19, passando pelo conversor lógico |

**Atenção:** o pino ECHO do HC-SR04 opera em 5 V. Utilize um conversor de nível lógico adequado antes de conectar esse sinal ao ESP32, cujos GPIOs trabalham em 3,3 V.

## 💻 Tecnologias utilizadas

- C/C++ para programação do ESP32.
- HTML para estrutura da interface.
- CSS para estilização.
- JavaScript para interatividade.
- Arduino IDE para compilação e gravação do firmware.
- Wi-Fi para comunicação local com a interface web.

## 🚀 Como acessar a interface

1. Ligue o protótipo PetFood.
2. No celular ou computador, procure a rede Wi-Fi criada pelo ESP32.
3. Conecte-se à rede do equipamento.
4. Abra o navegador.
5. Acesse `http://192.168.4.1`.

> O endereço acima é válido para o modo de ponto de acesso local configurado no firmware. O nome da rede e a senha dependem da configuração utilizada no código.

## 📂 Organização dos arquivos

```text
PetFood-tcc/
├── firmware/
│   └── PetFood.ino
├── imagens/
│   ├── interface-petfood.jpg
│   └── montagem-eletronica.jpg
├── index.html
├── style.css
├── script.js
└── README.md
```

*Ajuste os nomes e as pastas desta estrutura para corresponder aos arquivos realmente presentes no repositório.*

## 🎓 Desenvolvimento acadêmico

- **Instituição:** Centro Universitário CEUNI FAMETRO
- **Curso:** Engenharia da Computação
- **Projeto:** PetFood — Sistema Microcontrolador de Alimentação Automática para Animais
- **Orientadora:** Luana Leal

**Desenvolvedores**
- Leonardo Felipe dos Santos da Silva
- Filipe Sales Borges Figueiredo

## 🔮 Trabalhos futuros

- Aprimorar o monitoramento remoto.
- Adicionar notificações sobre o nível de ração.
- Expandir as opções de configuração das refeições.
- Melhorar a estrutura física do dispensador.
- Implementar novas funcionalidades de automação.

## 📄 Licença

Projeto desenvolvido para fins acadêmicos e educacionais como Trabalho de Conclusão de Curso de Engenharia da Computação.

---

<p align="center">
  🐾 <strong>PetFood — Tecnologia a serviço do cuidado animal.</strong>
</p>
