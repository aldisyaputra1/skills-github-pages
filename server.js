// server.js
const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const sqlite3 = require('sqlite3').verbose();

const app = express();
const PORT = 3000;

// Middleware
app.use(cors());
app.use(bodyParser.json());

// Inisialisasi database SQLite
const db = new sqlite3.Database('./farm_data.db');

// Buat tabel jika belum ada
db.serialize(() => {
  db.run(`CREATE TABLE IF NOT EXISTS sensor_data (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    temperature REAL,
    humidity REAL,
    soil_moisture INTEGER,
    light_intensity INTEGER,
    device_id TEXT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
  )`);
});

// Endpoint untuk menerima data sensor
app.post('/api/sensor-data', (req, res) => {
  const { temperature, humidity, soil_moisture, light_intensity, device_id } = req.body;
  
  console.log('Received sensor data:', req.body);
  
  // Simpan ke database
  const stmt = db.prepare(`INSERT INTO sensor_data 
    (temperature, humidity, soil_moisture, light_intensity, device_id) 
    VALUES (?, ?, ?, ?, ?)`);
  
  stmt.run([temperature, humidity, soil_moisture, light_intensity, device_id], function(err) {
    if (err) {
      console.error('Database error:', err);
      res.status(500).json({ error: 'Failed to save data' });
    } else {
      res.json({ 
        success: true, 
        message: 'Data saved successfully',
        id: this.lastID 
      });
    }
  });
  
  stmt.finalize();
});

// Endpoint untuk mendapatkan data terbaru
app.get('/api/latest-data', (req, res) => {
  db.get(`SELECT * FROM sensor_data 
          ORDER BY timestamp DESC 
          LIMIT 1`, (err, row) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json(row || {});
    }
  });
});

// Endpoint untuk mendapatkan data historis
app.get('/api/historical-data', (req, res) => {
  const limit = req.query.limit || 100;
  
  db.all(`SELECT * FROM sensor_data 
          ORDER BY timestamp DESC 
          LIMIT ?`, [limit], (err, rows) => {
    if (err) {
      res.status(500).json({ error: err.message });
    } else {
      res.json(rows);
    }
  });
});

app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});