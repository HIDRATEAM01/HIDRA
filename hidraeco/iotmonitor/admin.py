from django.contrib import admin

# Register your models here.
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
