# iotmonitor/firebase_service.py
import firebase_admin
from firebase_admin import credentials, db
from django.conf import settings
from django.core.cache import cache
import logging
import json
from datetime import datetime
from typing import Dict, Any, Optional
import os
import threading # Importar a biblioteca de threading

logger = logging.getLogger(__name__)
firebase_logger = logging.getLogger('firebase')

class FirebaseService:
    """
    Serviço para gerenciar conexão e operações com Firebase Realtime Database
    """
    _instance: Optional['FirebaseService'] = None
    _lock = threading.Lock()
    
    def __new__(cls):
        # Agora esta verificação funcionará corretamente
        if cls._instance is None:
            with cls._lock:
                if cls._instance is None:
                    cls._instance = super(FirebaseService, cls).__new__(cls)
        return cls._instance
    
    def __init__(self):
        if not hasattr(self, 'initialized'):
            self.app = None
            self.db_ref = None
            self.initialized = False
            self._initialize_firebase()
    
    def _initialize_firebase(self):
        """
        Inicializa conexão com Firebase usando credenciais do settings.py
        """
        try:
            if firebase_admin._apps:
                self.app = firebase_admin.get_app()
                firebase_logger.info("Firebase app já inicializado, reutilizando conexão")
            else:
                if hasattr(settings, 'FIREBASE_CREDENTIALS_JSON') and settings.FIREBASE_CREDENTIALS_JSON:
                    cred_dict = json.loads(settings.FIREBASE_CREDENTIALS_JSON)
                    cred = credentials.Certificate(cred_dict)
                elif hasattr(settings, 'FIREBASE_CREDENTIALS_PATH') and settings.FIREBASE_CREDENTIALS_PATH:
                    cred_path = settings.FIREBASE_CREDENTIALS_PATH
                    if not os.path.exists(cred_path):
                        cred_path = os.path.join(settings.BASE_DIR, os.path.basename(cred_path))
                    
                    if os.path.exists(cred_path):
                        firebase_logger.info(f"Carregando credenciais do arquivo: {cred_path}")
                        cred = credentials.Certificate(cred_path)
                    else:
                        raise FileNotFoundError(f"Arquivo de credenciais não encontrado: {cred_path}")
                else:
                    raise ValueError("Nenhuma configuração de credenciais Firebase encontrada")
                
                self.app = firebase_admin.initialize_app(cred, {
                    'databaseURL': settings.FIREBASE_DATABASE_URL
                })
                firebase_logger.info(f"Firebase inicializado com sucesso: {settings.FIREBASE_DATABASE_URL}")
            
            self.db_ref = db.reference('/')
            self.initialized = True
            
            if self.test_connection():
                firebase_logger.info("Teste de conectividade Firebase passou")
            else:
                firebase_logger.warning("Teste de conectividade Firebase falhou")
            
        except Exception as e:
            firebase_logger.error(f"Erro ao inicializar Firebase: {e}", exc_info=True)
            self.initialized = False

    def test_connection(self) -> bool:
        """ Testa a conexão com Firebase """
        try:
            if not self.initialized or not self.db_ref:
                return False
            # Usar uma chamada que não gera muito log para um simples teste
            self.db_ref.child('last_test').get(shallow=True)
            return True
        except Exception as e:
            firebase_logger.warning(f"Teste de conexão Firebase falhou: {e}")
            return False

    # --- FUNÇÃO ATUALIZADA COM LÓGICA DE EXTRAÇÃO MAIS SEGURA ---
    def get_latest_reading_from_leituras(self) -> Optional[Dict[str, Any]]:
        """
        Obtém a leitura mais recente do nó /leituras de forma segura.
        """
        if not self.initialized:
            firebase_logger.warning("Firebase não inicializado, impossível buscar leituras.")
            return None
        
        try:
            cache_key = 'firebase_latest_leitura'
            cached_data = cache.get(cache_key)
            if cached_data:
                firebase_logger.debug("Última leitura obtida do cache.")
                return cached_data

            leituras_ref = self.db_ref.child('leituras')
            latest_reading_node = leituras_ref.order_by_key().limit_to_last(1).get()

            raw_data = None
            try:
                # A chave é o ID da leitura (ex: "000001")
                id_leitura = list(latest_reading_node.keys())[0]
                # O valor correspondente já é o dicionário com os dados do sensor
                raw_data = latest_reading_node[id_leitura]

                if not isinstance(raw_data, dict):
                    firebase_logger.error(f"Payload para a leitura {id_leitura} não é um dicionário. Recebido: {type(raw_data)}")
                    return None

            except (IndexError, AttributeError) as e:
                firebase_logger.error(f"Erro ao extrair dados da estrutura do Firebase: {e}", exc_info=True)
                return None
            
            if not raw_data:
                 firebase_logger.error("Falha ao extrair raw_data da estrutura do Firebase.")
                 return None

            # Agora `raw_data` é o dicionário correto para ser formatado
            formatted_data = self._format_leitura_data(raw_data)
            
            cache.set(cache_key, formatted_data, 30)
            firebase_logger.info("Dados da última leitura obtidos e formatados com sucesso.")
            return formatted_data

        except Exception as e:
            firebase_logger.error(f"Erro ao obter a última leitura do Firebase: {e}", exc_info=True)
            return None

    def _format_leitura_data(self, raw_data: Dict[str, Any]) -> Dict[str, Any]:
        try:
            timestamp_str = raw_data.get('timestamp', '')
            try:
                dt_obj = datetime.strptime(timestamp_str, '%d-%m-%Y-%H-%M-%S')
                iso_timestamp = dt_obj.isoformat()
            except (ValueError, TypeError):
                iso_timestamp = datetime.now().isoformat()
                firebase_logger.warning(f"Timestamp inválido '{timestamp_str}', usando horário atual.")

            return {
                'OD': float(raw_data.get('oxigenio', 0)),
                'Residuos': float(raw_data.get('solidos', 0)),
                'NT': float(raw_data.get('nitrogenio', 0)),
                'FT': float(raw_data.get('fosforo', 0)),
                'Coliformes': float(raw_data.get('coliformes', 0)),
                'pH': float(raw_data.get('ph', 7.0)),
                'DBO': float(raw_data.get('dbo', 0)),
                'Temperatura': float(raw_data.get('temperatura', 25.0)),
                'Turbidez': float(raw_data.get('turbidez', 0)),
                'timestamp': iso_timestamp
            }
        except (ValueError, TypeError) as e:
            firebase_logger.error(f"Erro ao formatar dados da leitura: {e}")
            raise

    def get_device_status(self, device_id: str) -> Dict[str, Any]:
        """
        Determina o status do dispositivo verificando a idade da última leitura.
        """
        try:
            # Reutiliza a função que já busca e formata a última leitura
            latest_reading = self.get_latest_reading_from_leituras()

            if latest_reading and 'timestamp' in latest_reading:
                timestamp_str = latest_reading['timestamp']
                last_seen_dt = datetime.fromisoformat(timestamp_str)
                
                # Considera offline se a última leitura tem mais de 5 minutos
                if (datetime.now() - last_seen_dt).total_seconds() > 300:
                    status = "offline"
                    firebase_logger.warning(f"Dispositivo {device_id} considerado offline (última leitura > 5 min atrás).")
                else:
                    status = "online"
                
                return {
                    'status': status,
                    'last_seen': last_seen_dt.strftime('%d/%m/%Y %H:%M:%S')
                }
        except Exception as e:
            firebase_logger.error(f"Erro ao obter status do dispositivo {device_id}: {e}")

        # Retorno padrão em caso de erro ou nenhum dado
        return {
            'status': 'desconhecido',
            'last_seen': 'Nunca'
        }

    # O restante das funções (save_sensor_data, get_historical_data, etc.)
    # pode ser mantido para outras finalidades ou futuras implementações.
    # Elas operam na estrutura /sensors/ e não na /leituras/.
