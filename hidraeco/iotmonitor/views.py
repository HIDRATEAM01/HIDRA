from django.shortcuts import render
from django.http import HttpResponse, HttpResponseBadRequest, HttpResponseForbidden
from django.utils import timezone

def homepage(request):
    return render(request, 'iotmonitor/homepage.html')

def login(request):
    return render(request, 'iotmonitor/login.html')

def cadastro(request):
    return render(request, 'iotmonitor/cadastro.html')

def dashboard(request):
    last_update = timezone.now()
    return render(request, 'iotmonitor/dashboard.html', {'last_update': last_update})