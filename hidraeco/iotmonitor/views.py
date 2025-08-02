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
import json
import logging
from django.contrib.auth.decorators import login_required
from django.views.decorators.http import require_http_methods
from django.views.decorators.csrf import csrf_exempt, csrf_protect
from .utils import send_password_reset_email, validate_password_reset_token
from django.conf import settings
from .iqa_calculator import IQACalculator

# Configurar logging
logger = logging.getLogger(__name__)


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
    Aqui você pode integrar com seu banco de dados ou API dos sensores.
    Por enquanto, usando os valores de exemplo do seu código.
    
    TODO: Substituir por consulta ao banco de dados ou API real dos sensores
    Exemplo de integração com modelo Django:
    
    from .models import SensorReading
    
    latest_reading = SensorReading.objects.latest('timestamp')
    return {
        'Coliformes': latest_reading.coliformes,
        'pH': latest_reading.ph,
        'DBO': latest_reading.dbo,
        # ... outros campos
    }
    """
    return {
        'Coliformes': 30.96,     # NMP/100mL
        'pH': 8.9,             # Unidades de pH
        'DBO': 6.06,            # mg/L
        'NT': 2.10,             # mg/L
        'FT': 0.22,             # mg/L
        'Temperatura': 2,       # ∆T (diferença da temperatura da água e a ambiente)
        'Turbidez': 32.62,      # NTU
        'Residuos': 255.75,     # mg/L
        'OD': 7.28              # mg/L
    }


def get_flood_risk_assessment(valores_sensores):
    """
    Avalia o risco de enchente baseado nos parâmetros da água
    """
    turbidez = valores_sensores['Turbidez']
    residuos = valores_sensores['Residuos']
    
    # Lógica simples de avaliação de risco
    if turbidez > 50 or residuos > 400:
        return "Risco Alto", "status-critical"
    elif turbidez > 25 or residuos > 300:
        return "Risco Moderado", "status-warning"
    else:
        return "Risco Baixo", "status-good"


def dashboard(request):
    """
    View principal do dashboard integrada com IQA
    """
    try:
        # Obter dados dos sensores
        valores_sensores = get_sensor_data()
        
        # Calcular IQA
        calculator = IQACalculator()
        iqa_valor, subindices = calculator.calcular_IQA(valores_sensores)
        classificacao, css_class = calculator.classificar_IQA(iqa_valor)
        
        # Obter alertas dos parâmetros
        alertas = calculator.get_parametros_alertas(valores_sensores)
        
        # Avaliar risco de enchente
        flood_risk, flood_css = get_flood_risk_assessment(valores_sensores)
        
        # Preparar dados dos sensores para exibição
        # Converter temperatura delta para temperatura absoluta (assumindo ambiente ~22.5°C)
        temperatura_absoluta = valores_sensores['Temperatura'] + 22.5
        
        # Preparar dados para o template
        context = {
            'sensor_data': {
                'temperatura': temperatura_absoluta,
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
            'valores_brutos': valores_sensores  # Para debugging se necessário
        }
        
        # Adicionar contexto do usuário se autenticado
        if request.user.is_authenticated:
            context.update({
                'user': request.user,
                'username': request.user.username,
                'email': request.user.email,
                'date_joined': request.user.date_joined,
                'last_login': request.user.last_login,
            })
        
        return render(request, 'iotmonitor/dashboard.html', context)
        
    except Exception as e:
        # Log do erro em produção
        logger.error(f"Erro no dashboard: {e}")
        
        # Contexto de fallback em caso de erro
        context = {
            'sensor_data': {
                'temperatura': 25.5,
                'ph': 7.2,
                'oxigenio': 6.8,
                'dbo': 3.2,
                'coliformes': 120,
                'nitrogenio': 1.8,
                'fosforo': 0.12,
                'turbidez': 15.6,
                'solidos': 450
            },
            'iqa': {
                'valor': 0,
                'classificacao': 'Erro no Cálculo',
                'css_class': 'status-critical'
            },
            'subindices': {},
            'alertas': {},
            'flood_risk': 'Dados Indisponíveis',
            'flood_css': 'status-warning',
            'last_update': timezone.now(),
            'error_message': 'Erro ao carregar dados dos sensores'
        }
        
        # Adicionar contexto do usuário se autenticado
        if request.user.is_authenticated:
            context.update({
                'user': request.user,
                'username': request.user.username,
                'email': request.user.email,
                'date_joined': request.user.date_joined,
                'last_login': request.user.last_login,
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