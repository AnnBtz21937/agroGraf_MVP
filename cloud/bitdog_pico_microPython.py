# MicroPython para Raspberry Pi Pico W (BitDogLab) - envio de dados para Google Cloud Function

import network
import urequests
import time
import machine
import json

# Configurações Wi-Fi
ssid = 'SEU_WIFI'
password = 'SENHA_WIFI'

def conecta_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        wlan.connect(ssid, password)
        while not wlan.isconnected():
            time.sleep(1)
    print('Conectado ao Wi-Fi:', wlan.ifconfig())

def envia_dados():
    url = 'https://SUA_CLOUD_FUNCTION_URL'
    dados = {
        "agua": 500,
        "umidade": 35,
        "energia": 2.3,
        "temperatura": 28,
        "pragas": 1
    }
    headers = {'Content-Type': 'application/json'}
    response = urequests.post(url, data=json.dumps(dados), headers=headers)
    print('Resposta:', response.text)
    response.close()

conecta_wifi()
while True:
    envia_dados()
    time.sleep(30)
