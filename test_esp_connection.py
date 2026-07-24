#!/usr/bin/env python3
"""
Script de teste para simular ESP32 enviando dados para Django
Arquivo: test_esp_connection.py

Como usar:
1. Salve este arquivo na pasta raiz do seu projeto (mesmo nível que manage.py)
2. Certifique-se que o Django está rodando: python manage.py runserver
3. Execute: python test_esp_connection.py

Autor: Sistema IoT Hidra
Data: 2024
"""

import requests
import json
import time
import random
import sys
from datetime import datetime
import argparse

# ==================== CONFIGURAÇÕES ====================
# ALTERE ESTAS CONFIGURAÇÕES CONFORME SEU AMBIENTE
SERVER_URL = "http://127.0.0.1:8000"  # URL do seu servidor Django
DEVICE_ID = "ESP32_TEST_001"

# Endpoints
SENSOR_ENDPOINT = f"{SERVER_URL}/api/sensors/data/"
HEARTBEAT_ENDPOINT = f"{SERVER_URL}/api/sensors/heartbeat/"
DASHBOARD_API = f"{SERVER_URL}/api/dashboard/"

# ==================== CORES PARA TERMINAL ====================
class Colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def print_colored(message, color=Colors.ENDC):
    """Imprime mensagem colorida"""
    print(f"{color}{message}{Colors.ENDC}")

def print_header(title):
    """Imprime cabeçalho formatado"""
    print_colored("=" * 60, Colors.HEADER)
    print_colored(f" {title} ", Colors.HEADER + Colors.BOLD)
    print_colored("=" * 60, Colors.HEADER)

# ==================== GERAÇÃO DE DADOS ====================

def generate_realistic_sensor_data():
    """Gera dados realistas dos sensores para teste"""
    return {
        "coliformes": round(random.uniform(10, 150), 2),    # NMP/100mL
        "ph": round(random.uniform(6.0, 9.0), 2),           # pH
        "dbo": round(random.uniform(1, 15), 2),             # mg/L
        "nt": round(random.uniform(0.5, 8), 2),             # mg/L Nitrogênio Total
        "ft": round(random.uniform(0.05, 2), 2),            # mg/L Fósforo Total
        "temperatura": round(random.uniform(18, 32), 2),     # °C
        "turbidez": round(random.uniform(5, 80), 2),        # NTU
        "residuos": round(random.uniform(80, 500), 2),      # mg/L
        "od": round(random.uniform(2, 10), 2),              # mg/L Oxigênio Dissolvido
        "device_id": DEVICE_ID
    }

def generate_problematic_data():
    """Gera dados que devem disparar alertas"""
    return {
        "coliformes": 250,      # Alto
        "ph": 5.2,              # Muito ácido
        "dbo": 20,              # Alto
        "nt": 12,               # Alto
        "ft": 3.5,              # Alto
        "temperatura": 38,       # Alto
        "turbidez": 95,         # Muito alto
        "residuos": 600,        # Muito alto
        "od": 1.5,              # Muito baixo
        "device_id": DEVICE_ID
    }

# ==================== FUNÇÕES DE TESTE ====================

def check_server_status():
    """Verifica se o servidor Django está rodando"""
    print_colored("🔍 Verificando status do servidor...", Colors.OKCYAN)
    
    try:
        response = requests.get(SERVER_URL, timeout=5)
        print_colored(f"✅ Servidor respondendo: {SERVER_URL}", Colors.OKGREEN)
        print_colored(f"📊 Status Code: {response.status_code}", Colors.OKBLUE)
        return True
    except requests.exceptions.ConnectionError:
        print_colored(f"❌ Servidor não acessível em {SERVER_URL}", Colors.FAIL)
        print_colored("💡 Certifique-se que o Django está rodando:", Colors.WARNING)
        print_colored("   python manage.py runserver", Colors.WARNING)
        return False
    except Exception as e:
        print_colored(f"❌ Erro ao conectar: {e}", Colors.FAIL)
        return False

