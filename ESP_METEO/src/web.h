#pragma once
#include <pgmspace.h>

const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ESP32 Live Weather Charts</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body {
      background:#111;
      color:#eee;
      font-family:Arial, sans-serif;
      text-align:center;
      margin:0;
      padding:20px;
    }
    h2 { color:#00ffff; }
    canvas {
      max-width:90%;
      margin:20px auto;
      display:block;
      background:#222;
      border-radius:10px;
      padding:10px;
    }
    button {
      background:#00ffff;
      color:#111;
      border:none;
      border-radius:8px;
      padding:10px 20px;
      cursor:pointer;
      font-weight:bold;
      margin-top:10px;
    }
    button:hover { background:#00cccc; }
  </style>
</head>
<body>
  <h2>🌤️ ESP32 Live Weather</h2>

  <!-- графік температури й тиску -->
  <canvas id="lineChart"></canvas>

  <!-- кругова діаграма -->
  <canvas id="pieChart"></canvas>

  <button onclick="refreshNow()">🔄 Refresh Now</button>

  <script>
    // ======== 1️⃣ Ініціалізація масивів даних ========
    const timeLabels = [];
    const temperatures = [];
    const pressures = [];

    // ======== 2️⃣ Створення лінійного графіка ========
    const ctxLine = document.getElementById('lineChart').getContext('2d');
    const lineChart = new Chart(ctxLine, {
      type: 'line',
      data: {
        labels: timeLabels,
        datasets: [
          {
            label: 'Temperature (°C)',
            borderColor: '#00ffff',
            data: [],
            tension: 0.3,
            yAxisID: 'y'
          },
          {
            label: 'Pressure (hPa)',
            borderColor: '#ffaa00',
            data: [],
            tension: 0.3,
            yAxisID: 'y1'
          }
        ]
      },
      options: {
        scales: {
          y: {
            position: 'left',
            title: { display: true, text: 'Temperature °C' },
            grid: { color: 'rgba(255,255,255,0.1)' }
          },
          y1: {
            position: 'right',
            title: { display: true, text: 'Pressure hPa' },
            grid: { drawOnChartArea: false }
          },
          x: {
            title: { display: true, text: 'Time' },
            ticks: { color: '#ccc' }
          }
        },
        plugins: {
          legend: { labels: { color: '#fff' } },
          title: {
            display: true,
            text: 'Live Temperature & Pressure',
            color: '#00ffff'
          }
        }
      }
    });

    // ======== 3️⃣ Кругова діаграма (розподіл температури) ========
    const ctxPie = document.getElementById('pieChart').getContext('2d');
    const pieChart = new Chart(ctxPie, {
      type: 'pie',
      data: {
        labels: ['Cool (<23.5°C)', 'Warm (≥23.5°C)'],
        datasets: [{
          data: [0, 0],
          backgroundColor: ['#0077ff','#ff6600']
        }]
      },
      options: {
        plugins: {
          legend: {
            position: 'bottom',
            labels: { color: '#eee' }
          },
          title: {
            display: true,
            text: 'Temperature Distribution',
            color: '#00ffff'
          }
        }
      }
    });

    // ======== 4️⃣ Отримання даних із ESP32 ========
    async function fetchData() {
      try {
        const response = await fetch('/data.json'); // запитуємо дані з ESP32
        const data = await response.json();         // отримуємо JSON: {temp:..., press:...}

        const temp = data.temp;
        const press = data.press;
        const now = new Date().toLocaleTimeString().slice(0,5); // поточний час

        // додаємо у масиви
        timeLabels.push(now);
        temperatures.push(temp);
        pressures.push(press);

        // обмежуємо кількість точок (щоб не перевантажити)
        if (timeLabels.length > 50) {
          timeLabels.shift();
          temperatures.shift();
          pressures.shift();
        }

        // оновлюємо лінійний графік
        lineChart.data.labels = timeLabels;
        lineChart.data.datasets[0].data = temperatures;
        lineChart.data.datasets[1].data = pressures;
        lineChart.update();

        // оновлюємо кругову діаграму
        const warm = temperatures.filter(t => t >= 23.5).length;
        const cool = temperatures.length - warm;
        pieChart.data.datasets[0].data = [cool, warm];
        pieChart.update();

      } catch (err) {
        console.error("Failed to fetch data:", err);
      }
    }

    // ======== 5️⃣ Кнопка ручного оновлення ========
    function refreshNow() {
      fetchData();
    }

    // ======== 6️⃣ Автоматичне оновлення кожні 5 секунд ========
    setInterval(fetchData, 5000);
    fetchData(); // перший запит при завантаженні сторінки
  </script>

</body>
</html>
)rawliteral";
