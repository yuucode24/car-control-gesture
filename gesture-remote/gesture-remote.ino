#include <Wire.h>
#include <MPU6050.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

MPU6050 mpu;
const int threshold = 1000;  // Ambang batas perubahan akselerasi sumbu Z

// WiFi
const char* ssid = "Car Control";
const char* serverAddress = "192.168.4.1";  // Alamat IP server tujuan
String currentState = "S";  // State awal: posisi tetap

const int buzzerPin = D8;  // Ganti dengan pin yang sesuai

void setup() {
  Serial.begin(115200);
  

  
  Wire.begin();
  mpu.initialize();
  // Buzzer
  pinMode(buzzerPin, OUTPUT);
  // WiFi
  WiFi.begin(ssid);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    buzzOnce();
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
    buzzTwice(); // Buzzer berbunyi satu kali saat terhubung ke WiFi

}

void loop() {
  // Baca data akselerasi sumbu Y, X, dan Z
  int16_t accelY = mpu.getAccelerationY();
  int16_t accelX = mpu.getAccelerationX();
  int16_t accelZ = mpu.getAccelerationZ();

  // Deteksi posisi tetap berdasarkan akselerasi sumbu Z
  int16_t accelZRef = 16384;  // Nilai akselerasi sumbu Z saat tangan dalam posisi tetap (sekitar 9.8 m/s^2)
  int16_t diffZ = abs(accelZ - accelZRef);

  // Cek posisi tetap atau gerakan berdasarkan perbedaan akselerasi sumbu Z
  if (diffZ < threshold) {
    if (currentState != "S") {  // Mengirim permintaan posisi tetap hanya jika sebelumnya bukan posisi tetap
      Serial.println("Posisi Tetap");
      sendRequest("/?State=S");
      currentState = "S";
    }
  } else {
    // Cek arah maju/mundur berdasarkan sumbu Y
    if (accelY > 10000) {
      if (currentState != "F") {  // Mengirim permintaan maju hanya jika sebelumnya bukan maju
        Serial.println("Maju");
        sendRequest("/?State=F");
        currentState = "F";
      }
    } else if (accelY < -10000) {
      if (currentState != "B") {  // Mengirim permintaan mundur hanya jika sebelumnya bukan mundur
        Serial.println("Mundur");
        sendRequest("/?State=B");
        currentState = "B";
      }
    }

    // Cek arah kanan/kiri berdasarkan sumbu X
    if (accelX > 10000) {
      if (currentState != "R") {  // Mengirim permintaan ke kanan hanya jika sebelumnya bukan ke kanan
        Serial.println("Kanan");
        sendRequest("/?State=R");
        currentState = "R";
      }
    } else if (accelX < -10000) {
      if (currentState != "L") {  // Mengirim permintaan ke kiri hanya jika sebelumnya bukan ke kiri
        Serial.println("Kiri");
        sendRequest("/?State=L");
        currentState = "L";
      }
    }

    // Cek posisi tetap tambahan berdasarkan perbedaan akselerasi sumbu Y
    int16_t diffY = abs(accelY);
    if (diffY < threshold) {
      if (currentState != "S") {  // Mengirim permintaan posisi tetap tambahan hanya jika sebelumnya bukan posisi tetap tambahan
        Serial.println("Posisi Tetap Tambahan");
        sendRequest("/?State=S");
        currentState = "S";
      }
    }
  }

  delay(100);
}

void sendRequest(const char* path) {
  WiFiClient client;
  HTTPClient http;

  Serial.print("Sending HTTP GET request to server...");
  if (http.begin(client, serverAddress, 80, path)) {
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.printf("Response code: %d\n", httpResponseCode);
    } else {
      Serial.printf("Error: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  } else {
    Serial.println("Failed to connect to server");
  }
}

void buzzOnce() {
  digitalWrite(buzzerPin, HIGH);
  delay(100);
  digitalWrite(buzzerPin, LOW);
}

void buzzTwice() {
  buzzOnce();
  delay(200);
  buzzOnce();
}
