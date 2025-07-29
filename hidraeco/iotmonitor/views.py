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



def dashboard(request):
    last_update = timezone.now()

    context = {
        'last_update': last_update,
    }

    if request.user.is_authenticated:
        context.update({
            'user': request.user,
            'username': request.user.username,
            'email': request.user.email,
            'date_joined': request.user.date_joined,
            'last_login': request.user.last_login,
        })

    return render(request, 'iotmonitor/dashboard.html', context)


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
