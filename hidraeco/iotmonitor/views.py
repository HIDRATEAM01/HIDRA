from django.shortcuts import render, redirect
from django.http import HttpResponse, HttpResponseBadRequest, HttpResponseForbidden
from django.utils import timezone
from django.contrib.auth import authenticate, login as auth_login, logout as auth_logout
from django.contrib.auth.models import User
from django.contrib import messages
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
from django.contrib.auth.decorators import login_required
from django.core.validators import validate_email
from django.core.exceptions import ValidationError
from django.core.cache import cache
import json
import logging
from django.contrib.auth.decorators import login_required
from django.views.decorators.http import require_http_methods
from django.views.decorators.csrf import csrf_exempt, csrf_protect
from .utils import send_password_reset_email, validate_password_reset_token
from django.conf import settings
from .iqa_calculator import IQACalculator
from django.utils import timezone

# Configurar logging
logger = logging.getLogger(__name__)

# Logger específico para dados dos sensores
sensor_logger = logging.getLogger('sensors')

# Logger para alertas
alert_logger = logging.getLogger('alerts')

# Logger para IQA
iqa_logger = logging.getLogger('iqa')

# Logger para conexões ESP
esp_logger = logging.getLogger('esp_connection')


def homepage(request):
    return render(request, 'iotmonitor/homepage.html')


def login(request):
    """
    View para autenticação de usuários
    """
    if request.user.is_authenticated:
        return redirect('dashboard')

    if request.method == 'POST':
        email = request.POST.get('email', '').strip()
        password = request.POST.get('password', '')
        remember_me = request.POST.get('remember_me')

        # Validações básicas
        if not email or not password:
            messages.error(request, 'Email e senha são obrigatórios.')
            return render(request, 'iotmonitor/login.html')

        try:
            # Validar formato do email
            validate_email(email)
        except ValidationError:
            messages.error(request, 'Digite um email válido.')
            return render(request, 'iotmonitor/login.html')

        try:
            # Buscar usuário pelo email
            user_obj = User.objects.get(email=email)
            username = user_obj.username
        except User.DoesNotExist:
            messages.error(request, 'Email ou senha incorretos.')
            logger.warning(
                f'Tentativa de login com email inexistente: {email}')
            return render(request, 'iotmonitor/login.html')

        # Autenticar usuário
        user = authenticate(request, username=username, password=password)

        if user is not None:
            if user.is_active:
                auth_login(request, user)

                # Configurar sessão baseado no "lembrar-me"
                if not remember_me:
                    # Sessão expira quando o navegador fechar
                    request.session.set_expiry(0)
                else:
                    request.session.set_expiry(1209600)  # 2 semanas

                logger.info(
                    f'Login bem-sucedido para usuário: {user.username}')
                messages.success(request, f'Bem-vindo, {user.username}!')

                # Redirecionar para próxima página se especificada
                next_page = request.GET.get('next', 'dashboard')
                return redirect(next_page)
            else:
                messages.error(request, 'Sua conta está desativada.')
        else:
            messages.error(request, 'Email ou senha incorretos.')
            logger.warning(
                f'Tentativa de login com credenciais inválidas para: {email}')

    return render(request, 'iotmonitor/login.html')


