class HidraDashboard {
    constructor() {
        this.updateInterval = 300000; // 5 minutos
        this.chart = null;
        this.map = null;
        this.sensorData = {};

        this.init();
    }

    init() {
        this.initMap();
        this.initChart();
        this.startAutoUpdate();
        this.bindEvents();
        this.animateCards();

        // Inicializar AOS se disponível
        if (typeof AOS !== 'undefined') {
            AOS.init();
        }
    }

    initMap() {
        // Coordenadas atualizadas para Canal de Gaibu
        this.map = L.map('map').setView([-8.343516334550308, -34.94783081164153], 16);

        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            attribution: '© OpenStreetMap contributors'
        }).addTo(this.map);

        // Adicionar marcador do sensor com informações atualizadas
        const sensorMarker = L.marker([-8.343516334550308, -34.94783081164153]).addTo(this.map);
        sensorMarker.bindPopup(`
            <div class="sensor-popup">
                <h5><b>Sensor de Qualidade da Água</b></h5>
                <p><strong>Localização:</strong> Canal de Gaibu</p>
                <p><strong>Status:</strong> <span class="status-online">Ativo</span></p>
                <p><strong>Última atualização:</strong> ${new Date().toLocaleString('pt-BR')}</p>
            </div>
        `).openPopup();
    }

    initChart() {
        const ctx = document.getElementById('trendChart').getContext('2d');

        // Dados simulados melhorados para o gráfico (últimas 6 leituras)
        const labels = ['06:00', '09:00', '12:00', '15:00', '18:00', '21:00'];

        this.chart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: labels,
                datasets: [{
                    label: 'Temperatura (°C)',
                    data: [22.1, 23.5, 24.8, 25.2, 24.5, 23.8],
                    borderColor: '#ff6384',
                    backgroundColor: 'rgba(255, 99, 132, 0.2)',
                    borderWidth: 3,
                    fill: true,
                    tension: 0.4,
                    pointBackgroundColor: '#ff6384',
                    pointBorderColor: '#fff',
                    pointBorderWidth: 2,
                    pointRadius: 6
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        display: true,
                        position: 'top',
                        labels: {
                            color: '#333333', // ✅ CORREÇÃO: Era branco, agora é escuro
                            font: {
                                size: 14,
                                family: "'Segoe UI', Tahoma, Geneva, Verdana, sans-serif"
                            }
                        }
                    },
                    tooltip: {
                        backgroundColor: 'rgba(0, 0, 0, 0.8)',
                        titleColor: '#ffffff',
                        bodyColor: '#ffffff',
                        borderColor: '#ff6384',
                        borderWidth: 1
                    }
                },
                scales: {
                    y: {
                        beginAtZero: false,
                        grid: {
                            color: 'rgba(0, 0, 0, 0.1)', // ✅ CORREÇÃO: Grid visível
                            lineWidth: 1
                        },
                        ticks: {
                            color: '#666666', // ✅ CORREÇÃO: Texto escuro visível
                            font: {
                                size: 12
                            }
                        },
                        title: {
                            display: true,
                            text: 'Valores',
                            color: '#333333', // ✅ CORREÇÃO: Título escuro visível
                            font: {
                                size: 14,
                                weight: 'bold'
                            }
                        }
                    },
                    x: {
                        grid: {
                            color: 'rgba(0, 0, 0, 0.1)', // ✅ CORREÇÃO: Grid visível
                            lineWidth: 1
                        },
                        ticks: {
                            color: '#666666', // ✅ CORREÇÃO: Texto escuro visível
                            font: {
                                size: 12
                            }
                        },
                        title: {
                            display: true,
                            text: 'Horário',
                            color: '#333333', // ✅ CORREÇÃO: Título escuro visível
                            font: {
                                size: 14,
                                weight: 'bold'
                            }
                        }
                    }
                },
                elements: {
                    point: {
                        hoverRadius: 8
                    }
                }
            }
        });
    }

    generateTimeLabels(hours) {
        const labels = [];
        const now = new Date();

        for (let i = hours - 1; i >= 0; i--) {
            const time = new Date(now - i * 60 * 60 * 1000);
            labels.push(time.toLocaleTimeString('pt-BR', {
                hour: '2-digit',
                minute: '2-digit'
            }));
        }

        return labels;
    }

    generateRandomData(count, min, max, baseData = null) {
        if (baseData && baseData.length >= count) {
            return baseData.slice(0, count);
        }

        const data = [];
        let lastValue = (min + max) / 2;

        for (let i = 0; i < count; i++) {
            // Simulação de variação suave
            const variation = (Math.random() - 0.5) * 2;
            lastValue += variation;
            lastValue = Math.max(min, Math.min(max, lastValue));
            data.push(Number(lastValue.toFixed(2)));
        }

        return data;
    }

    updateChart(parameter) {
        const chartConfigs = {
            temperature: {
                label: 'Temperatura (°C)',
                data: [22.1, 23.5, 24.8, 25.2, 24.5, 23.8],
                color: '#ff6384'
            },
            ph: {
                label: 'pH',
                data: [7.1, 7.0, 7.2, 7.3, 7.2, 7.1],
                color: '#36a2eb'
            },
            oxygen: {
                label: 'Oxigênio Dissolvido (mg/L)',
                data: [7.2, 6.9, 6.5, 6.3, 6.8, 7.0],
                color: '#4bc0c0'
            },
            turbidity: {
                label: 'Turbidez (NTU)',
                data: [12.5, 14.2, 16.8, 18.1, 15.6, 13.9],
                color: '#9966ff'
            }
        };

        const config = chartConfigs[parameter];
        if (config) {
            // Atualizar labels para horários do dia
            this.chart.data.labels = ['06:00', '09:00', '12:00', '15:00', '18:00', '21:00'];

            this.chart.data.datasets[0] = {
                ...this.chart.data.datasets[0],
                label: config.label,
                data: config.data,
                borderColor: config.color,
                backgroundColor: config.color + '20',
                pointBackgroundColor: config.color
            };

            // Garantir que as cores dos labels permaneçam visíveis após update
            this.chart.options.plugins.legend.labels.color = '#333333';
            this.chart.options.scales.x.ticks.color = '#666666';
            this.chart.options.scales.y.ticks.color = '#666666';
            this.chart.options.scales.x.title.color = '#333333';
            this.chart.options.scales.y.title.color = '#333333';

            this.chart.update('active');
        }

        // Atualizar botões ativos
        document.querySelectorAll('.chart-btn').forEach(btn => {
            btn.classList.remove('active');
        });

        // Encontrar e ativar o botão correto
        const activeBtn = document.querySelector(`[onclick*="${parameter}"]`);
        if (activeBtn) {
            activeBtn.classList.add('active');
        }
    }

    async updateDashboardData() {
        try {
            const response = await fetch('/api/dashboard/');

            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }

            const data = await response.json();

            if (data.success) {
                this.updateSensorCards(data.sensor_data);
                this.updateIQADisplay(data.iqa);
                this.updateStatusCards(data);
                this.updateLastUpdateTime();
                this.updateMapPopup();

                console.log('Dashboard atualizado com sucesso');
            } else {
                console.error('Erro na resposta da API:', data.error);
                this.showError('Erro ao carregar dados dos sensores');
            }

        } catch (error) {
            console.error('Erro ao atualizar dashboard:', error);
            this.showError('Erro de conexão. Tentando novamente...');
        }
    }

    updateSensorCards(sensorData) {
        const mapping = {
            'temperatura': 'temperatura',
            'ph': 'ph',
            'oxigenio': 'oxigenio',
            'dbo': 'dbo',
            'coliformes': 'coliformes',
            'nitrogenio': 'nitrogenio',
            'fosforo': 'fosforo',
            'turbidez': 'turbidez',
            'solidos': 'solidos'
        };

        Object.entries(mapping).forEach(([key, dataKey]) => {
            // Buscar por diferentes seletores possíveis
            let valueElement = document.querySelector(
                `.sensor-card:has(.sensor-name:contains("${this.getSensorDisplayName(key)}")) .sensor-value`
            );

            // Fallback para busca mais genérica
            if (!valueElement) {
                const cards = document.querySelectorAll('.sensor-card');
                cards.forEach(card => {
                    const nameElement = card.querySelector('.sensor-name');
                    if (nameElement && nameElement.textContent.toLowerCase().includes(key.toLowerCase())) {
                        valueElement = card.querySelector('.sensor-value');
                    }
                });
            }

            if (valueElement && sensorData[dataKey] !== undefined) {
                const value = sensorData[dataKey];
                const formattedValue = this.formatSensorValue(key, value);
                valueElement.textContent = formattedValue;

                // Atualizar classes de alerta se necessário
                this.updateSensorAlert(valueElement.closest('.sensor-card'), key, value);
            }
        });
    }

    getSensorDisplayName(key) {
        const names = {
            'temperatura': 'Temperatura',
            'ph': 'pH',
            'oxigenio': 'Oxigênio',
            'dbo': 'DBO',
            'coliformes': 'Coliformes',
            'nitrogenio': 'Nitrogênio',
            'fosforo': 'Fósforo',
            'turbidez': 'Turbidez',
            'solidos': 'Sólidos'
        };
        return names[key] || key;
    }

    formatSensorValue(sensor, value) {
        const formatters = {
            'temperatura': val => val.toFixed(1),
            'ph': val => val.toFixed(1),
            'oxigenio': val => val.toFixed(1),
            'dbo': val => val.toFixed(1),
            'coliformes': val => Math.round(val),
            'nitrogenio': val => val.toFixed(2),
            'fosforo': val => val.toFixed(3),
            'turbidez': val => val.toFixed(1),
            'solidos': val => Math.round(val)
        };

        return formatters[sensor] ? formatters[sensor](value) : value.toString();
    }

    updateSensorAlert(cardElement, sensor, value) {
        if (!cardElement) return;

        // Remove classes de alerta existentes
        cardElement.classList.remove('alert', 'danger');

        // Definir limites para alertas
        const limits = {
            'temperatura': { warning: 30, critical: 35 },
            'ph': { warning_low: 6.5, warning_high: 8.5, critical_low: 6, critical_high: 9 },
            'oxigenio': { warning: 6, critical: 4 },
            'dbo': { warning: 5, critical: 10 },
            'coliformes': { warning: 100, critical: 1000 },
            'nitrogenio': { warning: 2.18, critical: 3.7 },
            'fosforo': { warning: 0.1, critical: 0.15 },
            'turbidez': { warning: 40, critical: 100 },
            'solidos': { warning: 300, critical: 500 }
        };

        const limit = limits[sensor];
        if (!limit) return;

        let alertClass = '';

        if (sensor === 'ph') {
            if (value < limit.critical_low || value > limit.critical_high) {
                alertClass = 'danger';
            } else if (value < limit.warning_low || value > limit.warning_high) {
                alertClass = 'alert';
            }
        } else if (sensor === 'oxigenio') {
            if (value < limit.critical) {
                alertClass = 'danger';
            } else if (value < limit.warning) {
                alertClass = 'alert';
            }
        } else {
            if (limit.critical && value > limit.critical) {
                alertClass = 'danger';
            } else if (limit.warning && value > limit.warning) {
                alertClass = 'alert';
            }
        }

        if (alertClass) {
            cardElement.classList.add(alertClass);
        }
    }

    updateIQADisplay(iqaData) {
        const iqaValueElement = document.querySelector('.iqa-value');
        const iqaClassificationElement = document.querySelector('.iqa-classification');
        const iqaDisplayElement = document.querySelector('.iqa-display');

        if (iqaValueElement) iqaValueElement.textContent = iqaData.valor;
        if (iqaClassificationElement) iqaClassificationElement.textContent = iqaData.classificacao;

        if (iqaDisplayElement) {
            // Remove classes CSS anteriores
            iqaDisplayElement.className = 'iqa-display ' + iqaData.css_class;
        }
    }

    updateStatusCards(data) {
        // Atualizar card de qualidade da água
        const qualityCard = document.querySelector('.status-card:not(.warning)');
        if (qualityCard) {
            const indicator = qualityCard.querySelector('.status-indicator');
            const value = qualityCard.querySelector('.status-value');

            if (indicator) indicator.className = 'status-indicator ' + data.iqa.css_class;
            if (value) {
                value.innerHTML = `
                    <span class="status-indicator ${data.iqa.css_class}"></span>
                    ${data.iqa.classificacao}
                `;
            }
        }

        // Atualizar card de risco de enchente
        const floodCard = document.querySelector('.status-card.warning, .status-card:has([class*="flood"])');
        if (floodCard && data.flood_risk) {
            const indicator = floodCard.querySelector('.status-indicator');
            const value = floodCard.querySelector('.status-value');

            if (indicator) indicator.className = 'status-indicator ' + data.flood_risk.css_class;
            if (value) {
                value.innerHTML = `
                    <span class="status-indicator ${data.flood_risk.css_class}"></span>
                    ${data.flood_risk.level}
                `;
            }
        }
    }

    updateMapPopup() {
        // Atualizar popup do mapa com horário atual
        const marker = this.map.eachLayer(layer => {
            if (layer instanceof L.Marker) {
                layer.setPopupContent(`
                    <div class="sensor-popup">
                        <h5><b>Sensor de Qualidade da Água</b></h5>
                        <p><strong>Localização:</strong> Canal de Gaibu</p>
                        <p><strong>Status:</strong> <span class="status-online">Ativo</span></p>
                        <p><strong>Última atualização:</strong> ${new Date().toLocaleString('pt-BR')}</p>
                    </div>
                `);
            }
        });
    }

    updateLastUpdateTime() {
        const lastUpdateElement = document.querySelector('.last-update');
        if (lastUpdateElement) {
            const now = new Date();
            const formattedTime = now.toLocaleString('pt-BR');
            lastUpdateElement.innerHTML = `
                <i class="fas fa-clock"></i> Última atualização: ${formattedTime}
            `;
        }
    }

    showError(message) {
        console.error(message);

        // Criar ou atualizar elemento de erro
        let errorElement = document.querySelector('.error-message');
        if (!errorElement) {
            errorElement = document.createElement('div');
            errorElement.className = 'error-message alert alert-danger';
            errorElement.style.cssText = `
                position: fixed;
                top: 20px;
                right: 20px;
                z-index: 9999;
                padding: 10px 15px;
                border-radius: 5px;
                background-color: #f8d7da;
                color: #721c24;
                border: 1px solid #f5c6cb;
                display: none;
            `;
            document.body.appendChild(errorElement);
        }

        errorElement.textContent = message;
        errorElement.style.display = 'block';

        setTimeout(() => {
            errorElement.style.display = 'none';
        }, 5000);
    }

    animateCards() {
        const cards = document.querySelectorAll('.card, .sensor-card');
        cards.forEach((card, index) => {
            card.style.opacity = '0';
            card.style.transform = 'translateY(20px)';
            setTimeout(() => {
                card.style.transition = 'all 0.6s ease';
                card.style.opacity = '1';
                card.style.transform = 'translateY(0)';
            }, index * 100);
        });
    }

    startAutoUpdate() {
        // Atualização automática a cada 5 minutos
        setInterval(() => {
            this.updateDashboardData();
        }, this.updateInterval);

        // Primeira atualização após 10 segundos
        setTimeout(() => {
            this.updateDashboardData();
        }, 10000);
    }

    bindEvents() {
        // Bind de eventos para botões do gráfico
        document.querySelectorAll('.chart-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                e.preventDefault();

                // Extrair parâmetro do onclick ou data attribute
                let parameter = '';
                const onclickAttr = e.target.getAttribute('onclick');
                if (onclickAttr) {
                    const match = onclickAttr.match(/'([^']+)'/);
                    if (match) parameter = match[1];
                }

                if (!parameter) {
                    parameter = e.target.getAttribute('data-parameter');
                }

                if (parameter) {
                    this.updateChart(parameter);
                }
            });
        });

        // Evento para atualização manual
        document.addEventListener('keydown', (e) => {
            if (e.key === 'F5' || (e.ctrlKey && e.key === 'r')) {
                e.preventDefault();
                this.updateDashboardData();
            }
        });

        // Evento para refresh button, se existir
        const refreshBtn = document.querySelector('.refresh-btn, [data-action="refresh"]');
        if (refreshBtn) {
            refreshBtn.addEventListener('click', (e) => {
                e.preventDefault();
                this.updateDashboardData();
            });
        }
    }
}

// Função global para compatibilidade com template
function updateChart(parameter) {
    if (window.hidraDashboard) {
        window.hidraDashboard.updateChart(parameter);
    }
}

// Inicializar dashboard quando o DOM estiver pronto
document.addEventListener('DOMContentLoaded', function () {
    window.hidraDashboard = new HidraDashboard();
});

// Executar animação quando a página carregar completamente
window.addEventListener('load', function () {
    if (window.hidraDashboard) {
        window.hidraDashboard.animateCards();
    }
});