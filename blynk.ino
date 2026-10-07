// 3 บรรทัดนี้ "ต้อง" อยู่บนสุด ก๊อปปี้มาจากหน้าเว็บ Blynk (แถบ Device Info)
#define BLYNK_TEMPLATE_ID "TMPL6WUo0uSAj"
#define BLYNK_TEMPLATE_NAME "Auoto Pump"
#define BLYNK_AUTH_TOKEN "oHw_Ua9thjdnEYmstJutbvbIO95rUPXF"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------- ตั้งค่า Wi-Fi ----------------
char ssid[] = "kk";      // เปลี่ยนเป็นชื่อ Wi-Fi (รองรับแค่ 2.4GHz)
char pass[] = "000000kk";      // เปลี่ยนเป็นรหัสผ่าน Wi-Fi

// ---------------- กำหนดขาใช้งานฮาร์ดแวร์ ----------------
const int moisturePin = 34;
const int potPin = 35;
const int relayPin = 16; // ใช้ขา 16 ตามที่แก้ปัญหาไปล่าสุด

LiquidCrystal_I2C lcd(0x27, 16, 2); 
BlynkTimer timer; // ใช้ Timer แทน delay() เพื่อไม่ให้เน็ตหลุด

// ฟังก์ชันนี้จะทำงานทุกๆ 1 วินาที (ตามที่ตั้งเวลาไว้ใน setup)
void sensorRoutine() {
  int moistureRaw = analogRead(moisturePin);
  int moisturePercent = map(moistureRaw, 4095, 0, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100); 

  int potRaw = analogRead(potPin);
  int setPoint = map(potRaw, 0, 4095, 0, 100);

  lcd.setCursor(0, 0);
  lcd.print("Moist: ");
  lcd.print(moisturePercent);
  lcd.print("%   "); 

  lcd.setCursor(0, 1);
  lcd.print("Set: ");
  lcd.print(setPoint);
  lcd.print("% P:");

  int pumpStatus = 0; // ตัวแปรเก็บสถานะปั๊มสำหรับส่งขึ้น Blynk

  // เงื่อนไขการสั่งปั๊มน้ำแบบ Low Level Trigger + Open Drain
  if (moisturePercent < setPoint) {
    digitalWrite(relayPin, LOW);  // เปิดปั๊ม
    lcd.print("ON ");
    pumpStatus = 1;
  } else {
    digitalWrite(relayPin, HIGH); // ปิดปั๊ม
    lcd.print("OFF");
    pumpStatus = 0;
  }

  // ส่งข้อมูลขึ้น Blynk ผ่าน Virtual Pins
  Blynk.virtualWrite(V0, moisturePercent); // ส่งความชื้นไป V0
  Blynk.virtualWrite(V1, setPoint);        // ส่งค่าที่ตั้งไว้ไป V1
  Blynk.virtualWrite(V2, pumpStatus);      // ส่งสถานะปั๊ม (0 หรือ 1) ไป V2
}

void setup() {
  Serial.begin(115200);

  // ตั้งค่าขา Relay เป็น Open Drain และปิดไว้ก่อน
  pinMode(relayPin, OUTPUT_OPEN_DRAIN); 
  digitalWrite(relayPin, HIGH); 

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  // เชื่อมต่อ Wi-Fi และ Blynk เซิร์ฟเวอร์
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  lcd.clear();
  lcd.print("Blynk Connected!");
  delay(1500);
  lcd.clear();

  // สั่งให้ฟังก์ชัน sensorRoutine ทำงานทุกๆ 1000 มิลลิวินาที (1 วินาที)
  timer.setInterval(1000L, sensorRoutine);
}

void loop() {
  // ใน loop ห้ามมีคำสั่ง delay() เด็ดขาด ปล่อยให้ 2 บรรทัดนี้จัดการตัวเอง
  Blynk.run();
  timer.run();
}