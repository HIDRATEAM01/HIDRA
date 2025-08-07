# models.py - Modelos para armazenar dados dos sensores

from django.db import models
from django.utils import timezone
from django.core.validators import MinValueValidator, MaxValueValidator

class SensorReading(models.Model):
    """
    Modelo para armazenar leituras dos sensores de qualidade da água
    """
    # Dados dos sensores
    coliformes = models.FloatField(
        verbose_name="Coliformes",
        help_text="Coliformes fecais em NMP/100mL",
        validators=[MinValueValidator(0)]
    )
    
    ph = models.FloatField(
        verbose_name="pH",
        help_text="Potencial Hidrogeniônico",
        validators=[MinValueValidator(0), MaxValueValidator(14)]
    )
    
    dbo = models.FloatField(
        verbose_name="DBO",
        help_text="Demanda Bioquímica de Oxigênio em mg/L",
        validators=[MinValueValidator(0)]
    )
    
    nt = models.FloatField(
        verbose_name="Nitrogênio Total",
        help_text="Nitrogênio Total em mg/L",
        validators=[MinValueValidator(0)]
    )
    
    ft = models.FloatField(
        verbose_name="Fósforo Total",
        help_text="Fósforo Total em mg/L",
        validators=[MinValueValidator(0)]
    )
    
    temperatura = models.FloatField(
        verbose_name="Temperatura",
        help_text="Temperatura da água em °C",
        validators=[MinValueValidator(-50), MaxValueValidator(100)]
    )
    
    turbidez = models.FloatField(
        verbose_name="Turbidez",
        help_text="Turbidez em NTU",
        validators=[MinValueValidator(0)]
    )
    
    residuos = models.FloatField(
        verbose_name="Resíduos Sólidos",
        help_text="Resíduos Sólidos Totais em mg/L",
        validators=[MinValueValidator(0)]
    )
    
    od = models.FloatField(
        verbose_name="Oxigênio Dissolvido",
        help_text="Oxigênio Dissolvido em mg/L",
        validators=[MinValueValidator(0)]
    )
    
    # Metadados
    device_id = models.CharField(
        max_length=50,
        default="ESP32_001",
        verbose_name="ID do Dispositivo",
        help_text="Identificador único do dispositivo sensor"
    )
    
    timestamp = models.DateTimeField(
        default=timezone.now,
        verbose_name="Timestamp",
        help_text="Data e hora da leitura"
    )
    
    # Campos calculados
    iqa_valor = models.FloatField(
        null=True,
        blank=True,
        verbose_name="Valor IQA",
        help_text="Índice de Qualidade da Água calculado"
    )
    
    iqa_classificacao = models.CharField(
        max_length=20,
        null=True,
        blank=True,
        verbose_name="Classificação IQA",
        help_text="Classificação baseada no valor IQA"
    )
    
    # Status da leitura
    is_valid = models.BooleanField(
        default=True,
        verbose_name="Leitura Válida",
        help_text="Indica se a leitura passou nas validações"
    )
    
    notes = models.TextField(
        blank=True,
        verbose_name="Observações",
        help_text="Observações sobre a leitura"
    )

    class Meta:
        ordering = ['-timestamp']
        verbose_name = "Leitura do Sensor"
        verbose_name_plural = "Leituras dos Sensores"
        indexes = [
            models.Index(fields=['timestamp']),
            models.Index(fields=['device_id', 'timestamp']),
        ]

    def __str__(self):
        return f"Leitura {self.device_id} - {self.timestamp.strftime('%d/%m/%Y %H:%M')}"
    
    def save(self, *args, **kwargs):
        """
        Override do save para calcular IQA automaticamente
        """
        # Calcular IQA antes de salvar
        try:
            from .iqa_calculator import IQACalculator
            calculator = IQACalculator()
            
            sensor_data = {
                'Coliformes': self.coliformes,
                'pH': self.ph,
                'DBO': self.dbo,
                'NT': self.nt,
                'FT': self.ft,
                'Temperatura': self.temperatura,
                'Turbidez': self.turbidez,
                'Residuos': self.residuos,
                'OD': self.od,
            }
            
            self.iqa_valor, _ = calculator.calcular_IQA(sensor_data)
            self.iqa_classificacao, _ = calculator.classificar_IQA(self.iqa_valor)
            
        except ImportError:
            pass  # IQA calculator não disponível
        
        super().save(*args, **kwargs)

    @property
    def temperatura_celsius(self):
        """Retorna temperatura em Celsius"""
        return self.temperatura
    
    @property
    def ph_status(self):
        """Retorna status do pH"""
        if 6.5 <= self.ph <= 8.5:
            return "Normal"
        elif self.ph < 6.5:
            return "Ácido"
        else:
            return "Básico"
    
    @property
    def od_status(self):
        """Retorna status do oxigênio dissolvido"""
        if self.od >= 5:
            return "Bom"
        elif self.od >= 3:
            return "Moderado"
        else:
            return "Ruim"