def cadastro(request):
    """
    View para cadastro de novos usuários
    """
    if request.user.is_authenticated:
        return redirect('dashboard')

    if request.method == 'POST':
        username = request.POST.get('username', '').strip()
        email = request.POST.get('email', '').strip()
        password = request.POST.get('password', '')
        password_confirm = request.POST.get('password_confirm', '')

        # Validações
        errors = []

        if not username or not email or not password or not password_confirm:
            errors.append('Todos os campos são obrigatórios.')

        if len(username) < 3:
            errors.append('Nome de usuário deve ter pelo menos 3 caracteres.')

        if not username.replace('_', '').isalnum():
            errors.append(
                'Nome de usuário pode conter apenas letras, números e underscore.')

        try:
            validate_email(email)
        except ValidationError:
            errors.append('Digite um email válido.')

        if len(password) < 8:
            errors.append('A senha deve ter pelo menos 8 caracteres.')

        if password != password_confirm:
            errors.append('As senhas não coincidem.')

        # Verificar se usuário já existe
        if User.objects.filter(username=username).exists():
            errors.append('Nome de usuário já existe.')

        if User.objects.filter(email=email).exists():
            errors.append('Email já está cadastrado.')

        # Se há erros, exibir e retornar
        if errors:
            for error in errors:
                messages.error(request, error)
            return render(request, 'iotmonitor/cadastro.html')

        # Criar usuário
        try:
            user = User.objects.create_user(
                username=username,
                email=email,
                password=password,
                first_name='',  # Pode ser adicionado depois no perfil
                last_name=''
            )

            logger.info(f'Novo usuário cadastrado: {username} ({email})')
            messages.success(
                request, 'Conta criada com sucesso! Faça login para continuar.')
            return redirect('login')

        except Exception as e:
            logger.error(f'Erro ao criar usuário {username}: {str(e)}')
            messages.error(
                request, 'Erro interno. Tente novamente em alguns minutos.')

    return render(request, 'iotmonitor/cadastro.html')


@require_http_methods(["GET", "POST"])
@csrf_protect
def logout_view(request):
    """
    View para logout de usuários
    Aceita tanto GET quanto POST para compatibilidade
    """
    if request.user.is_authenticated:
        username = request.user.username
        auth_logout(request)
        logger.info(f'Logout realizado para usuário: {username}')
        messages.success(request, 'Você foi desconectado com sucesso.')

    return redirect('homepage')

# Views AJAX para validações em tempo real


@csrf_exempt
def check_username(request):
    """
    View AJAX para verificar se username já existe
    """
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            username = data.get('username', '').strip()

            if not username:
                return JsonResponse({'exists': False, 'valid': False})

            exists = User.objects.filter(username=username).exists()
            valid = len(username) >= 3 and username.replace('_', '').isalnum()

            return JsonResponse({
                'exists': exists,
                'valid': valid,
                'message': 'Nome de usuário já existe.' if exists else ''
            })
        except Exception as e:
            logger.error(f'Erro na verificação de username: {str(e)}')
            return JsonResponse({'error': 'Erro interno'}, status=500)

    return JsonResponse({'error': 'Método não permitido'}, status=405)


@csrf_exempt
def check_email(request):
    """
    View AJAX para verificar se email já existe
    """
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            email = data.get('email', '').strip()

            if not email:
                return JsonResponse({'exists': False, 'valid': False})

            # Validar formato do email
            try:
                validate_email(email)
                valid = True
            except ValidationError:
                valid = False

            exists = User.objects.filter(
                email=email).exists() if valid else False

            return JsonResponse({
                'exists': exists,
                'valid': valid,
                'message': 'Email já está cadastrado.' if exists else 'Email inválido.' if not valid else ''
            })
        except Exception as e:
            logger.error(f'Erro na verificação de email: {str(e)}')
            return JsonResponse({'error': 'Erro interno'}, status=500)

    return JsonResponse({'error': 'Método não permitido'}, status=405)





def password_reset_request(request):
    """
    View para solicitar recuperação de senha
    """
    if request.method == 'POST':
        email = request.POST.get('email', '').strip()

        if not email:
            messages.error(request, 'Digite seu email.')
            return render(request, 'iotmonitor/registration/password_reset.html')

        try:
            validate_email(email)
        except ValidationError:
            messages.error(request, 'Digite um email válido.')
            return render(request, 'iotmonitor/registration/password_reset.html')

        try:
            user = User.objects.get(email=email)

            if send_password_reset_email(request, user):
                messages.success(request,
                                 'Se este email estiver cadastrado, você receberá instruções para redefinir sua senha.')
            else:
                messages.error(
                    request, 'Erro ao enviar email. Tente novamente.')

        except User.DoesNotExist:
            # Por segurança, não revelamos se o email existe ou não
            messages.success(request,
                             'Se este email estiver cadastrado, você receberá instruções para redefinir sua senha.')

        return redirect('password_reset_done')

    return render(request, 'iotmonitor/registration/password_reset.html')


