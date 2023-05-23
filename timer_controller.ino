#include <Wire.h>  //時鐘模組
#include <ThreeWire.h>
#include <RtcDS1302.h>

#include <RotaryEncoder.h>     //旋轉編碼器
#include <SoftwareSerial.h>    // 軟體串口封包
SoftwareSerial display(6, 7);  //UART螢幕定義
RotaryEncoder encoder(12, 13, RotaryEncoder::LatchMode::TWO03);
#define sw 11  //編碼器按鈕
#define btnext 5
#define btback 4
#define btstart 3
#define btstop 2
#define ROTARYMIN 0
ThreeWire myWire(9, 10, 8);  // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
bool setflag = false;
int ROTARYMAX = 100;
int oldpos = -1, pos = 0;
bool mute = false;
void setup() {
  display.begin(9600);
  Serial.begin(9600);
  Rtc.Begin();
  pinMode(sw, INPUT_PULLUP);
  pinMode(btnext, INPUT_PULLUP);
  pinMode(btback, INPUT_PULLUP);
  pinMode(btstart, INPUT_PULLUP);
  pinMode(btstop, INPUT_PULLUP);
  topage(1);
}
int page = 1;
int slc = 0, oldslc = -1;
int btnum = 0;
int oldbtnum = -1;
int btmax = 0;
int timermm = 0, timerss = 0;
bool btnextflag = false;
bool btbackflag = false;
unsigned long time1, time2;
void loop() {
  RtcDateTime now = Rtc.GetDateTime();
  time1 = millis();
  encoder.tick();
  pos = encoder_loop();
  bt_loop();
  switch (page) {
    case 1:  //clock
      if ((time1 - time2) > 100) {
        time2 = time1;
        settime(now.Hour(), now.Minute());
        Serial.print(now.Hour());
        Serial.print(":");
        Serial.println(now.Minute());
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
        }
      }
      break;
    case 2:  //timer
      btmax = 6;
      // Serial.println(btnum);
      if (oldbtnum != btnum) {
        oldbtnum = btnum;
        setqpicc(btnum, 3);
        for (int i = 0; i < 7; i++) {
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
            Serial.print(slc);
            Serial.print(slc);
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
            Serial.print(slc);
            Serial.print(slc);
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
            Serial.print(slc);
            Serial.print(slc);
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
            Serial.print(slc);
            Serial.print(slc);
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
        case 4:  //start
          break;
        case 5:  //stop
          break;
        case 6:  //next menu
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
      break;
    case 3:  //set
      if (setflag) {
        switch (btnum) {
          case 0:  //dimmer

            ROTARYMAX = 255;
            if (oldpos != pos) {
              oldpos = pos;
              setnum(0, pos);
            }
            break;
          case 1:  //mute
            ROTARYMAX = 1;
            if (oldpos != pos) {
              oldpos = pos;
              mute = (pos == 0);
              if (mute) {
                setqpicc(2, 5);
              } else {
                setqpicc(2, 7);
              }
            }
            break;
        }

        if (digitalRead(sw) == LOW) {
          delay(20);
          if (digitalRead(sw) == LOW) {
            setflag = false;
            while (digitalRead(sw) == LOW) { delay(1); }

            if (btnum == 0) {
              display.print("n0.pco=65535");  //white
              end_screen();
              setqpicc(0, 6);
            } else {
              setqpicc(1, 6);
              if (mute) {
                setqpicc(2, 4);
              } else {
                setqpicc(2, 6);
              }
            }
            btnum = slc;
          }
        }
      } else {
        btmax = 3;
        if (oldbtnum != btnum) {
          oldbtnum = btnum;
          switch (btnum) {
            case 0:  //dimmer
              setqpicc(0, 6);
              setqpicc(1, 4);
              setqpicc(3, 4);
              break;
            case 1:  //mute
              setqpicc(0, 4);
              setqpicc(1, 6);
              setqpicc(3, 4);
              break;
            case 2:  //next page
              setqpicc(0, 4);
              setqpicc(1, 4);
              setqpicc(3, 6);
              break;
          }
        }
        switch (btnum) {
          case 0:  //dimmer
            if (digitalRead(sw) == LOW) {
              delay(20);
              if (digitalRead(sw) == LOW) {
                while (digitalRead(sw) == LOW) { delay(1); }
                setqpicc(0, 4);
                setflag = true;
                slc = btnum;
                oldpos = -1;
                display.print("n0.pco=63845");  //red
                //display.print("n0.pco=65535");//white
                end_screen();
              }
            }
            break;
          case 1:  //mute
            if (digitalRead(sw) == LOW) {
              delay(20);
              if (digitalRead(sw) == LOW) {
                while (digitalRead(sw) == LOW) { delay(1); }
                setqpicc(1, 4);
                setflag = true;
                slc = btnum;
                oldpos = -1;
                if (mute) {
                  pos = 0;
                } else {
                  pos = 1;
                }
              }
            }
            break;
          case 2:  //back
            if (digitalRead(sw) == LOW) {
              delay(20);
              if (digitalRead(sw) == LOW) {
                page = 1;
                while (digitalRead(sw) == LOW) { delay(1); }
                topage(page);
                btnum = 0;
                encoder.setPosition(0);
              }
            }
            break;
        }
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
        Serial.println("btnext");
        Serial.println(btnum);
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
        Serial.println("btback");
        Serial.println(btnum);
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
