// Pin untuk sensor ultrasonik
const int trigPin = 13;   // Pin trigger
const int echoPin = 12;   // Pin echo

// Pin untuk sensor pH
const int sensorPin = A0;

// Pin untuk relay
#define RELAY_PIN 2       // Pin GPIO untuk relay

// Variabel untuk sensor ultrasonik
long duration;
int distance;

// Variabel untuk sensor pH
float phValue = 0;
const int sampleSize = 10;    // Ukuran sampel untuk rata-rata
float phReadings[sampleSize]; // Array untuk menyimpan sampel pH
int sampleIndex = 0;

// Fungsi untuk membaca nilai pH dengan filter
float readPH(int pin) {
  // Baca nilai tegangan sensor
  int rawValue = analogRead(pin);
  float voltage = 5.0 / 1024.0 * rawValue; // Konversi ke tegangan

  // Hitung nilai pH berdasarkan kalibrasi
  float ph = 7.00 + ((3.69 - voltage) / 0.092);

  // Validasi data: Abaikan nilai tidak masuk akal
  if (ph < 0 || ph > 14) {
    return phValue; // Kembalikan nilai terakhir yang valid
  }

  // Simpan pembacaan ke dalam array untuk moving average
  phReadings[sampleIndex] = ph;
  sampleIndex = (sampleIndex + 1) % sampleSize;

  // Hitung rata-rata dari sampel
  float total = 0;
  for (int i = 0; i < sampleSize; i++) {
    total += phReadings[i];
  }
  return total / sampleSize;
}

void setup() {
  // Inisialisasi pin untuk sensor ultrasonik
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Inisialisasi pin untuk relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Pastikan relay mati saat awal

  // Inisialisasi array sampel dengan nilai awal 7.0 (netral)
  for (int i = 0; i < sampleSize; i++) {
    phReadings[i] = 7.0;
  }

  // Inisialisasi Serial Monitor
  Serial.begin(115200);
  Serial.println("Sistem Siap!");
}

void loop() {
  // === SENSOR ULTRASONIK ===
  // Mengirim sinyal trigger pada sensor ultrasonik
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Membaca sinyal echo dan menghitung durasi pantulnya
  duration = pulseIn(echoPin, HIGH);

  // Hitung jarak (cm)
  if (duration > 0) {
    distance = duration * 0.034 / 2;
    Serial.print("Jarak: ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    distance = -1;  // Menandakan error
    Serial.println("Tidak ada jarak yang terdeteksi.");
  }

  // === SENSOR pH ===
  phValue = readPH(sensorPin); // Membaca pH dengan filter
  Serial.print("Nilai pH: ");
  Serial.println(phValue, 2);

  // === LOGIKA KONTROL RELAY ===
  if (distance > 0 && distance <= 20) {
    // Jarak air di bawah atau sama dengan 20 cm
    Serial.println("Ketinggian air terlalu rendah, pompa mati.");
    digitalWrite(RELAY_PIN, LOW);
  } else if (phValue < 5 || phValue > 9) {
    // Nilai pH di luar rentang 5-9
    Serial.println("pH di luar rentang aman, pompa mati.");
    digitalWrite(RELAY_PIN, LOW);
  } else {
    // Semua kondisi aman
    Serial.println("Kondisi aman, pompa menyala.");
    digitalWrite(RELAY_PIN, HIGH);
  }

  // Jeda pembacaan selama 1 detik
  delay(1000);
}
