#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

const char* ssid = "Redmi Note 5";
const char* password = "1sampek8";

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
const int TOLERANCE = 150;    // Toleransi perbedaan sensor
const int STEP_SMALL = 5;     // Langkah kecil
const int STEP_MEDIUM = 10;    // Langkah sedang
const int STEP_LARGE = 15;     // Langkah besar

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
    String payload = 
      "{\"" + String(VARIABLE_SERVO_H) + "\": " + String(posisiServo2) + 
      ", \"" + String(VARIABLE_SERVO_V) + "\": " + String(posisiServo1) + 
      ", \"" + String(VARIABLE_LDR) + "\": " + String(sensor_mean) + "}";
    
    Serial.println(payload);

    String topic = "/v1.6/devices/" + String(DEVICE_LABEL);
    client.publish(topic.c_str(), payload.c_str());
    update = 50; // Update setiap 5 detik
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
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  digitalWrite(18, LOW);
  digitalWrite(19, LOW);
  digitalWrite(21, LOW);

  int pinLed = 18+(update % 3);

  digitalWrite(pinLed == 20? 21:pinLed, HIGH);

  // Baca nilai LDR
  int nilaiKanan = analogRead(ldrKanan);
  int nilaiAtas = analogRead(ldrAtas);
  int nilaiBawah = analogRead(ldrBawah);
  int nilaiKiri = analogRead(ldrKiri);

  Serial.println("---- LDR Values ----");
  Serial.print("Atas: "); Serial.print(nilaiAtas);
  Serial.print(" | Bawah: "); Serial.print(nilaiBawah);
  Serial.print(" | Kanan: "); Serial.print(nilaiKanan);
  Serial.print(" | Kiri: "); Serial.print(nilaiKiri);
  Serial.print(" | Servo1: "); Serial.print(posisiServo1);
  Serial.print(" | Servo2: "); Serial.print(posisiServo2);
  Serial.print(" | Update: "); Serial.println(update);

  // Tracking horizontal (servo1) - kiri/kanan
  int diffHorizontal = nilaiKanan - nilaiKiri;
  
  if (abs(diffHorizontal) > TOLERANCE) {
    int step = STEP_SMALL;
    
    // Tentukan ukuran langkah berdasarkan perbedaan
    if (abs(diffHorizontal) > 500) step = STEP_LARGE;
    else if (abs(diffHorizontal) > 300) step = STEP_MEDIUM;
    
    step += 5;
    
    if (diffHorizontal > 0) {
      // Kanan lebih terang, putar ke kanan
      posisiServo1 = constrain(posisiServo1 + step, 0, 180);
    } else {
      // Kiri lebih terang, putar ke kiri  
      posisiServo1 = constrain(posisiServo1 - step, 0, 180);
    }
  }

  // Tracking vertikal (servo2) - atas/bawah
  int diffVertikal = nilaiAtas - nilaiBawah;
  
  if (abs(diffVertikal) > TOLERANCE) {
    int step = STEP_SMALL;
    
    // Tentukan ukuran langkah berdasarkan perbedaan
    if (abs(diffVertikal) > 500) step = STEP_LARGE;
    else if (abs(diffVertikal) > 300) step = STEP_MEDIUM;
    
    if (diffVertikal > 0) {
      // Atas lebih terang, putar ke atas
      posisiServo2 = constrain(posisiServo2 - step, 0, 180);
    } else {
      // Bawah lebih terang, putar ke bawah
      posisiServo2 = constrain(posisiServo2 + step, 0, 180);
    }
  }

  // Gerakkan servo ke posisi baru
  servo1.write(posisiServo1);
  servo2.write(posisiServo2);

  // Update dashboard
  float sensor_mean = (nilaiKiri + nilaiKanan + nilaiAtas + nilaiBawah) / 4.0;
  updateDashboard(sensor_mean);

  delay(150); // Delay yang cukup untuk pergerakan halus
}