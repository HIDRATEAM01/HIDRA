from django.urls import path
from . import views


urlpatterns = [
    # URLs principais
    path('', views.homepage, name='homepage'),
    path('dashboard/', views.dashboard, name='dashboard'),
    # URLs de autenticação
    path('login/', views.login, name='login'),
    path('cadastro/', views.cadastro, name='cadastro'),
    path('logout/', views.logout_view, name='logout'),
    # URLs para recuperação de senha
    path('password_reset/', views.password_reset_request, name='password_reset'),
    path('password-reset/done/', views.password_reset_done, name='password_reset_done'),
    path('password-reset/confirm/<uidb64>/<token>/', views.password_reset_confirm, name='password_reset_confirm'),
    path('password-reset/complete/', views.password_reset_complete, name='password_reset_complete'),
    # URLs AJAX para validações
    path('check-username/', views.check_username, name='check_username'),
    path('check-email/', views.check_email, name='check_email'),
    # APIs do Dashboard com IQA
    path('api/dashboard/', views.dashboard_api, name='dashboard_api'),
    path('api/update-sensors/', views.update_sensor_values, name='update_sensors'),
]