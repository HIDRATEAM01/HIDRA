#!/usr/bin/env python3
"""
Script de teste para ESP32 em PRODUÇÃO
Arquivo: test_esp_production.py

IMPORTANTE: Este script testa a conexão com o servidor em PRODUÇÃO
"""

import requests
import json
import time
import random
import sys
import ssl
from datetime import datetime
import argparse
import urllib3

# Desabilitar warnings SSL para teste (remover em produção real)
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

# ==================== CONFIGURAÇÕES DE PRODUÇÃO ====================
PRODUCTION_SERVERS = {
    'render': "https://hidra-eco.onrender.com",
    'domain': "https://hidra-eco.com.br",
    'www': "https://www.hidra-eco.com.br"
}

DEVICE_ID = "ESP32_PROD_001"

class Colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'

def print_colored(message, color=Colors.ENDC):
    print(f"{color}{message}{Colors.ENDC}")

def print_header(title):
    print_colored("=" * 70, Colors.HEADER)
    print_colored(f" {title} ", Colors.HEADER + Colors.BOLD)
    print_colored("=" * 70, Colors.HEADER)

def test_ssl_connection(server_url):
    """Testa conexão SSL"""
    print_colored(f"🔒 Testando SSL para {server_url}...", Colors.OKCYAN)
    
    try:
        response = requests.get(server_url, timeout=10, verify=True)
        print_colored(f"✅ SSL OK - Status: {response.status_code}", Colors.OKGREEN)
        return True
    except requests.exceptions.SSLError as e:
        print_colored(f"❌ Erro SSL: {e}", Colors.FAIL)
        return False
    except requests.exceptions.RequestException as e:
        print_colored(f"⚠️  Erro de conexão: {e}", Colors.WARNING)
        return False

def test_cors_preflight(endpoint_url):
    """Testa CORS preflight request"""
    print_colored("🌐 Testando CORS preflight...", Colors.OKCYAN)
    
    headers = {
        'Origin': 'https://esp32.local',
        'Access-Control-Request-Method': 'POST',
        'Access-Control-Request-Headers': 'Content-Type'
    }
    
    try:
        response = requests.options(endpoint_url, headers=headers, timeout=10)
        print_colored(f"📊 CORS Preflight Status: {response.status_code}", Colors.OKBLUE)
        
        cors_headers = {
            'Access-Control-Allow-Origin': response.headers.get('Access-Control-Allow-Origin'),
            'Access-Control-Allow-Methods': response.headers.get('Access-Control-Allow-Methods'),
            'Access-Control-Allow-Headers': response.headers.get('Access-Control-Allow-Headers'),
        }
        
        print_colored("🌐 Headers CORS recebidos:", Colors.OKBLUE)
        for header, value in cors_headers.items():
            print(f"   {header}: {value}")
        
        return response.status_code == 200
        
    except Exception as e:
        print_colored(f"❌ Erro no teste CORS: {e}", Colors.FAIL)
        return False

def test_production_endpoint(server_url, endpoint_path):
    """Testa endpoint específico em produção"""
    endpoint_url = f"{server_url}{endpoint_path}"
    
    print_colored(f"🧪 Testando endpoint: {endpoint_url}", Colors.OKCYAN)
    
    # Teste CORS primeiro
    test_cors_preflight(endpoint_url)
    
    # Dados de teste realistas
    test_data = {
        "device_id": DEVICE_ID,
        "coliformes": round(random.uniform(10, 150), 2),
        "ph": round(random.uniform(6.5, 8.5), 2),
        "dbo": round(random.uniform(1, 15), 2),
        "nt": round(random.uniform(0.5, 8), 2),
        "ft": round(random.uniform(0.05, 2), 2),
        "temperatura": round(random.uniform(20, 30), 2),
        "turbidez": round(random.uniform(5, 80), 2),
        "residuos": round(random.uniform(80, 500), 2),
        "od": round(random.uniform(4, 10), 2),
    }
    
    headers = {
        'Content-Type': 'application/json',
        'User-Agent': 'ESP32-Arduino/1.0',
        'Accept': 'application/json'
    }
    
    try:
        print_colored("📡 Enviando dados:", Colors.OKBLUE)
        for key, value in test_data.items():
            if key != 'device_id':
                print(f"   {key}: {value}")
        
        response = requests.post(
            endpoint_url,
            json=test_data,
            headers=headers,
            timeout=15,
            verify=True  # Verificar certificado SSL
        )
        
        print_colored(f"📊 Status Code: {response.status_code}", Colors.OKBLUE)
        print_colored(f"📋 Response Headers:", Colors.OKBLUE)
        
        important_headers = [
            'Content-Type', 'Access-Control-Allow-Origin',
            'Server', 'X-Frame-Options'
        ]
        
        for header in important_headers:
            value = response.headers.get(header, 'N/A')
            print(f"   {header}: {value}")
        
        if response.status_code == 200:
            try:
                response_data = response.json()
                print_colored("📋 Resposta JSON:", Colors.OKBLUE)
                print(json.dumps(response_data, indent=2))
                print_colored("✅ TESTE PASSOU!", Colors.OKGREEN)
                return True
            except json.JSONDecodeError:
                print_colored("⚠️  Resposta não é JSON válido", Colors.WARNING)
                print_colored(f"Resposta: {response.text[:200]}", Colors.WARNING)
        else:
            print_colored(f"❌ TESTE FALHOU - Status {response.status_code}", Colors.FAIL)
            print_colored(f"Resposta: {response.text[:200]}", Colors.FAIL)
        
        return False
        
    except requests.exceptions.SSLError as e:
        print_colored(f"❌ Erro SSL: {e}", Colors.FAIL)
        return False
    except requests.exceptions.Timeout:
        print_colored("❌ Timeout - servidor demorou para responder", Colors.FAIL)
        return False
    except requests.exceptions.ConnectionError:
        print_colored("❌ Erro de conexão - servidor inacessível", Colors.FAIL)
        return False
    except Exception as e:
        print_colored(f"❌ Erro inesperado: {e}", Colors.FAIL)
        return False