def password_reset_confirm(request, uidb64, token):
    """
    View para confirmar e redefinir senha
    """
    user = validate_password_reset_token(uidb64, token)

    if user is None:
        messages.error(request, 'Link inválido ou expirado.')
        return redirect('password_reset')

    if request.method == 'POST':
        password = request.POST.get('password', '')
        password_confirm = request.POST.get('password_confirm', '')

        if not password or not password_confirm:
            messages.error(request, 'Todos os campos são obrigatórios.')
            return render(request, 'registration/password_reset_confirm.html', {'user': user})

        if len(password) < 8:
            messages.error(
                request, 'A senha deve ter pelo menos 8 caracteres.')
            return render(request, 'registration/password_reset_confirm.html', {'user': user})

        if password != password_confirm:
            messages.error(request, 'As senhas não coincidem.')
            return render(request, 'registration/password_reset_confirm.html', {'user': user})

        # Redefinir senha
        user.set_password(password)
        user.save()

        logger.info(f'Senha redefinida para usuário: {user.username}')
        messages.success(
            request, 'Senha redefinida com sucesso! Faça login com sua nova senha.')
        return redirect('login')

    return render(request, 'registration/password_reset_confirm.html', {'user': user})


def password_reset_done(request):
    """
    View de confirmação após solicitar recuperação
    """
    return render(request, 'registration/password_reset_done.html')


def password_reset_complete(request):
    """
    View de confirmação após redefinir senha
    """
    return render(request, 'registration/password_reset_complete.html')

# ==================== DASHBOARD COM IQA ====================


def get_sensor_data():
    """
    Obter dados dos sensores com logging detalhado
    """
    logger.debug("Iniciando obtenção de dados dos sensores")
    
    try:
        # Tentar cache primeiro
        cached_data = cache.get('sensor_data')
        if cached_data:
            logger.debug("Dados obtidos do cache")
            sensor_logger.info("Dados carregados do cache com sucesso")
            return cached_data
        
        logger.debug("Dados não encontrados no cache, tentando banco de dados")
        
        # Tentar banco de dados
        try:
            from .models import SensorReading
            latest_reading = SensorReading.objects.latest('timestamp')
            
            data = {
                'Coliformes': latest_reading.coliformes,
                'pH': latest_reading.ph,
                'DBO': latest_reading.dbo,
                'NT': latest_reading.nt,
                'FT': latest_reading.ft,
                'Temperatura': latest_reading.temperatura,
                'Turbidez': latest_reading.turbidez,
                'Residuos': latest_reading.residuos,
                'OD': latest_reading.od,
            }
            
            cache.set('sensor_data', data, 30)
            logger.info(f"Dados obtidos do banco de dados: Device {latest_reading.device_id}")
            sensor_logger.info(f"Última leitura: {latest_reading.timestamp}, Device: {latest_reading.device_id}")
            return data
            
        except Exception as db_error:
            logger.warning(f"Erro ao acessar banco de dados: {db_error}")
            sensor_logger.warning("Fallback para dados de exemplo")
            
        # Fallback para dados de exemplo
        fallback_data = {
            'Coliformes': 30.96,
            'pH': 8.9,
            'DBO': 6.06,
            'NT': 2.10,
            'FT': 0.22,
            'Temperatura': 25.0,
            'Turbidez': 32.62,
            'Residuos': 255.75,
            'OD': 7.28
        }
        
        logger.info("Usando dados de fallback")
        return fallback_data
        
    except Exception as e:
        logger.error(f"Erro crítico ao obter dados dos sensores: {e}")
        sensor_logger.error(f"Falha completa na obtenção de dados: {e}")
        
        # Dados de emergência
        return {
            'Coliformes': 0,
            'pH': 7.0,
            'DBO': 0,
            'NT': 0,
            'FT': 0,
            'Temperatura': 25,
            'Turbidez': 0,
            'Residuos': 0,
            'OD': 0
        }

