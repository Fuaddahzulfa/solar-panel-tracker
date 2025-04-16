#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// Ganti dengan kredensial WiFi kamu
const char* ssid = "Redmi Note 5";
const char* password = "1sampek8";

// Ubidots MQTT broker
const char* mqtt_server = "industrial.api.ubidots.com";
const int mqtt_port = 1883;

// Ganti dengan TOKEN dari akun Ubidots kamu
const char* TOKEN = "BBUS-VKMrY9nBvYdn5QpmgN2jsZQ6beUh3E"; 
const char* DEVICE_LABEL = "solar-panel-tracker"; // nama device di ubidots

const char* VARIABLE_SERVO_H = "servo_h";
const char* VARIABLE_SERVO_V = "servo_v";

// Pin LDR
int ldrAtas = 33;
int ldrKanan  = 32;
int ldrKiri = 34;
int ldrBawah  = 35;

// Servo
int pinServo1 = 25;  // MG90S
int pinServo2 = 26;  // MG996R

Servo servo1;
Servo servo2;

int posisiServo1 = 90;
int posisiServo2 = 90;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void reconnect() {
  // Loop sampai koneksi berhasil
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

void updateServoPosition() {
  // Gerakkan servo ke posisi yang sesuai
  servo1.write(posisiServo1);
  servo2.write(posisiServo2);

  char payload[100];
  sprintf(payload, "{\"%s\": %d, \"%s\": %d}", VARIABLE_SERVO_H, posisiServo2, VARIABLE_SERVO_V, posisiServo1);


  // Mengirimkan data ke Ubidots
  String topic = "/v1.6/devices/" + String(DEVICE_LABEL);
  client.publish(topic.c_str(), payload);
}


void setup() {
  Serial.begin(115200);
  servo1.attach(pinServo1);
  servo2.attach(pinServo2);
  servo1.write(posisiServo1);
  servo2.write(posisiServo2);

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
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Baca nilai LDR
  int nilaiKanan = analogRead(ldrKanan);
  int nilaiAtas  = analogRead(ldrAtas);
  int nilaiBawah = analogRead(ldrBawah);
  int nilaiKiri  = analogRead(ldrKiri);

  Serial.println("---- LDR Values ----");
  Serial.print("Kanan: "); Serial.print(nilaiKanan);
  Serial.print(" | Kiri: "); Serial.print(nilaiKiri);
  Serial.print(" | Atas: "); Serial.print(nilaiAtas);
  Serial.print(" | Bawah: "); Serial.println(nilaiBawah);

  // =====================
  // Prioritas jika ada cahaya sangat terang (> 4000)
  // =====================
  if (nilaiKanan > 4000) posisiServo1 = constrain(posisiServo1 - 2, 0, 180);
  else if (nilaiKiri > 4000) posisiServo1 = constrain(posisiServo1 + 2, 0, 180);

  if (nilaiAtas > 4000) posisiServo2 = constrain(posisiServo2 - 2, 0, 180);
  else if (nilaiBawah > 4000) posisiServo2 = constrain(posisiServo2 + 2, 0, 180);

  // =====================
  // Kalau tidak ada yang melebihi 4000, lanjut gerak normal halus
  // =====================
  if (nilaiKanan <= 4000 && nilaiKiri <= 4000) {
    int diff1 = nilaiKanan - nilaiKiri;
    if (diff1 > 20) posisiServo1 -= 5;
    else if (diff1 < -20) posisiServo1 += 5;
    else {
      if (nilaiKanan > nilaiKiri) posisiServo1 -= 5;
      else if (nilaiKiri > nilaiKanan) posisiServo1 += 5;
    }
  }

  if (nilaiAtas <= 4000 && nilaiBawah <= 4000) {
    int diff2 = nilaiAtas - nilaiBawah;
    if (diff2 > 20) posisiServo2 -= 10;
    else if (diff2 < -20) posisiServo2 += 10;
    else {
      if (nilaiAtas > nilaiBawah) posisiServo2 -= 10;
      else if (nilaiBawah > nilaiAtas) posisiServo2 += 10;
    }
  }

  // Tulis ke servo
  posisiServo1 = constrain(posisiServo1, 0, 180);
  posisiServo2 = constrain(posisiServo2, 0, 180);

  updateServoPosition();

  delay(100);  // Smooth delay
}
