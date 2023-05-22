#include <RotaryEncoder.h>
#include <SoftwareSerial.h>    // 引用程式庫
SoftwareSerial display(6, 7);  // 接收腳, 傳送腳
RotaryEncoder encoder(12, 13, RotaryEncoder::LatchMode::TWO03);
#define sw 11
#define btnext 5
#define btback 4
#define btstart 3
#define btstop 2
#define ROTARYMIN 0
bool setflag = false;
int ROTARYMAX = 100;
int oldpos = -1, pos = 0;
bool mute = false;
void setup() {
  display.begin(9600);
  //setup pinmode

  pinMode(sw, INPUT_PULLUP);
  pinMode(btnext, INPUT_PULLUP);
  pinMode(btback, INPUT_PULLUP);
  pinMode(btstart, INPUT_PULLUP);
  pinMode(btstop, INPUT_PULLUP);
  topage(1);
}
int page = 1;
int slc = 0;
int pos2 = 0;
int oldpos2 = -1;
int btmax = 0;
bool btnextflag = false;
bool btbackflag = false;
void loop() {
  encoder.tick();
  pos = encoder_loop();
  bt_loop();
  switch (page) {
    case 1:  //clock
      if (digitalRead(sw) == LOW) {
        delay(20);
        if (digitalRead(sw) == LOW) {
          while (digitalRead(sw) == LOW) { delay(1); }
          page = 2;
          topage(page);
          slc = 0;
          encoder.setPosition(0);
          pos2 = 0;
        }
      }
      break;
    case 2:  //timer

      btmax = 6;
      if (oldpos2 != pos2) {
        oldpos2 = pos2;
        setqpicc(pos2, 3);
        for (int i = 0; i < 7; i++) {
          if (i != pos2) {
            setqpicc(i, 2);
          }
        }
      }
      slc = pos2;
      switch (slc) {//
        case 0://第一位數值
          break;
        case 1://第二位數值
          break;
        case 2://第三位數值
          break;
        case 3://第四位數值
          break;
        case 4://start
          break;
        case 5://stop
          break;
        case 6://next menu
          if (digitalRead(sw) == LOW) {
            delay(20);
            if (digitalRead(sw) == LOW) {
              while (digitalRead(sw) == LOW) { delay(1); }
              page = 3;
              slc = 0;
              topage(page);
              pos2 = 0;
              encoder.setPosition(0);
            }
          }
          break;
      }
      break;
    case 3:  //set
      if (setflag) {
        switch (pos2) {
          case 0:

            ROTARYMAX = 255;
            if (oldpos != pos) {
              oldpos = pos;
              setnum(0, pos);
            }
            break;
          case 1:
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

            if (pos2 == 0) {
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
            pos2 = slc;
          }
        }
      } else {
        btmax = 3;
        if (oldpos2 != pos2) {
          oldpos2 = pos2;
          switch (pos2) {
            case 0:
              setqpicc(0, 6);
              setqpicc(1, 4);
              setqpicc(3, 4);
              break;
            case 1:
              setqpicc(0, 4);
              setqpicc(1, 6);
              setqpicc(3, 4);
              break;
            case 2:
              setqpicc(0, 4);
              setqpicc(1, 4);
              setqpicc(3, 6);
              break;
          }
        }
        switch (pos2) {
          case 0:
            if (digitalRead(sw) == LOW) {
              delay(20);
              if (digitalRead(sw) == LOW) {
                while (digitalRead(sw) == LOW) { delay(1); }
                setqpicc(0, 4);
                setflag = true;
                slc = pos2;
                oldpos = -1;
                display.print("n0.pco=63845");  //red
                //display.print("n0.pco=65535");//white
                end_screen();
              }
            }
            break;
          case 1: 
            if (digitalRead(sw) == LOW) {
              delay(20);
              if (digitalRead(sw) == LOW) {
                while (digitalRead(sw) == LOW) { delay(1); }
                setqpicc(1, 4);
                setflag = true;
                slc = pos2;
                oldpos = -1;
                if (mute) {
                  pos = 0;
                } else {
                  pos = 1;
                }
              }
            }
            break;
          case 2:
            if (digitalRead(sw) == LOW) {
              delay(20);
              if (digitalRead(sw) == LOW) {
                page = 1;
                while (digitalRead(sw) == LOW) { delay(1); }
                topage(page);
                pos2 = 0;
                encoder.setPosition(0);
              }
            }
            break;
        }
      }
      break;
  }
}

void end_screen() {
  display.print("\xff\xff\xff");
}
void topage(int p) {
  display.print("page " + String(p));
  end_screen();
}
void setqpicc(int q, int pic) {
  display.print("q" + String(q) + ".picc=" + String(pic));
  end_screen();
}
void setnum(int n, int val) {
  display.print("n" + String(n) + ".val=" + String(val));
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
        btnextflag = true;
        pos2++;
        if (pos2 > btmax) {
          pos2 = btmax;
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
        btbackflag = true;
        pos2--;
        if (pos2 == -1) {
          pos2 = 0;
        }
      }
    }
  } else {
    btbackflag = false;
  }
}
