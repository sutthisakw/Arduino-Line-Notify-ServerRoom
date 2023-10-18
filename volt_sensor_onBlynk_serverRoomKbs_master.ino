// Last Update: 18/10/2023
// Upload to device: Yes
/*************************************************************
  Blynk is a platform with iOS and Android apps to control
  ESP32, Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build mobile and web interfaces for any
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: https://www.blynk.io
    Sketch generator:           https://examples.blynk.cc
    Blynk community:            https://community.blynk.cc
    Follow us:                  https://www.fb.com/blynkapp
                                https://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on ESP8266 chip.

  NOTE: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right ESP8266 module
  in the Tools -> Board menu!

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/
#include <TridentTD_LineNotify.h>
#include <time.h>
#include <ZMPT101B.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

double voltage;
#define SENSITIVITY 500.0f
ZMPT101B voltageSensor(A0, 50.0);

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID   "xxxxx"
#define BLYNK_TEMPLATE_NAME "xxxxx"
#define BLYNK_AUTH_TOKEN    "xxxxx"
#define LINE_TOKEN          "xxxxx" // token line กลุ่มแจ้งเตือนไฟดับ

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "xxxxx";
char pass[] = "xxxxx";

boolean flag1 = 0;         // เก็บค่าการเริ่มทำงานของ Sensor หลังจากแจ้งไลน์
boolean flag2 = 0;         // เก็บค่าการเริ่มทำงานของ Sensor หลังจากแจ้งไลน์
boolean status_v = 0;      // เก็บค่าการเริมจับเวลาหลังไฟดับ 0=ยังไม่เก็บค่า, 1=เก็บค่าแล้ว
#define ledpin1 D1
#define ledpin2 D2

// กำหนดตัวแปรชั่วโมง
// unsigned long H1 = 00;
// unsigned long H2 = 03;
// unsigned long H3 = 06;
// unsigned long H4 = 9;
unsigned long H5 = 12;
// unsigned long H6 = 15;
// unsigned long H7 = 18;
// unsigned long H8 = 21;

unsigned long Time1 = 0;  // ตัวแปรเก็บค่าเวลา เมื่อไฟดับและบวกเวลาที่ใช้หน่วงอีก 200 (คือ 3 นาที 20 วินาที)
unsigned long time_stop;  // ตัวแปรที่เก็บค่าเวลา ณ ขณะที่ไฟดับ(หน่วยวินาที)
int timezone = 7 * 3600;  // ตั้งค่า TimeZone ตามเวลาประเทศไทย
int dst = 0;              // กำหนดค่า Date Swing Time
int status_wifi;          // ตัวแปรสถานะ WiFi
int wf;                   // ตัวแปรสัญญาณ WiFi

void setup() {
  // Debug console
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  voltageSensor.setSensitivity(SENSITIVITY);
  Serial.println(LINE.getVersion());
  pinMode(ledpin1, OUTPUT);
  pinMode(ledpin2, OUTPUT);
  digitalWrite(ledpin2, HIGH);
  digitalWrite(ledpin1, LOW);
  
  // loop while เช็คการเชื่อมต่อ WiFi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Connecting to WiFi ", ssid);
    delay(500);
  }

  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");  // ดึงเวลาจาก Server
  Serial.println("\nLoading time");
  while (!time(nullptr)) {
    Serial.print("*");
    delay(2000);
  }
  delay(2000);
  Blynk.virtualWrite(V7, 0);      // reset ปุ่ม restart ที่ blynk เป็น 0
}

void loop() {
  status_wifi = WiFi.status();
  // Serial.println("Status WiFi = "+String(status_wifi));
  if (status_wifi != 3) {
    delay(5000);
    ESP.restart();
  }

  Blynk.run();

  wf = WiFi.RSSI();
  Blynk.virtualWrite(V3,wf);  // ส่งค่าความแรงสัญญาณไวไฟไปที่ blynk

  float voltage = voltageSensor.getRmsVoltage();
  Blynk.virtualWrite(V0, voltage);

  // กำหนดตัวแปรเกี่ยวกับเวลา
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  unsigned long time_start = millis()/1000; // เริ่มนับเวลา(หน่วยวินาที)

  //เทสเช็ค led กับแรงดันไฟ
  // if (voltage < 50){                   // เมื่อค่า V น้อยกว่า 50
  //   digitalWrite(ledpin2, HIGH);       // ไฟสีแดงติด
  //   Blynk.virtualWrite(V1, 1);         // ไฟสีแดงที่ blynk ติด
  //   digitalWrite(ledpin1, LOW);        // ไฟสีเขียวดับ
  //   Blynk.virtualWrite(V2, 0);         // ไฟสีเขียวที่ blynk ดับ
  //   Serial.println("Volt = "+String(voltage)+" VAC");
  //   delay(1000);
  // }else{                               // เมื่อค่า V มากกว่า 50
  //   digitalWrite(ledpin1, HIGH);       // ไฟสีเขียวติด
  //   Blynk.virtualWrite(V2, 1);         // ไฟสีเขียวที่ blynk ติด
  //   digitalWrite(ledpin2, LOW);        // ไฟสีแดงดับ
  //   Blynk.virtualWrite(V1, 0);         // ไฟสีแดงที่ blynk ดับ
  //   Serial.println("Volt = "+String(voltage)+" VAC");
  //   delay(1000);
  // }

// Restart อุปกรณ์ทุกวันตอนเวลา 12:00
  if (p_tm->tm_hour == H5 && p_tm->tm_min == 00 && p_tm->tm_sec == 00) {
    delay(5000);
    ESP.restart();
  }
  //   LINE.setToken(LINE_TOKEN);
  //   LINE.notify("เวลา 00:00 น. Device ในห้องเซิร์ฟเวอร์ครบุรียังทำงานปกติ");
  // }else if (p_tm->tm_hour == H2 && p_tm->tm_min == 00 && p_tm->tm_sec == 00) {
  //   LINE.setToken(LINE_TOKEN);
  //   LINE.notify("เวลา 03:00 น. Device ในห้องเซิร์ฟเวอร์ครบุรียังทำงานปกติ");
  // }else if (p_tm->tm_hour == H3 && p_tm->tm_min == 00 && p_tm->tm_sec == 00) {
  //   LINE.setToken(LINE_TOKEN);
  //   LINE.notify("เวลา 06:00 น. Device ในห้องเซิร์ฟเวอร์ครบุรียังทำงานปกติ");
  // }else if (p_tm->tm_hour == H4 && p_tm->tm_min == 00 && p_tm->tm_sec == 00) {
  //   LINE.setToken(LINE_TOKEN);
  //   LINE.notify("เวลา 09:00 น. Device ในห้องเซิร์ฟเวอร์ครบุรียังทำงานปกติ");
  // }else if (p_tm->tm_hour == H5 && p_tm->tm_min == 00 && p_tm->tm_sec == 00) {
  //   LINE.setToken(LINE_TOKEN);
  //   LINE.notify("เวลา 12:00 น. Device ในห้องเซิร์ฟเวอร์ครบุรียังทำงานปกติ");
  // }else if (p_tm->tm_hour == H6 && p_tm->tm_min == 00 && p_tm->tm_sec == 00) {
  //   LINE.setToken(LINE_TOKEN);
  //   LINE.notify("เวลา 15:00 น. Device ในห้องเซิร์ฟเวอร์ครบุรียังทำงานปกติ");
  // }else if (p_tm->tm_hour == H7 && p_tm->tm_min == 00 && p_tm->tm_sec == 00) {
  //   LINE.setToken(LINE_TOKEN);
  //   LINE.notify("เวลา 18:00 น. Device ในห้องเซิร์ฟเวอร์ครบุรียังทำงานปกติ");
  // }else if (p_tm->tm_hour == H8 && p_tm->tm_min == 00 && p_tm->tm_sec == 00) {
  //   LINE.setToken(LINE_TOKEN);
  //   LINE.notify("เวลา 21:00 น. Device ในห้องเซิร์ฟเวอร์ครบุรียังทำงานปกติ");
  // }

  // เมื่อแรงดันไฟฟ้ามากกว่า 50
  if (voltage > 50) {
    digitalWrite(ledpin1, HIGH);  // ไฟสีเขียวติด
    Blynk.virtualWrite(V2, 1);    // ไฟสีเขียวที่ blynk ติด
    digitalWrite(ledpin2, LOW);   // ไฟสีแดงดับ
    Blynk.virtualWrite(V1, 0);    // ไฟสีแดงที่ blynk ดับ
  }

  // เมื่อแรงดันไฟฟ้าต่ำกว่า 50 หรือเมนไฟหลักดับ
  if ((voltage < 50) && flag1 == 0 && status_v == 0) {
    time_stop = time_start;
    Time1 = time_stop + 310; //เก็บค่าเวลาขณะที่ไฟดับ + เวลาที่ใช้จับเวลา
    status_v = 1; // หลังจากไฟดับและเริ่มจับเวลา ณ ขณะที่ไฟดับแล้ว เปลี่ยนค่าเป็น 1
  }

  // ออกจาก loop นับเวลาเมื่อไฟมาก่อน 3:20 วินาที
  if ((voltage > 50) && status_v == 1) {
    status_v = 0;                           // เซ็ต status_v = 0 เพื่อให้ออกจาก loop การนับเวลา
    Time1 = 0;                              // เซ็ต Time1 = 0 เพื่อรอให้เข้าเงื่อนไขนับเวลาใหม่
  }

  // if เช็คว่าไฟดับแล้ว เวลา ณ ขณะนี้ต้องมากกว่าหรือเท่ากับเวลาที่จับไว้ เพื่อให้ทราบว่าเวลาปัจจุบันเกินเวลาที่กำหนดแล้ว ให้แจ้งเตือนไลน์
  if ((voltage < 50) && flag1 == 0 && status_v == 1 && millis()/1000 >= Time1) {
    digitalWrite(ledpin2, HIGH);
    Blynk.virtualWrite(V1, 1);
    digitalWrite(ledpin1, LOW);
    Blynk.virtualWrite(V2, 0);
    LINE.setToken(LINE_TOKEN);
    LINE.notify("ห้องเซิร์ฟเวอร์ครบุรีไฟดับเกินกว่า 5 นาทีและเครื่องปั่นไฟไม่ทำงาน กรุณาตรวจสอบโดยด่วน!");
    flag1 = 1;
    flag2 = 1;
  }

  if ((voltage > 50) && flag1 == 1 && flag2 == 1) {
    LINE.setToken(LINE_TOKEN);
    LINE.notify("ไฟฟ้าห้องเซิร์ฟเวอร์ครบุรีกลับมาใช้งานได้แล้ว");
    flag1 = 0;
    flag2 = 0;
    status_v = 0;
    Time1 = 0;
  }
  delay(1000);
}

// ส่งค่าปุ่ม virtual V7 จาก blynk มา restart อุปกรณ์
BLYNK_WRITE(V7) {
  int pinValue = param.asInt();

  if (pinValue == 1) {
    ESP.restart();
  }
}