@csrf_exempt
@require_http_methods(["POST"])
def esp_sensor_data(request):
    """
    Endpoint para receber dados dos sensores com logging completo
    """
    client_ip = request.META.get('REMOTE_ADDR', 'Unknown')
    esp_logger.info(f"Conexão recebida de IP: {client_ip}")
    
    try:
        # Parse do JSON
        raw_data = request.body.decode('utf-8')
        esp_logger.debug(f"Dados brutos recebidos: {raw_data}")
        
        data = json.loads(raw_data)
        device_id = data.get('device_id', 'Unknown')
        
        esp_logger.info(f"Processando dados do dispositivo: {device_id}")
        
        # Validação de campos obrigatórios
        required_fields = ['coliformes', 'ph', 'dbo', 'nt', 'ft', 
                          'temperatura', 'turbidez', 'residuos', 'od']
        
        missing_fields = [field for field in required_fields if field not in data]
        if missing_fields:
            error_msg = f'Campos obrigatórios ausentes: {missing_fields}'
            esp_logger.warning(f"Dispositivo {device_id}: {error_msg}")
            return JsonResponse({'success': False, 'error': error_msg}, status=400)
        
        # Conversão e validação de tipos
        try:
            sensor_data = {
                'Coliformes': float(data['coliformes']),
                'pH': float(data['ph']),
                'DBO': float(data['dbo']),
                'NT': float(data['nt']),
                'FT': float(data['ft']),
                'Temperatura': float(data['temperatura']),
                'Turbidez': float(data['turbidez']),
                'Residuos': float(data['residuos']),
                'OD': float(data['od']),
            }
        except (ValueError, TypeError) as e:
            error_msg = f'Erro na conversão de dados: {str(e)}'
            esp_logger.error(f"Dispositivo {device_id}: {error_msg}")
            return JsonResponse({'success': False, 'error': error_msg}, status=400)
        
        # Log dos valores recebidos
        sensor_logger.info(f"Dados válidos recebidos do {device_id}:")
        for param, value in sensor_data.items():
            sensor_logger.info(f"  {param}: {value}")
        
        # Validações de range
        validation_errors = []
        
        if not (0 <= sensor_data['pH'] <= 14):
            validation_errors.append(f"pH fora do range (0-14): {sensor_data['pH']}")
        
        if sensor_data['Temperatura'] < -50 or sensor_data['Temperatura'] > 100:
            validation_errors.append(f"Temperatura fora do range (-50°C a 100°C): {sensor_data['Temperatura']}")
        
        if sensor_data['OD'] < 0:
            validation_errors.append(f"Oxigênio Dissolvido negativo: {sensor_data['OD']}")
        
        if validation_errors:
            for error in validation_errors:
                esp_logger.warning(f"Dispositivo {device_id}: {error}")
            return JsonResponse({
                'success': False, 
                'error': 'Dados fora dos ranges válidos',
                'details': validation_errors
            }, status=400)
        
        # Verificar alertas
        check_sensor_alerts(sensor_data, device_id)
        
        # Salvar no banco de dados
        try:
            from .models import SensorReading
            
            new_reading = SensorReading.objects.create(
                coliformes=sensor_data['Coliformes'],
                ph=sensor_data['pH'],
                dbo=sensor_data['DBO'],
                nt=sensor_data['NT'],
                ft=sensor_data['FT'],
                temperatura=sensor_data['Temperatura'],
                turbidez=sensor_data['Turbidez'],
                residuos=sensor_data['Residuos'],
                od=sensor_data['OD'],
                device_id=device_id
            )
            
            sensor_logger.info(f"Leitura salva no banco: ID {new_reading.id}, Device {device_id}")
            
        except Exception as db_error:
            logger.error(f"Erro ao salvar no banco: {db_error}")
            # Continua mesmo se não conseguir salvar no banco
        
        # Salvar no cache
        cache.set('sensor_data', sensor_data, 300)
        logger.debug("Dados salvos no cache")
        
        # Calcular IQA
        iqa_valor = None
        try:
            from .iqa_calculator import IQACalculator
            calculator = IQACalculator()
            iqa_valor, _ = calculator.calcular_IQA(sensor_data)
            
            iqa_logger.info(f"IQA calculado para {device_id}: {iqa_valor:.2f}")
            
            # Alert se IQA muito baixo
            if iqa_valor < 25:
                alert_logger.warning(f"IQA crítico detectado no {device_id}: {iqa_valor:.2f}")
            
        except ImportError:
            iqa_logger.warning("IQACalculator não disponível")
        except Exception as iqa_error:
            iqa_logger.error(f"Erro no cálculo do IQA: {iqa_error}")
        
        # Resposta de sucesso
        response_data = {
            'success': True,
            'message': 'Dados recebidos e processados com sucesso',
            'timestamp': timezone.now().isoformat(),
            'device_id': device_id
        }
        
        if iqa_valor is not None:
            response_data['iqa_calculado'] = round(iqa_valor, 2)
        
        esp_logger.info(f"Processamento concluído com sucesso para {device_id}")
        return JsonResponse(response_data)
        
    except json.JSONDecodeError as e:
        error_msg = f'JSON inválido: {str(e)}'
        esp_logger.error(f"Erro de parsing JSON de {client_ip}: {error_msg}")
        return JsonResponse({'success': False, 'error': 'JSON inválido'}, status=400)
    
    except Exception as e:
        logger.error(f"Erro inesperado ao processar dados do ESP: {e}")
        esp_logger.error(f"Erro crítico no processamento: {e}")
        return JsonResponse({
            'success': False,
            'error': 'Erro interno do servidor'
        }, status=500)
