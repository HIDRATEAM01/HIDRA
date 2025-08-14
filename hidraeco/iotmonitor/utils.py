from django.contrib.auth.tokens import default_token_generator
from django.utils.http import urlsafe_base64_encode, urlsafe_base64_decode
from django.utils.encoding import force_bytes, force_str
from django.core.mail import send_mail
from django.template.loader import render_to_string
from django.contrib.sites.shortcuts import get_current_site
from django.contrib.auth.models import User
import logging
from django.conf import settings


logger = logging.getLogger(__name__)

def send_password_reset_email(request, user):
    """
    Enviar email de recuperação de senha
    """
    try:
        current_site = get_current_site(request)
        subject = 'Recuperação de senha - HIDRA'
        
        token = default_token_generator.make_token(user)
        uid = urlsafe_base64_encode(force_bytes(user.pk))
        
        context = {
            'user': user,
            'domain': current_site.domain,
            'uid': uid,
            'token': token,
        }
        
        message = render_to_string('registration/password_reset_email.html', context)
        
        send_mail(
            subject,
            message,
            settings.DEFAULT_FROM_EMAIL,
            [user.email],
            fail_silently=False,
        )
        
        logger.info(f'Email de recuperação enviado para: {user.email}')
        return True
        
    except Exception as e:
        logger.error(f'Erro ao enviar email de recuperação: {str(e)}')
        return False

def validate_password_reset_token(uidb64, token):
    """
    Validar token de recuperação de senha
    """
    try:
        uid = force_str(urlsafe_base64_decode(uidb64))
        user = User.objects.get(pk=uid)
    except (TypeError, ValueError, OverflowError, User.DoesNotExist):
        return None
    
    if default_token_generator.check_token(user, token):
        return user
    return None

def get_client_ip(request):
    """
    Obter IP real do cliente
    """
    x_forwarded_for = request.META.get('HTTP_X_FORWARDED_FOR')
    if x_forwarded_for:
        ip = x_forwarded_for.split(',')[0]
    else:
        ip = request.META.get('REMOTE_ADDR')
    return ip

def log_authentication_attempt(request, username, success=False):
    """
    Registrar tentativas de autenticação
    """
    ip_address = get_client_ip(request)
    user_agent = request.META.get('HTTP_USER_AGENT', '')
    
    log_message = f'Login {"bem-sucedido" if success else "falhado"} - User: {username}, IP: {ip_address}'
    
    if success:
        logger.info(log_message)
    else:
        logger.warning(log_message)