# Instância global do serviço (deve vir DEPOIS da definição da classe)
firebase_service = FirebaseService()

# --- FUNÇÃO AUXILIAR ATUALIZADA ---
# A views.py chama esta função. Agora, ela usará a nova lógica.
# --- FUNÇÃO AUXILIAR GLOBAL ---
def get_firebase_sensor_data() -> Optional[Dict[str, float]]:
    """
    Função auxiliar para obter dados dos sensores do Firebase.
    """
    try:
        data = firebase_service.get_latest_reading_from_leituras()
        
        if data:
            return data
        else:
            firebase_logger.warning("Fallback: Nenhum dado retornado do Firebase. Usando dados de exemplo.")
            return {
                'Coliformes': 30.96, 'pH': 8.9, 'DBO': 6.06, 'NT': 2.10, 'FT': 0.22,
                'Temperatura': 25.0, 'Turbidez': 32.62, 'Residuos': 255.75, 'OD': 7.28
            }
    except Exception as e:
        logger.error(f"Erro crítico ao chamar get_latest_reading_from_leituras: {e}", exc_info=True)
        return {
            'Coliformes': 0, 'pH': 7.0, 'DBO': 0, 'NT': 0, 'FT': 0,
            'Temperatura': 25, 'Turbidez': 0, 'Residuos': 0, 'OD': 0
        }

# Instância global do serviço
firebase_service = FirebaseService()

# A função save_firebase_sensor_data não é usada pelo ESP32, mas pode ser mantida
# para salvar dados a partir do Django, se necessário.
def save_firebase_sensor_data(sensor_data: Dict[str, Any], device_id: str = "ESP32_001") -> bool:
    """
    Função auxiliar para salvar dados dos sensores no Firebase (na estrutura antiga /sensors/).
    """
    try:
        # Note que esta função salva no caminho antigo /sensors/
        return firebase_service.save_sensor_data(sensor_data, device_id)
    except Exception as e:
        logger.error(f"Erro ao salvar dados no Firebase: {e}")
        return False