def test_sensor_endpoint(data=None, test_name="Dados dos sensores"):
    """Testa o endpoint de dados dos sensores"""
    print_colored(f"🧪 Testando: {test_name}", Colors.OKCYAN)
    
    if data is None:
        data = generate_realistic_sensor_data()
    
    try:
        print_colored("📡 Enviando dados:", Colors.OKBLUE)
        for key, value in data.items():
            if key != 'device_id':
                print(f"   {key}: {value}")
        
        response = requests.post(
            SENSOR_ENDPOINT,
            json=data,
            headers={'Content-Type': 'application/json'},
            timeout=10
        )
        
        print_colored(f"📊 Status Code: {response.status_code}", Colors.OKBLUE)
        
        if response.status_code == 200:
            response_data = response.json()
            print_colored("📋 Resposta do servidor:", Colors.OKBLUE)
            print(json.dumps(response_data, indent=2))
            print_colored(f"✅ {test_name}: SUCESSO", Colors.OKGREEN)
            return True
        else:
            print_colored("📋 Resposta de erro:", Colors.WARNING)
            print(response.text)
            print_colored(f"❌ {test_name}: FALHA", Colors.FAIL)
            return False
            
    except requests.exceptions.RequestException as e:
        print_colored(f"❌ Erro de conexão: {e}", Colors.FAIL)
        return False

def test_heartbeat():
    """Testa o endpoint de heartbeat"""
    print_colored("💓 Testando heartbeat...", Colors.OKCYAN)
    
    data = {
        "device_id": DEVICE_ID,
        "status": "online",
        "uptime": random.randint(1000, 300000)
    }
    
    try:
        response = requests.post(
            HEARTBEAT_ENDPOINT,
            json=data,
            headers={'Content-Type': 'application/json'},
            timeout=10
        )
        
        print_colored(f"💓 Heartbeat data: {data}", Colors.OKBLUE)
        print_colored(f"📊 Status Code: {response.status_code}", Colors.OKBLUE)
        
        if response.status_code == 200:
            print_colored("📋 Resposta:", Colors.OKBLUE)
            print(response.text)
            print_colored("✅ Heartbeat: SUCESSO", Colors.OKGREEN)
            return True
        else:
            print_colored(f"❌ Heartbeat: FALHA - {response.text}", Colors.FAIL)
            return False
            
    except requests.exceptions.RequestException as e:
        print_colored(f"❌ Erro no heartbeat: {e}", Colors.FAIL)
        return False

def test_dashboard_api():
    """Testa a API do dashboard"""
    print_colored("📊 Testando API do dashboard...", Colors.OKCYAN)
    
    try:
        response = requests.get(DASHBOARD_API, timeout=10)
        
        print_colored(f"📊 Status Code: {response.status_code}", Colors.OKBLUE)
        
        if response.status_code == 200:
            data = response.json()
            print_colored("📋 Dados do dashboard:", Colors.OKBLUE)
            
            if 'iqa' in data:
                print(f"   IQA: {data['iqa']['valor']:.2f} - {data['iqa']['classificacao']}")
            
            if 'sensor_data' in data:
                print("   Sensores:")
                for sensor, value in data['sensor_data'].items():
                    print(f"     {sensor}: {value}")
            
            print_colored("✅ Dashboard API: SUCESSO", Colors.OKGREEN)
            return True
        else:
            print_colored(f"❌ Dashboard API: FALHA - {response.text}", Colors.FAIL)
            return False
            
    except requests.exceptions.RequestException as e:
        print_colored(f"❌ Erro na API do dashboard: {e}", Colors.FAIL)
        return False

