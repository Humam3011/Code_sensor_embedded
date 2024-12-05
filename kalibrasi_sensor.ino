// === Pin untuk sensor ultrasonik ===
const int trigPin = 13;   // Pin trigger
const int echoPin = 12;   // Pin echo

// === Pin untuk sensor pH ===
const int sensorPin = A0; // Pin analog sensor pH

// === Pin untuk relay ===
#define RELAY_PIN 2       // Pin GPIO untuk relay

// === Variabel untuk sensor ultrasonik ===
long duration;
int distance;

// === Variabel untuk sensor pH ===
float phValue = 0;
const int sampleSize = 10;    // Ukuran sampel untuk rata-rata
float phReadings[sampleSize]; // Array untuk menyimpan sampel pH
int sampleIndex = 0;

// Tinggi wadah
const int tinggiWadah = 19;

// === Library LCD ===
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inisialisasi LCD dengan alamat I2C 0x27 dan ukuran 16x2
const int col = 16;
const int row = 2;
LiquidCrystal_I2C lcd(0x27, col, row);

// === Fungsi membaca nilai pH dengan filter ===
float readPH(int pin) {
  int rawValue = analogRead(pin);
  float voltage = 5.0 / 1024.0 * rawValue;
  float ph = 7.00 + ((3.69 - voltage) / 0.092);

  if (ph < 0 || ph > 14) {
    return phValue;
  }

  phReadings[sampleIndex] = ph;
  sampleIndex = (sampleIndex + 1) % sampleSize;

  float total = 0;
  for (int i = 0; i < sampleSize; i++) {
    total += phReadings[i];
  }
  return total / sampleSize;
}

void setup() {
  // Inisialisasi pin
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Inisialisasi nilai awal sampel pH
  for (int i = 0; i < sampleSize; i++) {
    phReadings[i] = 7.0;
  }

  // Inisialisasi Serial
  Serial.begin(115200);

  // Penundaan untuk memastikan perangkat stabil
  delay(1000);

  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  Serial.println("Sistem Siap!");
}

void loop() {
  // === SENSOR ULTRASONIK ===
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Membaca durasi pantulan
  duration = pulseIn(echoPin, HIGH);
  if (duration > 0) {
    distance = duration * 0.034 / 2; // Konversi ke cm
    Serial.print("Jarak: ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    distance = -1; // Jika tidak ada sinyal pantulan
    Serial.println("Tidak ada jarak yang terdeteksi.");
  }

  // Hitung ketinggian air
  int tinggiAir = (distance > 0) ? tinggiWadah - distance : -1;

  // === SENSOR pH ===
  phValue = readPH(sensorPin);
  Serial.print("Nilai pH: ");
  Serial.println(phValue, 2);

  // === LOGIKA KONTROL RELAY ===
  String pumpStatus = "Mati"; // Status default pompa
  if (phValue >= 5 && phValue <= 9) { // Rentang pH aman
    if (tinggiAir >= 0 && tinggiAir <= 14) {
      // Pompa hidup jika ketinggian air belum penuh
      Serial.println("Kondisi aman, pompa hidup.");
      digitalWrite(RELAY_PIN, HIGH);
      pumpStatus = "Hidup";
    } else {
      // Pompa mati jika air penuh
      Serial.println("Air penuh, pompa mati.");
      digitalWrite(RELAY_PIN, LOW);
    }
  } else {
    // Pompa mati jika pH tidak aman
    Serial.println("pH tidak aman, pompa mati.");
    digitalWrite(RELAY_PIN, LOW);
  }

  // === TAMPILKAN DATA PADA LCD ===
  lcd.clear();

  // Baris 1: pH dan Status Pompa
  lcd.setCursor(0, 0);
  lcd.print("pH : ");
  lcd.print(phValue, 2);
  lcd.print(" ");
  lcd.print(pumpStatus);

  // Baris 2: Ketinggian Air dan Status
  lcd.setCursor(0, 1);
  if (tinggiAir >= 0) {
    lcd.print("Air: ");
    lcd.print(tinggiAir);
    lcd.print(" cm ");
    lcd.print((tinggiAir <= 14) ? "Aman" : "Penuh");
  } else {
    lcd.print("Air: N/A");
  }

  // Jeda pembacaan selama 1 detik
  delay(1000);
}
