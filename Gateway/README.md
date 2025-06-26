# Projeto Hidra

Projeto Hidra - Monitoramento de corpos de água utilizando esp32 e comunicação via LoRa.

## Módulo

Este código é referente ao módulo de gateway, que vai operar como ponte entre os módulos de monitoramento e o servidor web.

Este módulo possui um pequeno sistema para configuração, que será possível se conectar ao utilizar a mesma rede do dispositvo. A _Interface backend-frontend_ define como as rotas serão tratadas.

## Interface backend-frontend

##### [GET] /

Tela principal, retorna arquivos do tipo `.js.tar.gz`, que são responsáveis por carregar a interface do sistema.

##### [GET] /config

Retorna parâmetros de configuração do dispositivo

```json
{
  "data": "22/04/2025",
  "hora": "23:59:31",
  "address": "0xaa"
}
```

##### [GET] /wifi/status

Retorna informações da conexão wi-fi, como nome da rede, ip do dispositivo e força do sinal

```json
{
  "ssid": "wifi-teste",
  "rssi": "-30db",
  "ip": "192.168.4.1"
}
```

##### [GET] /wifi/networks

Retorna lista de redes disponíveis e salvas

```json
{
  "saved": [
    { "id": 0, "ssid": "rede 1", "pass": "********" },
    { "id": 1, "ssid": "rede 2", "pass": "********" },
    { "id": 2, "ssid": "rede 3", "pass": "********" }
  ],
  "near": [
    { "id": 0, "ssid": "rede A", "rssi": "-80" },
    { "id": 0, "ssid": "rede B", "rssi": "-90" },
    { "id": 0, "ssid": "rede C", "rssi": "-96" }
  ]
}
```

##### [GET] /server/config

Retorna o status com os dados do webserver, conteúdo similar à:

```json
{
  "ssid": "hidra",
  "pass": "hidra1234",
  "ip": "192.168.4.1"
}
```

##### [GET] /modules

Retorna informação dos módulos conectados a este gateway

```json
{
  "modules": [
    {
      "id": 0,
      "name": "rio azul 2",
      "data-recebido": "2025-23-06",
      "hora-recebido": "14:19:00",
      "bat": 45
    },
    {
      "id": 1,
      "name": "rio mais verde",
      "data-recebido": "2025-23-06",
      "hora-recebido": "12:29:00",
      "bat": 36
    }
  ]
}
```

##### [GET] /modules/:id/data

Retorna últimas leituras de um módulo específico conectado a este gateway

```json
{
  "id": 0,
  "name": "rio azul 2",
  "data-recebido": "2025-23-06",
  "hora-recebido": "14:19:00",
  "data-modulo": "2025-23-06",
  "hora-modulo": "14:19:00",
  "bat": 45,
  "sensors": [
    {
      "id": 0,
      "type": "NTC",
      "value": 123.32
    },
    {
      "id": 1,
      "type": "COR",
      "value-1": 123,
      "value-2": 848,
      "value-3": 543,
      "value-4": 565
    },
    {
      "id": 2,
      "type": "SUN",
      "value": 3741
    }
  ]
}
```

##### [POST] /wifi/connect

Envia informações de conexão com rede wifi

```json
{
  "ssid": "22/04/2025",
  "pass": "23:59:31"
}
```

##### [POST] /config/time

Define uma nova configuração de relógio

```json
{
  "data": "22/04/2025",
  "hora": "23:59:31"
}
```

##### [POST] /server/config

Envia uma nova configuração de informações do servidor.

```json
{
  "ssid": "novo nome",
  "pass": "novasenha"
}
```

##### [POST] /modules/:id/config

Modifica parâmetros de um determinado módulo.

```json
{
  "id": 0,
  "oldname": "nome antigo para confirmar",
  "name": "novo nome"
}
```

##### [POST] /modules/new

Tenta se conectar a um novo módulo.

```json
{
  "name": "novo nome",
  "address": "0xab"
}
```

##### [DELETE] /wifi/:id

Remove uma rede da lista de redes salvas

```json
{
  "id": 1,
  "ssid": "nome para confirmar"
}
```

##### [DELETE] /modules/:id

Remove uma rede da lista de redes salvas

```json
{
  "id": 1,
  "name": "nome para confirmar"
}
```