def test_invalid_data():
    """Testa com dados inválidos"""
    invalid_tests = [
        {
            "name": "pH inválido (>14)",
            "data": {
                **generate_realistic_sensor_data(),
                "ph": 15.5
            }
        },
        {
            "name": "Temperatura extrema",
            "data": {
                **generate_realistic_sensor_data(),
                "temperatura": -100
            }
        },
        {
            "name": "Dados incompletos",
            "data": {
                "ph": 7.5,
                "temperatura": 25,
                "device_id": DEVICE_ID
                # Faltam campos obrigatórios
            }
        },
        {
            "name": "JSON inválido",
            "data": "not_a_json"
        }
    ]
    
    success_count = 0
    
    for test in invalid_tests:
        print_colored(f"\n🚨 Teste de validação: {test['name']}", Colors.WARNING)
        
        try:
            if isinstance(test['data'], str):
                response = requests.post(
                    SENSOR_ENDPOINT,
                    data=test['data'],  # String inválida em vez de JSON
                    headers={'Content-Type': 'application/json'},
                    timeout=10
                )
            else:
                response = requests.post(
                    SENSOR_ENDPOINT,
                    json=test['data'],
                    headers={'Content-Type': 'application/json'},
                    timeout=10
                )
            
            if response.status_code == 400:
                print_colored(f"✅ Validação funcionando: dados rejeitados corretamente", Colors.OKGREEN)
                print_colored(f"   Resposta: {response.text[:100]}...", Colors.OKBLUE)
                success_count += 1
            else:
                print_colored(f"⚠️  Validação pode ter problema: Status {response.status_code}", Colors.WARNING)
                
        except Exception as e:
            print_colored(f"❌ Erro no teste: {e}", Colors.FAIL)
    
    return success_count

def continuous_simulation(duration=60, interval=10):
    """Simulação contínua do ESP32"""
    print_colored(f"🔄 Iniciando simulação contínua por {duration} segundos (envios a cada {interval}s)", Colors.HEADER)
    
    start_time = time.time()
    cycle_count = 0
    success_count = 0
    
    try:
        while time.time() - start_time < duration:
            cycle_count += 1
            current_time = datetime.now().strftime("%H:%M:%S")
            
            print_colored(f"\n--- Ciclo {cycle_count} - {current_time} ---", Colors.OKCYAN)
            
            # Alternar entre dados normais e problemáticos
            if cycle_count % 5 == 0:  # A cada 5 ciclos, enviar dados problemáticos
                data = generate_problematic_data()
                test_name = "Dados problemáticos (alertas)"
            else:
                data = generate_realistic_sensor_data()
                test_name = "Dados normais"
            
            if test_sensor_endpoint(data, test_name):
                success_count += 1
            
            # Heartbeat
            test_heartbeat()
            
            print_colored(f"⏰ Aguardando {interval} segundos...", Colors.OKBLUE)
            time.sleep(interval)
    
    except KeyboardInterrupt:
        print_colored("\n⏹️  Simulação interrompida pelo usuário", Colors.WARNING)
    
    print_colored(f"\n📈 Resultado da simulação:", Colors.HEADER)
    print_colored(f"✅ Ciclos executados: {cycle_count}", Colors.OKGREEN)
    print_colored(f"✅ Envios bem-sucedidos: {success_count}", Colors.OKGREEN)
    print_colored(f"❌ Falhas: {cycle_count - success_count}", Colors.FAIL)
    
    if cycle_count > 0:
        success_rate = (success_count / cycle_count) * 100
        print_colored(f"📊 Taxa de sucesso: {success_rate:.1f}%", Colors.OKBLUE)

# ==================== FUNÇÃO PRINCIPAL ====================

def run_basic_tests():
    """Executa testes básicos"""
    print_header("TESTES BÁSICOS ESP32 ↔ DJANGO")
    
    if not check_server_status():
        return False
    
    tests = [
        ("Dados válidos", lambda: test_sensor_endpoint()),
        ("Heartbeat", test_heartbeat),
        ("Dashboard API", test_dashboard_api),
        ("Dados problemáticos", lambda: test_sensor_endpoint(generate_problematic_data(), "Dados com alertas")),
    ]
    
    results = []
    
    for test_name, test_func in tests:
        print_colored(f"\n{'─' * 50}", Colors.OKBLUE)
        result = test_func()
        results.append((test_name, result))
    
    # Teste de validação
    print_colored(f"\n{'─' * 50}", Colors.OKBLUE)
    validation_successes = test_invalid_data()
    results.append(("Validações", validation_successes > 0))
    
    # Resumo
    print_header("RESUMO DOS TESTES")
    
    passed = 0
    for test_name, result in results:
        status = "✅ PASSOU" if result else "❌ FALHOU"
        color = Colors.OKGREEN if result else Colors.FAIL
        print_colored(f"{test_name}: {status}", color)
        if result:
            passed += 1
    
    total = len(results)
    print_colored(f"\n📊 Resultado Final: {passed}/{total} testes passaram", Colors.HEADER)
    
    if passed == total:
        print_colored("🎉 Todos os testes passaram! Sistema funcionando.", Colors.OKGREEN)
        return True
    else:
        print_colored("⚠️  Alguns testes falharam. Verifique as configurações.", Colors.WARNING)
        return False

