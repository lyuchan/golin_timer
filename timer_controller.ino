#include <Wire.h>  //時鐘模組
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <DMXSerial.h>  //dmx通訊
#include <EEPROM.h>     //eeprom
/*
dmx通道協議
1.左顯示數值，超過100不顯示
2.右顯示數值，超過100不顯示
3.中間兩點(超過100啟用)
4.dimmer
5.蜂鳴器(超過100啟用)
*/
#include <RotaryEncoder.h>     //旋轉編碼器
#include <SoftwareSerial.h>    // 軟體串口封包
SoftwareSerial display(6, 7);  //UART螢幕定義
RotaryEncoder encoder(12, 13, RotaryEncoder::LatchMode::TWO03);
#define sw 11  //編碼器按鈕
#define btnext 5
#define btback 4
#define btstart 3
#define btstop 14
#define dmxslc 2
#define ROTARYMIN 0
#define beedelay 1000
ThreeWire myWire(9, 10, 8);  // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
bool setflag = false;
int ROTARYMAX = 100;
int oldpos = -1, pos = 0;
bool mute = false;
uint8_t dimmer = 128;
int timermod = 1;
void setup() {
  DMXSerial.init(DMXController);

  display.begin(9600);
  //Serial.begin(9600);
  Rtc.Begin();
  dimmer = read_dimmer();
  mute = read_mute();
  pinMode(sw, INPUT_PULLUP);
  pinMode(btnext, INPUT_PULLUP);
  pinMode(btback, INPUT_PULLUP);
  pinMode(btstart, INPUT_PULLUP);
  pinMode(btstop, INPUT_PULLUP);
  pinMode(dmxslc, OUTPUT);
  digitalWrite(dmxslc, HIGH);  //console mod
  topage(1);
}
int page = 1;
int slc = 0, oldslc = -1;
int btnum = 0;
int oldbtnum = -1;
int btmax = 0;
int timermm = 0, timerss = 0, oldtimermm = 0, oldtimerss = 0;
bool btnextflag = false;
bool btbackflag = false;
bool timergo = false, oldtimergo = false, timeup = true;
unsigned long time1, time2, time3, time4, beetime;

