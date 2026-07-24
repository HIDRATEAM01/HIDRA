from django.contrib.auth.decorators import login_required
from django.shortcuts import redirect
from functools import wraps

def login_required_custom(view_func):
    """
    Decorator customizado que redireciona para login se usuário não estiver autenticado
    """
    @wraps(view_func)
    def wrapper(request, *args, **kwargs):
        if not request.user.is_authenticated:
            return redirect('login')
        return view_func(request, *args, **kwargs)
    return wrapper