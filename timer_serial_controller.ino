#include <ArduinoJson.h>
#include <DMXSerial.h>  //dmx通訊
void setup() {
  Serial.begin(9600);
  DMXSerial.init(DMXController);
  DMXSerial.write(1, 0);
  DMXSerial.write(2, 0);
  DMXSerial.write(3, 255);
  DMXSerial.write(4, 255);
  DMXSerial.write(5, 0);
}
int sec;
void loop() {
  if (Serial.available()) {
    String jsonStr = Serial.readStringUntil('\n');  // 讀取串口接收到的 JSON 字串
    jsonStr.trim();                                 // 移除字串前後的空白字符

    // 解析 JSON
    StaticJsonDocument<50> doc;
    DeserializationError error = deserializeJson(doc, jsonStr);

    if (error) {
      Serial.println("Failed to parse JSON");
      return;
    }

    // 確認 get 是否為 "updatetime"
    int get = doc["get"];
    if (get==0) {
      //  Serial.println("Invalid 'get' value");
      return;
    }

    // 將 data 存入 int sec
     sec = doc["data"];
   
  }
 // Serial.print(sec / 60);
  //  Serial.print(":");
   // Serial.println(sec % 60);
    DMXSerial.write(1, (sec / 60));
    DMXSerial.write(2, (sec % 60));
}