def main():
    """Função principal com menu interativo"""
    parser = argparse.ArgumentParser(description='Teste de conexão ESP32 ↔ Django')
    parser.add_argument('--auto', action='store_true', help='Executar testes automaticamente')
    parser.add_argument('--continuous', type=int, help='Simulação contínua por X segundos')
    parser.add_argument('--server', default="http://127.0.0.1:8000", help='URL do servidor Django')
    
    args = parser.parse_args()
    


    SERVER_URL = args.server
    SENSOR_ENDPOINT = f"{SERVER_URL}/api/sensors/data/"
    HEARTBEAT_ENDPOINT = f"{SERVER_URL}/api/sensors/heartbeat/"
    DASHBOARD_API = f"{SERVER_URL}/api/dashboard/"
    
    print_header("🧪 SISTEMA DE TESTE ESP32 ↔ DJANGO")
    print_colored(f"🌐 Servidor: {SERVER_URL}", Colors.OKBLUE)
    print_colored(f"🔧 Device ID: {DEVICE_ID}", Colors.OKBLUE)
    
    if args.continuous:
        continuous_simulation(duration=args.continuous)
        return
    
    if args.auto:
        run_basic_tests()
        return
    
    # Menu interativo
    while True:
        print_colored("\n" + "═" * 50, Colors.HEADER)
        print_colored("📋 MENU DE TESTES", Colors.HEADER + Colors.BOLD)
        print_colored("═" * 50, Colors.HEADER)
        print("1. 🧪 Executar testes básicos")
        print("2. 📡 Teste único de sensor")
        print("3. 💓 Teste de heartbeat")
        print("4. 📊 Teste da API do dashboard")
        print("5. 🚨 Teste de validação")
        print("6. 🔄 Simulação contínua")
        print("7. 📈 Enviar dados problemáticos")
        print("0. ❌ Sair")
        
        try:
            choice = input("\n🔵 Escolha uma opção: ").strip()
            
            if choice == '0':
                print_colored("👋 Saindo...", Colors.OKCYAN)
                break
            elif choice == '1':
                run_basic_tests()
            elif choice == '2':
                test_sensor_endpoint()
            elif choice == '3':
                test_heartbeat()
            elif choice == '4':
                test_dashboard_api()
            elif choice == '5':
                test_invalid_data()
            elif choice == '6':
                try:
                    duration = int(input("Duração em segundos (padrão 60): ") or "60")
                    interval = int(input("Intervalo entre envios (padrão 10): ") or "10")
                    continuous_simulation(duration, interval)
                except ValueError:
                    print_colored("❌ Valores inválidos", Colors.FAIL)
            elif choice == '7':
                test_sensor_endpoint(generate_problematic_data(), "Dados problemáticos")
            else:
                print_colored("❌ Opção inválida", Colors.FAIL)
                
        except KeyboardInterrupt:
            print_colored("\n\n👋 Saindo...", Colors.OKCYAN)
            break
        except Exception as e:
            print_colored(f"❌ Erro: {e}", Colors.FAIL)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print_colored("\n\n⏹️ Programa interrompido", Colors.WARNING)
    except Exception as e:
        print_colored(f"\n❌ Erro inesperado: {e}", Colors.FAIL)
        import traceback
        traceback.print_exc()