# ESP-IDF Sonar de envio customizado de sinal 
Este repositório contém o projeto de firmware usado para gerar e receber um sinal analógico proveniente de transdutores de ultrassom. A comunicação com o microcontrolador é feita através de um API em python, disponível em:

```
github.com/GabrielKis/ultrasonic_recv_emiter_python_api
```

## Utilização

Para gravar o Firmware basta seguir as diretrizes da esp-idf, uma vez que este foi desenvolvido nela. Um guia completo de como preparar o ambiente e gravar o firmware está em:

```
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html
```

A placa utilizada contém conversores A-D e D-A. O esquemático que corresponde às ligações se encontra no arquivo `esquematico_circuito.jpeg`, neste mesmo repositório.