class DeviceStatus(models.Model):
    """
    Modelo para rastrear status dos dispositivos ESP32/ESP8266
    """
    device_id = models.CharField(
        max_length=50,
        unique=True,
        verbose_name="ID do Dispositivo"
    )
    
    status = models.CharField(
        max_length=20,
        choices=[
            ('online', 'Online'),
            ('offline', 'Offline'),
            ('error', 'Erro'),
            ('maintenance', 'Manutenção'),
        ],
        default='offline',
        verbose_name="Status"
    )
    
    last_seen = models.DateTimeField(
        default=timezone.now,
        verbose_name="Última Conexão"
    )
    
    ip_address = models.GenericIPAddressField(
        null=True,
        blank=True,
        verbose_name="Endereço IP"
    )
    
    firmware_version = models.CharField(
        max_length=20,
        null=True,
        blank=True,
        verbose_name="Versão do Firmware"
    )
    
    battery_level = models.IntegerField(
        null=True,
        blank=True,
        validators=[MinValueValidator(0), MaxValueValidator(100)],
        verbose_name="Nível da Bateria (%)"
    )
    
    signal_strength = models.IntegerField(
        null=True,
        blank=True,
        verbose_name="Força do Sinal WiFi (dBm)"
    )
    
    uptime = models.BigIntegerField(
        null=True,
        blank=True,
        verbose_name="Tempo de Atividade (ms)"
    )
    
    total_readings = models.IntegerField(
        default=0,
        verbose_name="Total de Leituras"
    )
    
    error_count = models.IntegerField(
        default=0,
        verbose_name="Contador de Erros"
    )
    
    notes = models.TextField(
        blank=True,
        verbose_name="Observações"
    )

    class Meta:
        verbose_name = "Status do Dispositivo"
        verbose_name_plural = "Status dos Dispositivos"
        ordering = ['device_id']

    def __str__(self):
        return f"{self.device_id} - {self.status}"
    
    @property
    def is_online(self):
        """Verifica se dispositivo está online baseado na última conexão"""
        from datetime import timedelta
        threshold = timezone.now() - timedelta(minutes=5)
        return self.last_seen > threshold and self.status == 'online'
    
    @property
    def uptime_hours(self):
        """Retorna uptime em horas"""
        if self.uptime:
            return round(self.uptime / (1000 * 60 * 60), 2)
        return 0


