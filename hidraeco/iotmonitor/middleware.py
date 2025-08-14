from django.http import HttpResponseRedirect
from django.urls import reverse
from django.contrib import messages
import logging

logger = logging.getLogger(__name__)

class SecurityMiddleware:
    """
    Middleware para adicionar camadas de segurança
    """
    def __init__(self, get_response):
        self.get_response = get_response

    def __call__(self, request):
        # Código executado antes da view
        
        # Adicionar headers de segurança
        response = self.get_response(request)
        
        # Headers de segurança
        response['X-Content-Type-Options'] = 'nosniff'
        response['X-Frame-Options'] = 'DENY'
        response['X-XSS-Protection'] = '1; mode=block'
        
        return response
    
    