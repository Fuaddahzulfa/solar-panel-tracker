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

int update = 0;

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
  // Gerakkan servo ke posisi yang sesuai
  servo1.write(posisiServo1);
  servo2.write(posisiServo2);

  if (update <= 0){
    String payload = 
      "{\"" + String(VARIABLE_SERVO_H) + "\": " + String(posisiServo2) + 
      ", \"" + String(VARIABLE_SERVO_V) + "\": " + String(posisiServo1) + 
      ", \"" + String(VARIABLE_LDR) + "\": " + String(sensor_mean) + "}";
    
    Serial.println(payload);

    String topic = "/v1.6/devices/" + String(DEVICE_LABEL);
    client.publish(topic.c_str(), payload.c_str());
    update = 30;
  } else update--;
}


void setup() {
  Serial.begin( 115200);
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
  Serial.print(" | Bawah: "); Serial.print(nilaiBawah);
  Serial.print(" | Update: "); Serial.println(update);

  if (nilaiKanan > 4000) posisiServo1 = constrain(posisiServo1 - 2, 0, 180);
  else if (nilaiKiri > 4000) posisiServo1 = constrain(posisiServo1 + 2, 0, 180);

  if (nilaiAtas > 4000) posisiServo2 = constrain(posisiServo2 - 2, 0, 180);
  else if (nilaiBawah > 4000) posisiServo2 = constrain(posisiServo2 + 2, 0, 180);


  if (nilaiKanan <= 4000 && nilaiKiri <= 4000) {
    int diff1 = nilaiKanan - nilaiKiri;
    if (diff1 > 20) posisiServo1 -= 3;
    else if (diff1 < -20) posisiServo1 += 3;
    else {
      if (nilaiKanan > nilaiKiri) posisiServo1 -= 3;
      else if (nilaiKiri > nilaiKanan) posisiServo1 += 3;
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

  posisiServo1 = constrain(posisiServo1, 0, 180);
  posisiServo2 = constrain(posisiServo2, 0, 180);

  updateDashboard((nilaiKiri+nilaiKanan+nilaiAtas+nilaiBawah)/4);

  delay(100);
}