def test_all_servers():
    """Testa todos os servidores de produção"""
    print_header("TESTE DE PRODUÇÃO ESP32 → DJANGO")
    
    endpoints_to_test = [
        "/api/sensors/data/",
        "/api/sensors/heartbeat/",
    ]
    
    results = {}
    
    for server_name, server_url in PRODUCTION_SERVERS.items():
        print_colored(f"\n{'='*50}", Colors.HEADER)
        print_colored(f"🌐 TESTANDO SERVIDOR: {server_name.upper()}", Colors.HEADER)
        print_colored(f"🔗 URL: {server_url}", Colors.OKBLUE)
        print_colored(f"{'='*50}", Colors.HEADER)
        
        results[server_name] = {}
        
        # Teste SSL primeiro
        if not test_ssl_connection(server_url):
            print_colored(f"❌ Pulando testes para {server_name} - SSL falhou", Colors.FAIL)
            continue
        
        # Testa cada endpoint
        for endpoint in endpoints_to_test:
            print_colored(f"\n{'-'*40}", Colors.OKBLUE)
            results[server_name][endpoint] = test_production_endpoint(server_url, endpoint)
            time.sleep(2)  # Pausa entre testes
    
    # Resumo final
    print_header("RESUMO DOS TESTES")
    
    for server_name, endpoints in results.items():
        print_colored(f"\n🌐 {server_name.upper()}:", Colors.HEADER)
        
        if not endpoints:
            print_colored("   ❌ Não testado (SSL falhou)", Colors.FAIL)
            continue
        
        for endpoint, success in endpoints.items():
            status = "✅ PASSOU" if success else "❌ FALHOU"
            color = Colors.OKGREEN if success else Colors.FAIL
            print_colored(f"   {endpoint}: {status}", color)

def simulate_esp32_production():
    """Simula ESP32 enviando dados continuamente para produção"""
    print_header("SIMULAÇÃO CONTÍNUA ESP32 - PRODUÇÃO")
    
    # Usar o servidor principal
    server_url = PRODUCTION_SERVERS['domain']
    endpoint = f"{server_url}/api/sensors/data/"
    
    print_colored(f"🎯 Enviando dados para: {endpoint}", Colors.OKBLUE)
    print_colored("⏹️  Pressione Ctrl+C para parar", Colors.WARNING)
    
    cycle = 0
    successes = 0
    
    try:
        while True:
            cycle += 1
            current_time = datetime.now().strftime("%H:%M:%S")
            
            print_colored(f"\n--- Ciclo {cycle} - {current_time} ---", Colors.OKCYAN)
            
            if test_production_endpoint(server_url, "/api/sensors/data/"):
                successes += 1
            
            print_colored(f"📊 Taxa de sucesso: {successes}/{cycle} ({(successes/cycle)*100:.1f}%)", Colors.OKBLUE)
            print_colored("⏳ Aguardando 30 segundos...", Colors.WARNING)
            
            time.sleep(30)
    
    except KeyboardInterrupt:
        print_colored(f"\n⏹️  Simulação parada pelo usuário", Colors.WARNING)
        print_colored(f"📈 Resultado final: {successes}/{cycle} sucessos", Colors.OKGREEN)

def main():
    parser = argparse.ArgumentParser(description='Teste ESP32 → Django Produção')
    parser.add_argument('--simulate', action='store_true', help='Simulação contínua')
    parser.add_argument('--server', choices=['render', 'domain', 'www'], 
                       default='domain', help='Servidor para testar')
    
    args = parser.parse_args()
    
    if args.simulate:
        simulate_esp32_production()
    else:
        test_all_servers()

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print_colored("\n⏹️ Programa interrompido", Colors.WARNING)