def check_sensor_alerts(sensor_data, device_id):
    """
    Verifica e registra alertas baseados nos dados dos sensores
    """
    alerts = []
    
    # Alertas de pH
    if sensor_data['pH'] < 6.0:
        alerts.append(f"pH muito ácido: {sensor_data['pH']}")
    elif sensor_data['pH'] > 9.0:
        alerts.append(f"pH muito básico: {sensor_data['pH']}")
    
    # Alertas de temperatura
    if sensor_data['Temperatura'] > 35:
        alerts.append(f"Temperatura alta: {sensor_data['Temperatura']}°C")
    elif sensor_data['Temperatura'] < 5:
        alerts.append(f"Temperatura baixa: {sensor_data['Temperatura']}°C")
    
    # Alertas de oxigênio dissolvido
    if sensor_data['OD'] < 4:
        alerts.append(f"Oxigênio dissolvido baixo: {sensor_data['OD']} mg/L")
    
    # Alertas de turbidez
    if sensor_data['Turbidez'] > 40:
        alerts.append(f"Turbidez alta: {sensor_data['Turbidez']} NTU")
    
    # Alertas de coliformes
    if sensor_data['Coliformes'] > 80:
        alerts.append(f"Coliformes elevados: {sensor_data['Coliformes']} NMP/100mL")
    
    # Registrar alertas
    for alert in alerts:
        alert_logger.warning(f"ALERTA {device_id}: {alert}")
    
    if alerts:
        logger.info(f"Total de {len(alerts)} alertas gerados para {device_id}")

