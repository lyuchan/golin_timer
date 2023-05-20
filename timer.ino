/*
這個專案是因為我所工作的地方購買了一個時鐘，原先的使用方式非常不優雅，
因此我逆向了這個商品，使用arduino進行控制，因為我需要使用xlr來做中間的傳輸線，
因此我決定使用dmx這個協議，遠且可以傳送滿多資料的。
*/
//通訊使用
#include <DMXSerial.h>
//顯示器使用spi通訊
#include <SPI.h>
const int dataPin = 10;   //移位寄存器資料
const int clockPin = 11;  //移位寄存器時鐘
const int latchPin = 12;  //栓鎖器
const int ohPin = 9;      //pwm亮度控制

const int buzzer = 8;  //蜂鳴器
//初始化
void setup() {
  //設定腳位輸出
  //Serial.begin(9600);
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(ohPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  //bebe兩聲
  tone(buzzer, 523);
  delay(250);
  noTone(buzzer);
  delay(100);
  tone(buzzer, 523);
  delay(250);
  noTone(buzzer);
  //初始化顯示器
  rst_display();
  display_dimmer(255);
  //初始化通訊
  DMXSerial.init(DMXReceiver);
}
//字體庫
uint16_t lowbyte[10] = { 0xFF00, 0x0000, 0x0F0F, 0x000F, 0xF00F, 0xF00F, 0xFF0F, 0x0000, 0xFF0F, 0xF00F };
uint16_t highbyte[10] = { 0xFFFF, 0xF00F, 0xFFF0, 0xFFFF, 0xF00F, 0x0FFF, 0x0FFF, 0xFF0F, 0xFFFF, 0xFFFF };
//主程式
bool blink = false;
bool dot = false;
void loop() {
  unsigned long lastPacket = DMXSerial.noDataSince();  //未連接通訊時間

  if (lastPacket < 5000) {  //連接正常
    //  analogWrite(RedPin, DMXSerial.read(startChannel));
    digitalWrite(latchPin, LOW);

    dot = (DMXSerial.read(3) > 100);  //中間兩點

    if (DMXSerial.read(1) > 99) {  //左顯示數值
      updateOutput(DMXSerial.read(1) / 10, dot, false);
      updateOutput(DMXSerial.read(1) % 10, dot, false);
    } else {
      updateOutput(DMXSerial.read(1) / 10, dot, true);
      updateOutput(DMXSerial.read(1) % 10, dot, true);
    }
    if (DMXSerial.read(2) > 99) {  //右顯示數值
      updateOutput(DMXSerial.read(2) / 10, dot, false);
      updateOutput(DMXSerial.read(2) % 10, dot, false);
    } else {
      updateOutput(DMXSerial.read(2) / 10, dot, true);
      updateOutput(DMXSerial.read(2) % 10, dot, true);
    }
    digitalWrite(latchPin, HIGH);
    display_dimmer(DMXSerial.read(4));  //亮度


    if (DMXSerial.read(5) > 100) {  //蜂鳴器
      tone(buzzer, 523);
    } else {
      noTone(buzzer);
    }
  } else {  //未連接超過五秒，閃爍提示
    blink = !blink;
    display_dimmer(255);  //亮度
    digitalWrite(latchPin, LOW);
    updateOutput(8, true, blink);
    updateOutput(8, true, blink);
    updateOutput(8, true, blink);
    updateOutput(8, true, blink);
    digitalWrite(latchPin, HIGH);
    delay(250);
  }
}
//顯示數值
void updateOutput(int num, bool dotflag, bool onflag) {
  if (onflag) {
    if (dotflag) {
      shiftOut(dataPin, clockPin, MSBFIRST, (lowbyte[num]) | 0xF0);
      shiftOut(dataPin, clockPin, MSBFIRST, (lowbyte[num] >> 8));
      shiftOut(dataPin, clockPin, MSBFIRST, highbyte[num]);
      shiftOut(dataPin, clockPin, MSBFIRST, (highbyte[num] >> 8));
    } else {
      shiftOut(dataPin, clockPin, MSBFIRST, lowbyte[num]);
      shiftOut(dataPin, clockPin, MSBFIRST, (lowbyte[num] >> 8));
      shiftOut(dataPin, clockPin, MSBFIRST, highbyte[num]);
      shiftOut(dataPin, clockPin, MSBFIRST, (highbyte[num] >> 8));
    }
  } else {
    shiftOut(dataPin, clockPin, MSBFIRST, 0);
    shiftOut(dataPin, clockPin, MSBFIRST, 0);
    shiftOut(dataPin, clockPin, MSBFIRST, 0);
    shiftOut(dataPin, clockPin, MSBFIRST, 0);
  }
}
//清除顯示器
void rst_display() {
  for (int i = 0, i < 16; i++) {
    shiftOut(dataPin, clockPin, MSBFIRST, 0);
  }
}
//亮度
void display_dimmer(int dim) {
  analogWrite(ohPin, 255 - dim);
}
