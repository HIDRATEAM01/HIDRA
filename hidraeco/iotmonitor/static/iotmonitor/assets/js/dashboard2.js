// Inicializar Mapa
let map = L.map('map').setView([-8.343516334550308, -34.94783081164153], 16); 

L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '© OpenStreetMap contributors'
}).addTo(map);

// Adicionar marcador do sensor
let sensorMarker = L.marker([-8.343516334550308, -34.94783081164153]).addTo(map)
    .bindPopup('<b>Sensor de Qualidade da Água</b><br>Canal de Gaibu<br>Status: Ativo')
    .openPopup();

// Dados simulados para o gráfico
const chartData = {
    temperature: {
        labels: ['06:00', '09:00', '12:00', '15:00', '18:00', '21:00'],
        data: [22.1, 23.5, 24.8, 25.2, 24.5, 23.8],
        label: 'Temperatura (°C)',
        color: '#ff6384'
    },
    ph: {
        labels: ['06:00', '09:00', '12:00', '15:00', '18:00', '21:00'],
        data: [7.1, 7.0, 7.2, 7.3, 7.2, 7.1],
        label: 'pH',
        color: '#36a2eb'
    },
    oxygen: {
        labels: ['06:00', '09:00', '12:00', '15:00', '18:00', '21:00'],
        data: [7.2, 6.9, 6.5, 6.3, 6.8, 7.0],
        label: 'Oxigênio Dissolvido (mg/L)',
        color: '#4bc0c0'
    },
    turbidity: {
        labels: ['06:00', '09:00', '12:00', '15:00', '18:00', '21:00'],
        data: [12.5, 14.2, 16.8, 18.1, 15.6, 13.9],
        label: 'Turbidez (NTU)',
        color: '#9966ff'
    }
};

// Configurar gráfico
const ctx = document.getElementById('trendChart').getContext('2d');
let chart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: chartData.temperature.labels,
        datasets: [{
            label: chartData.temperature.label,
            data: chartData.temperature.data,
            borderColor: chartData.temperature.color,
            backgroundColor: chartData.temperature.color + '20',
            borderWidth: 3,
            fill: true,
            tension: 0.4,
            pointBackgroundColor: chartData.temperature.color,
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
                position: 'top'
            }
        },
        scales: {
            y: {
                beginAtZero: false,
                grid: {
                    color: 'rgba(0,0,0,0.1)'
                }
            },
            x: {
                grid: {
                    color: 'rgba(0,0,0,0.1)'
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

// Função para atualizar gráfico
function updateChart(parameter) {
    // Remover classe active de todos os botões
    document.querySelectorAll('.chart-btn').forEach(btn => btn.classList.remove('active'));

    // Adicionar classe active ao botão clicado
    event.target.classList.add('active');

    const data = chartData[parameter];
    chart.data.labels = data.labels;
    chart.data.datasets[0].label = data.label;
    chart.data.datasets[0].data = data.data;
    chart.data.datasets[0].borderColor = data.color;
    chart.data.datasets[0].backgroundColor = data.color + '20';
    chart.data.datasets[0].pointBackgroundColor = data.color;
    chart.update('active');
}

// Animação de entrada para os cards
function animateCards() {
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

// Executar animação quando a página carregar
window.addEventListener('load', animateCards);

// Inicializar AOS se disponível
if (typeof AOS !== 'undefined') {
    AOS.init();
}