@csrf_exempt
@require_http_methods(["POST", "GET"])
def esp_heartbeat(request):
    """
    Endpoint para heartbeat com logging
    """
    client_ip = request.META.get('REMOTE_ADDR', 'Unknown')
    
    if request.method == 'POST':
        try:
            data = json.loads(request.body.decode('utf-8'))
            device_id = data.get('device_id', 'Unknown')
            status = data.get('status', 'online')
            uptime = data.get('uptime', 0)
            
            esp_logger.info(f"Heartbeat de {device_id} (IP: {client_ip}): {status}, uptime: {uptime}ms")
            
            # Salvar status no cache
            cache.set(f'device_status_{device_id}', {
                'status': status,
                'last_seen': timezone.now().isoformat(),
                'uptime': uptime,
                'ip_address': client_ip
            }, 600)
            
            return JsonResponse({
                'success': True,
                'message': 'Heartbeat recebido',
                'server_time': timezone.now().isoformat()
            })
            
        except Exception as e:
            esp_logger.error(f"Erro no heartbeat de {client_ip}: {e}")
            return JsonResponse({'success': False, 'error': str(e)}, status=500)
    
    else:  # GET
        esp_logger.debug(f"Ping recebido de {client_ip}")
        return JsonResponse({
            'success': True,
            'server_time': timezone.now().isoformat(),
            'message': 'Servidor online'
        })


def get_flood_risk_assessment(valores_sensores):
    """
    Avalia risco de enchente com logging
    """
    turbidez = valores_sensores['Turbidez']
    residuos = valores_sensores['Residuos']
    
    logger.debug(f"Avaliando risco de enchente: Turbidez={turbidez}, Resíduos={residuos}")
    
    if turbidez > 50 or residuos > 400:
        risk_level = "Risco Alto"
        css_class = "status-critical"
        alert_logger.warning(f"Risco alto de enchente detectado: Turbidez={turbidez}, Resíduos={residuos}")
    elif turbidez > 25 or residuos > 300:
        risk_level = "Risco Moderado"
        css_class = "status-warning"
        logger.info(f"Risco moderado de enchente: Turbidez={turbidez}, Resíduos={residuos}")
    else:
        risk_level = "Risco Baixo"
        css_class = "status-good"
        logger.debug("Risco baixo de enchente")
    
    return risk_level, css_class

def dashboard(request):
    """
    View principal do dashboard com logging detalhado
    """
    logger.info(f"Dashboard acessado por {request.user if request.user.is_authenticated else 'usuário anônimo'}")
    
    try:
        # Obter dados dos sensores
        valores_sensores = get_sensor_data()
        logger.debug("Dados dos sensores obtidos para dashboard")
        
        # Calcular IQA
        iqa_valor = 0
        classificacao = 'Não calculado'
        css_class = 'status-warning'
        subindices = {}
        alertas = {}
        
        try:
            from .iqa_calculator import IQACalculator
            calculator = IQACalculator()
            iqa_valor, subindices = calculator.calcular_IQA(valores_sensores)
            classificacao, css_class = calculator.classificar_IQA(iqa_valor)
            alertas = calculator.get_parametros_alertas(valores_sensores)
            
            iqa_logger.info(f"IQA calculado para dashboard: {iqa_valor:.2f} ({classificacao})")
            
        except ImportError:
            iqa_logger.warning("IQACalculator não disponível para dashboard")
        except Exception as e:
            iqa_logger.error(f"Erro no cálculo do IQA para dashboard: {e}")
        
        # Avaliar risco de enchente
        flood_risk, flood_css = get_flood_risk_assessment(valores_sensores)
        logger.debug(f"Risco de enchente avaliado: {flood_risk}")
        
        # Status dos dispositivos
        device_status = cache.get('device_status_ESP32_001', {
            'status': 'offline',
            'last_seen': 'Nunca'
        })
        
        # Preparar contexto
        context = {
            'sensor_data': {
                'temperatura': valores_sensores['Temperatura'],
                'ph': valores_sensores['pH'],
                'oxigenio': valores_sensores['OD'],
                'dbo': valores_sensores['DBO'],
                'coliformes': valores_sensores['Coliformes'],
                'nitrogenio': valores_sensores['NT'],
                'fosforo': valores_sensores['FT'],
                'turbidez': valores_sensores['Turbidez'],
                'solidos': valores_sensores['Residuos']
            },
            'iqa': {
                'valor': iqa_valor,
                'classificacao': classificacao,
                'css_class': css_class
            },
            'subindices': subindices,
            'alertas': alertas,
            'flood_risk': flood_risk,
            'flood_css': flood_css,
            'last_update': timezone.now(),
            'device_status': device_status,
            'valores_brutos': valores_sensores
        }
        
        if request.user.is_authenticated:
            context.update({
                'user': request.user,
                'username': request.user.username,
                'email': request.user.email,
                'date_joined': request.user.date_joined,
                'last_login': request.user.last_login,
            })
            logger.debug(f"Dashboard carregado para usuário autenticado: {request.user.username}")
        
        logger.info("Dashboard renderizado com sucesso")
        return render(request, 'iotmonitor/dashboard.html', context)
        
    except Exception as e:
        logger.error(f"Erro crítico no dashboard: {e}")
        
        # Contexto de fallback
        context = {
            'error_message': 'Erro ao carregar dados dos sensores',
            'sensor_data': {},
            'iqa': {'valor': 0, 'classificacao': 'Erro', 'css_class': 'status-critical'}
        }
        
        if request.user.is_authenticated:
            context.update({
                'user': request.user,
                'username': request.user.username,
                'email': request.user.email,
            })
        
        return render(request, 'iotmonitor/dashboard.html', context)


