#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

const char* ssid = "Tracker";
const char* password = "rplmaalma";

// Ubidots MQTT broker
const char* mqtt_server = "industrial.api.ubidots.com";
const int mqtt_port = 1883;

const char* TOKEN = "BBUS-VKMrY9nBvYdn5QpmgN2jsZQ6beUh3E";
const char* DEVICE_LABEL = "solar-panel-tracker";

const char* VARIABLE_SERVO_H = "servo_h";
const char* VARIABLE_SERVO_V = "servo_v";
const char* VARIABLE_LDR = "ldr";

// Pin LDR
int ldrAtas = 35;
int ldrKanan = 33;
int ldrKiri = 34;
int ldrBawah = 32;

// Servo
int pinServo1 = 25;
int pinServo2 = 26;

Servo servo1;
Servo servo2;

int posisiServo1 = 90;
int posisiServo2 = 90;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

int update = 0;

// Parameter untuk pergerakan halus
const int TOLERANCE = 50;   // Toleransi perbedaan sensor
const int STEP_SMALL = 3;   // Langkah kecil
const int STEP_MEDIUM = 7;  // Langkah sedang
const int STEP_LARGE = 10;  // Langkah besar

void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT... ");
    if (client.connect("ESP32Client", TOKEN, "")) {
      Serial.println("terhubung!");
    } else {
      Serial.print("gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void updateDashboard(float sensor_mean) {
  if (update <= 0) {
    if (!client.connected()) {
      reconnect();
    }
    String payload =
      "{\"" + String(VARIABLE_SERVO_H) + "\": " + String(posisiServo2) + ", \"" + String(VARIABLE_SERVO_V) + "\": " + String(posisiServo1) + ", \"" + String(VARIABLE_LDR) + "\": " + String(sensor_mean) + "}";

    Serial.println(payload);

    String topic = "/v1.6/devices/" + String(DEVICE_LABEL);
    client.publish(topic.c_str(), payload.c_str());
    update = 50;  // Update setiap 5 detik
  } else {
    update--;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(21, OUTPUT);

  servo1.attach(pinServo1);
  servo2.attach(pinServo2);
  servo1.write(posisiServo1);
  servo2.write(posisiServo2);
  delay(500);

  // Koneksi ke WiFi
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTerhubung ke WiFi");

  // Set MQTT server dan port
  client.setServer(mqtt_server, mqtt_port);

  Serial.println("Solar Panel Tracker siap!");
}

void loop() {
  digitalWrite(18, LOW);
  digitalWrite(19, LOW);
  digitalWrite(21, LOW);

  int pinLed = 18 + (update % 3);

  digitalWrite(pinLed == 20 ? 21 : pinLed, HIGH);

  // Baca nilai LDR
  int nilaiKanan = analogRead(ldrKanan);
  int nilaiAtas = analogRead(ldrAtas);
  int nilaiBawah = analogRead(ldrBawah);
  int nilaiKiri = analogRead(ldrKiri);

  Serial.println("---- LDR Values ----");
  Serial.print("Atas: ");
  Serial.print(nilaiAtas);
  Serial.print(" | Bawah: ");
  Serial.print(nilaiBawah);
  Serial.print(" | Kanan: ");
  Serial.print(nilaiKanan);
  Serial.print(" | Kiri: ");
  Serial.print(nilaiKiri);
  Serial.print(" | Servo1: ");
  Serial.print(posisiServo1);
  Serial.print(" | Servo2: ");
  Serial.print(posisiServo2);
  Serial.print(" | Update: ");
  Serial.println(update);

  // Horizontal
  int diffHorizontal = nilaiKanan - nilaiKiri;
  int stepH = hitungStep(diffHorizontal);

  if (stepH > 0) {
    posisiServo1 = constrain(posisiServo1 + (diffHorizontal > 0 ? stepH : -stepH), 0, 179);
  }

  // Vertikal
  int diffVertikal = nilaiAtas - nilaiBawah;
  int stepV = hitungStep(diffVertikal);

  if (stepV > 0) {
    posisiServo2 = constrain(posisiServo2 + (diffVertikal > 0 ? -stepV : stepV), 0, 179);
  }

  // Gerakkan servo ke posisi baru
  servo1.write(posisiServo1);
  servo2.write(posisiServo2);

  // Update dashboard
  float sensor_mean = (nilaiKiri + nilaiKanan + nilaiAtas + nilaiBawah) / 4.0;
  updateDashboard(sensor_mean);

  delay(100);  // Delay yang cukup untuk pergerakan halus
}

int hitungStep(int selisih) {
  int selisihAbs = abs(selisih);
  if (selisihAbs > TOLERANCE * 3) return STEP_LARGE;
  else if (selisihAbs > TOLERANCE * 2) return STEP_MEDIUM;
  else if (selisihAbs > TOLERANCE) return STEP_SMALL;
  else return 0;  // Tidak bergerak
}