class SensorAlert(models.Model):
    """
    Modelo para alertas baseados nas leituras dos sensores
    """
    ALERT_TYPES = [
        ('ph_high', 'pH Alto'),
        ('ph_low', 'pH Baixo'),
        ('temperature_high', 'Temperatura Alta'),
        ('temperature_low', 'Temperatura Baixa'),
        ('turbidity_high', 'Turbidez Alta'),
        ('coliform_high', 'Coliformes Alto'),
        ('od_low', 'Oxigênio Dissolvido Baixo'),
        ('device_offline', 'Dispositivo Offline'),
        ('iqa_poor', 'IQA Ruim'),
    ]
    
    SEVERITY_LEVELS = [
        ('low', 'Baixo'),
        ('medium', 'Médio'),
        ('high', 'Alto'),
        ('critical', 'Crítico'),
    ]
    
    sensor_reading = models.ForeignKey(
        SensorReading,
        on_delete=models.CASCADE,
        null=True,
        blank=True,
        verbose_name="Leitura do Sensor"
    )
    
    device_id = models.CharField(
        max_length=50,
        verbose_name="ID do Dispositivo"
    )
    
    alert_type = models.CharField(
        max_length=20,
        choices=ALERT_TYPES,
        verbose_name="Tipo de Alerta"
    )
    
    severity = models.CharField(
        max_length=10,
        choices=SEVERITY_LEVELS,
        default='medium',
        verbose_name="Severidade"
    )
    
    message = models.TextField(
        verbose_name="Mensagem"
    )
    
    value = models.FloatField(
        null=True,
        blank=True,
        verbose_name="Valor que Causou o Alerta"
    )
    
    threshold = models.FloatField(
        null=True,
        blank=True,
        verbose_name="Limite Ultrapassado"
    )
    
    timestamp = models.DateTimeField(
        default=timezone.now,
        verbose_name="Data/Hora do Alerta"
    )
    
    acknowledged = models.BooleanField(
        default=False,
        verbose_name="Reconhecido"
    )
    
    acknowledged_by = models.CharField(
        max_length=100,
        null=True,
        blank=True,
        verbose_name="Reconhecido por"
    )
    
    acknowledged_at = models.DateTimeField(
        null=True,
        blank=True,
        verbose_name="Reconhecido em"
    )

    class Meta:
        ordering = ['-timestamp']
        verbose_name = "Alerta do Sensor"
        verbose_name_plural = "Alertas dos Sensores"
        indexes = [
            models.Index(fields=['timestamp']),
            models.Index(fields=['device_id', 'timestamp']),
            models.Index(fields=['acknowledged']),
        ]

    def __str__(self):
        return f"Alerta {self.device_id}: {self.get_alert_type_display()}"


# ==================== MIGRATIONS ====================
"""
Para criar as migrações, execute os comandos:

python manage.py makemigrations
python manage.py migrate

Se você já tem dados existentes, pode ser necessário fazer uma migração de dados.
"""

# ==================== ADMIN CUSTOMIZADO ====================
"""
Adicione ao seu admin.py:

from django.contrib import admin
from .models import SensorReading, DeviceStatus, SensorAlert

@admin.register(SensorReading)
class SensorReadingAdmin(admin.ModelAdmin):
    list_display = ['device_id', 'timestamp', 'ph', 'temperatura', 'iqa_valor', 'is_valid']
    list_filter = ['device_id', 'is_valid', 'timestamp']
    search_fields = ['device_id']
    readonly_fields = ['iqa_valor', 'iqa_classificacao']
    date_hierarchy = 'timestamp'

@admin.register(DeviceStatus)
class DeviceStatusAdmin(admin.ModelAdmin):
    list_display = ['device_id', 'status', 'last_seen', 'battery_level', 'total_readings']
    list_filter = ['status']
    search_fields = ['device_id']

@admin.register(SensorAlert)
class SensorAlertAdmin(admin.ModelAdmin):
    list_display = ['device_id', 'alert_type', 'severity', 'timestamp', 'acknowledged']
    list_filter = ['alert_type', 'severity', 'acknowledged']
    search_fields = ['device_id', 'message']
    actions = ['mark_acknowledged']
    
    def mark_acknowledged(self, request, queryset):
        queryset.update(acknowledged=True, acknowledged_by=request.user.username)
    mark_acknowledged.short_description = "Marcar como reconhecido"
"""