@require_http_methods(["GET"])
def dashboard_api(request):
    """
    API endpoint para obter dados do dashboard em JSON
    Útil para atualizações via AJAX ou apps móveis
    """
    try:
        valores_sensores = get_sensor_data()
        calculator = IQACalculator()
        iqa_valor, subindices = calculator.calcular_IQA(valores_sensores)
        classificacao, css_class = calculator.classificar_IQA(iqa_valor)
        alertas = calculator.get_parametros_alertas(valores_sensores)
        flood_risk, flood_css = get_flood_risk_assessment(valores_sensores)
        
        data = {
            'success': True,
            'timestamp': timezone.now().isoformat(),
            'sensor_data': {
                'temperatura': valores_sensores['Temperatura'] + 22.5,
                'ph': valores_sensores['pH'],
                'oxigenio': valores_sensores['OD'],
                'dbo': valores_sensores['DBO'],
                'coliformes': valores_sensores['Coliformes'],
                'nitrogenio': valores_sensores['NT'],
                'fosforo': valores_sensores['FT'],
                'turbidez': valores_sensores['Turbidez'],
                'solidos': valores_sensores['Residuos']
            },
            'iqa': {
                'valor': iqa_valor,
                'classificacao': classificacao,
                'css_class': css_class
            },
            'subindices': subindices,
            'alertas': alertas,
            'flood_risk': {
                'level': flood_risk,
                'css_class': flood_css
            }
        }
        
        return JsonResponse(data)
        
    except Exception as e:
        logger.error(f"Erro na API do dashboard: {e}")
        return JsonResponse({
            'success': False,
            'error': str(e),
            'timestamp': timezone.now().isoformat()
        }, status=500)


@csrf_exempt
@require_http_methods(["POST"])
def update_sensor_values(request):
    """
    View para atualizar valores dos sensores (para testes ou integração)
    Apenas para desenvolvimento - remover em produção
    """
    try:
        data = json.loads(request.body)
        
        # Aqui você implementaria a lógica para salvar novos valores
        # Por exemplo, salvando em um modelo Django
        
        # Exemplo de como seria:
        # from .models import SensorReading
        # 
        # new_reading = SensorReading(
        #     coliformes=data.get('coliformes'),
        #     ph=data.get('ph'),
        #     dbo=data.get('dbo'),
        #     # ... outros campos
        #     timestamp=timezone.now()
        # )
        # new_reading.save()
        
        logger.info(f"Valores de sensores atualizados: {data}")
        return JsonResponse({'success': True, 'message': 'Valores atualizados'})
        
    except json.JSONDecodeError:
        return JsonResponse({'success': False, 'error': 'JSON inválido'}, status=400)
    except Exception as e:
        logger.error(f"Erro ao atualizar sensores: {e}")
        return JsonResponse({'success': False, 'error': str(e)}, status=500)