void bee_loop() {
  if (beetime >= time1) {
    if (mute) {
      DMXSerial.write(1, 0);
    } else {
      DMXSerial.write(1, 255);
    }
  } else {
    DMXSerial.write(5, 0);
  }
}
void loop() {
  RtcDateTime now = Rtc.GetDateTime();
  time1 = millis();

  encoder.tick();
  pos = encoder_loop();
  bt_loop();
  switch (page) {
    case 1:  //clock
      DMXSerial.write(3, 255);
      if ((time1 - time2) > 100) {
        time2 = time1;
        settime(now.Hour(), now.Minute());
        ////Serial.print(now.Hour());
        ////Serial.print(":");
        //Serial.println(now.Minute());
      }
      if (digitalRead(sw) == LOW) {
        delay(20);
        if (digitalRead(sw) == LOW) {
          while (digitalRead(sw) == LOW) { delay(1); }
          page = 2;
          topage(page);
          slc = 0;
          encoder.setPosition(0);
          btnum = 0;
          oldbtnum = -1;
          oldslc = -1;
          setqpicc(6, 3);
        }
      }
      break;
    case 2:  //timer
      DMXSerial.write(3, 255);
      btmax = 4;
      // //Serial.println(btnum);
      if (digitalRead(btstart) == LOW) {
        delay(20);
        if (digitalRead(btstart) == LOW) {
          while (digitalRead(btstart) == LOW) { delay(1); }
          if (!timergo) {

            timergo = true;
            setqpicc(6, 2);
            setqpicc(5, 3);
            if (timeup) {
              if (timermm == 0 && timerss == 0) {
                timermod = 1;
              } else {
                timermod = 2;
              }
              oldtimermm = timermm;
              oldtimerss = timerss;
              timeup = false;
            }
          } else {
            timergo = false;
          }
        }
      }
      if (digitalRead(btstop) == LOW) {
        delay(20);
        if (digitalRead(btstop) == LOW) {
          while (digitalRead(btstop) == LOW) { delay(1); }
          timermm = 0;
          timerss = 0;
          timergo = false;
          timeup = true;
          setqpicc(6, 3);
          setqpicc(5, 2);
          settime(timermm, timerss);
        }
      }
      time3 = millis();
      if (time3 - time4 > 1000) {
        time4 = time3;
        if (timergo) {
          if (timermod == 2) {
            timerss--;
            if (timerss <= -1) {
              timermm--;
              timerss = 59;
            }
            if (timermm <= -1) {
              timermm = 0;
              timerss = 0;
              timergo = false;
              timeup = true;
              setqpicc(6, 3);
              setqpicc(5, 2);
              timermm = oldtimermm;
              timerss = oldtimerss;
              beetime = time1 + beedelay;
            }
          } else {
            timerss++;
            if (timerss >= 60) {
              timermm++;
              timerss = 0;
            }
            if (timermm >= 99) {
              timermm = 0;
              timerss = 0;
              timergo = false;
              timeup = true;
              setqpicc(6, 3);
              setqpicc(5, 2);
              timermm = oldtimermm;
              timerss = oldtimerss;
            }
          }
          settime(timermm, timerss);
        }
      }

      if (!timergo) {
        if (oldbtnum != btnum) {
          oldbtnum = btnum;
          setqpicc(btnum, 3);
          for (int i = 0; i < 5; i++) {
            if (i != btnum) {
              setqpicc(i, 2);
            }
          }
          settime(timermm, timerss);
        }
        if (oldtimergo != timergo) {
          oldtimergo = timergo;
          btnum = 0;
          setqpicc(btnum, 3);
          for (int i = 0; i < 5; i++) {
            if (i != btnum) {
              setqpicc(i, 2);
            }
          }
          settime(timermm, timerss);
        }
        slc = btnum;
        switch (slc) {  //
          case 0:       //第一位數值
            ROTARYMAX = 9;
            if (oldslc != slc) {
              oldslc = slc;
              pos = timermm / 10;
              encoder.setPosition(pos);
              //Serial.print(slc);
              //Serial.print(slc);
            }
            if (oldpos != pos) {
              oldpos = pos;
              timermm = (pos * 10) + timermm % 10;
              settime(timermm, timerss);
            }
            if (digitalRead(sw) == LOW) {
              delay(20);
              if (digitalRead(sw) == LOW) {
                btnum = 1;
                pos = timermm % 10;
                while (digitalRead(sw) == LOW) { delay(1); }
              }
            }

            break;
          case 1:  //第二位數值
            ROTARYMAX = 9;
            if (oldslc != slc) {
              oldslc = slc;
              pos = timermm % 10;
              encoder.setPosition(pos);
              oldpos = -1;
              //Serial.print(slc);
              //Serial.print(slc);
            }
            if (oldpos != pos) {
              oldpos = pos;
              timermm = (timermm / 10) * 10 + pos;
              settime(timermm, timerss);
            }
            if (digitalRead(sw) == LOW) {
              delay(20);
              if (digitalRead(sw) == LOW) {
                btnum = 2;
                while (digitalRead(sw) == LOW) { delay(1); }
              }
            }
            break;
          case 2:  //第三位數值
            ROTARYMAX = 5;
            if (oldslc != slc) {
              oldslc = slc;
              pos = timerss / 10;
              encoder.setPosition(pos);
              oldpos = -1;
              //Serial.print(slc);
              //Serial.print(slc);
            }
            if (oldpos != pos) {
              oldpos = pos;
              timerss = (pos * 10) + timerss % 10;
              settime(timermm, timerss);
            }
            if (digitalRead(sw) == LOW) {
              delay(20);
              if (digitalRead(sw) == LOW) {
                btnum = 3;
                while (digitalRead(sw) == LOW) { delay(1); }
              }
            }
            break;
          case 3:  //第四位數值
            ROTARYMAX = 9;
            if (oldslc != slc) {
              oldslc = slc;
              pos = timerss % 10;
              encoder.setPosition(pos);
              oldpos = -1;
              //Serial.print(slc);
              //Serial.print(slc);
            }
            if (oldpos != pos) {
              oldpos = pos;
              timerss = (timerss / 10) * 10 + pos;
              settime(timermm, timerss);
            }
            if (digitalRead(sw) == LOW) {
              delay(20);
              if (digitalRead(sw) == LOW) {
                btnum = 0;
                while (digitalRead(sw) == LOW) { delay(1); }
              }
            }
            break;
          case 4:
            if (digitalRead(sw) == LOW) {
              delay(20);
              if (digitalRead(sw) == LOW) {
                while (digitalRead(sw) == LOW) { delay(1); }
                page = 3;
                slc = 0;
                topage(page);
                btnum = 0;
                encoder.setPosition(0);
              }
            }
            break;
        }

      } else {
        slc = 0;
        if (oldtimergo != timergo) {
          oldtimergo = timergo;
          for (int i = 0; i < 5; i++) {
            setqpicc(i, 2);
          }
          settime(timermm, timerss);
        }
      }

      break;
    case 3:  //set

      btmax = 2;
      if (oldbtnum != btnum) {
        oldbtnum = btnum;
        switch (btnum) {
          case 0:
            setqpicc(0, 6);
            setqpicc(1, 4);
            setqpicc(3, 4);
            encoder.setPosition(dimmer);
            break;
          case 1:
            setqpicc(0, 4);
            setqpicc(1, 6);
            setqpicc(3, 4);
            if (mute) {
              encoder.setPosition(0);
            } else {
              encoder.setPosition(1);
            }
            break;
          case 2:
            setqpicc(0, 4);
            setqpicc(1, 4);
            setqpicc(3, 6);
            break;
        }
      }
      switch (btnum) {
        case 0:

          DMXSerial.write(1, 88);
          DMXSerial.write(2, 88);
          DMXSerial.write(3, 255);
          DMXSerial.write(4, dimmer);
          ROTARYMAX = 255;
          if (oldpos != pos) {
            oldpos = pos;
            dimmer = pos;
            setnum(0, dimmer);
          }
          if (digitalRead(sw) == LOW) {
            delay(20);
            if (digitalRead(sw) == LOW) {
              while (digitalRead(sw) == LOW) { delay(1); }
              topage(4);  //save
              save_dimmer_to_eeprom(dimmer);
              delay(1000);
              topage(3);
              oldbtnum = -1;
              setnum(0, dimmer);
            }
          }
          break;
        case 1:
          DMXSerial.write(1, 200);
          DMXSerial.write(2, 200);
          DMXSerial.write(3, 0);
          ROTARYMAX = 1;
          if (oldpos != pos) {
            oldpos = pos;
            mute = (pos == 0);
            if (mute) {
              setqpicc(2, 6);
            } else {
              setqpicc(2, 4);
            }
          }
          if (digitalRead(sw) == LOW) {
            delay(20);
            if (digitalRead(sw) == LOW) {
              while (digitalRead(sw) == LOW) { delay(1); }
              topage(4);  //save
              save_mute_to_eeprom(mute);
              delay(500);
              topage(3);
              oldbtnum = -1;
              if (mute) {
                setqpicc(2, 6);
              } else {
                setqpicc(2, 4);
              }
            }
          }
          break;
        case 2:
          DMXSerial.write(1, 200);
          DMXSerial.write(2, 200);
          DMXSerial.write(3, 0);
          if (digitalRead(sw) == LOW) {
            delay(20);
            if (digitalRead(sw) == LOW) {
              while (digitalRead(sw) == LOW) { delay(1); }
              page = 1;
              slc = 0;
              topage(page);
              btnum = 0;
              encoder.setPosition(0);
            }
          }
          break;
      }
      break;
  }
}

void end_screen() {  //結束資料
  display.print("\xff\xff\xff");
}
void topage(int p) {  //變更頁面
  display.print("page " + String(p));
  end_screen();
}
void setqpicc(int q, int pic) {  //切圖 編號，圖片
  display.print("q" + String(q) + ".picc=" + String(pic));
  end_screen();
}
void setnum(int n, int val) {  //數值
  display.print("n" + String(n) + ".val=" + String(val));
  end_screen();
}
void settxt(int txt, int hh, int mm) {
  display.print("t" + String(txt) + ".txt=\"" + String(hh) + ":" + String(mm) + "\"");
  end_screen();
}
void settime(int hh, int mm) {
  String h, m;
  if (hh < 10) {
    h = "0" + String(hh);
  } else {
    h = String(hh);
  }
  if (mm < 10) {
    m = "0" + String(mm);
  } else {
    m = String(mm);
  }
  DMXSerial.write(1, hh);
  DMXSerial.write(2, mm);
  display.print("t0.txt=\"" + h + ":" + m + "\"");
  end_screen();
}
int encoder_loop() {
  int newPos = encoder.getPosition();

  if (newPos < ROTARYMIN) {
    encoder.setPosition(ROTARYMIN);
    newPos = ROTARYMIN;

  } else if (newPos > ROTARYMAX) {
    encoder.setPosition(ROTARYMAX);
    newPos = ROTARYMAX;
  }  // if
  return newPos;
}

void bt_loop() {
  if (digitalRead(btnext) == LOW) {
    if (btnextflag == false) {
      delay(20);
      if (digitalRead(btnext) == LOW) {
        //Serial.println("btnext");
        //Serial.println(btnum);
        btnextflag = true;
        btnum++;
        if (btnum > btmax) {
          btnum = btmax;
        }
      }
    }
  } else {
    btnextflag = false;
  }
  if (digitalRead(btback) == LOW) {
    if (btbackflag == false) {
      delay(20);
      if (digitalRead(btback) == LOW) {
        //Serial.println("btback");
        //Serial.println(btnum);
        btbackflag = true;
        btnum--;
        if (btnum == -1) {
          btnum = 0;
        }
      }
    }
  } else {
    btbackflag = false;
  }
}




void save_dimmer_to_eeprom(int dim) {
  EEPROM.write(10, dim);
}
void save_mute_to_eeprom(bool muteval) {
  if (muteval) {
    EEPROM.write(20, 255);
  } else {
    EEPROM.write(20, 0);
  }
}
int read_dimmer() {
  return EEPROM.read(10);
}
bool read_mute() {
  if (EEPROM.read(20) == 255) {
    return true;
  } else {
    return false;
  }
}
