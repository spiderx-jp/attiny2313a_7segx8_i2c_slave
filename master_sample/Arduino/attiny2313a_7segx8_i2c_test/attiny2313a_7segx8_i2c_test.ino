#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);
}

uint8_t buff[] = {
  0x3f, // 0
  0x06, // 1
  0x5b, // 2
  0x4f, // 3
  0x66, // 4
  0x6d, // 5
  0x7d, // 6
  0x07, // 7
  0x7f, // 8
  0x6f, // 9
  0x77, // a
  0x7c, // b
  0x39, // c
  0x5e, // d
  0x79, // e
  0x71, // f
  0x80, // dot
};

void loop() {
  int led, buff_p, i;

  // Test01
  Serial.println("============================ TEST01 START");
  for (led = 0; led < 8; led++) {
    for (buff_p = 0; buff_p < sizeof(buff); buff_p++) {
      Serial.println("beginTransmission");
      Wire.beginTransmission(0x09);
      Serial.print("    LED=");
      Serial.print(led, HEX);
      Wire.write(led);
      Serial.print(", DATA=");
      Serial.println(buff[buff_p], HEX);
      Wire.write(buff[buff_p]);
      Serial.println("endTransmission");
      Wire.endTransmission();
      delay(100);
    }
  }
  Serial.println("============================ TEST01 END");
  Serial.println("");
  delay(1000);

  // Test02
  Serial.println("============================ TEST02 START");
  for (led = 0; led < 8; led++) {
    buff_p = 0;
    Serial.println("beginTransmission");
    Wire.beginTransmission(0x09);
    Serial.print("    LED=");
    Serial.println(led, HEX);
    Wire.write(led);
    for (i = led; i < 8; i++) {
      Serial.print("        DATA=");
      Serial.println(buff[buff_p], HEX);
      Wire.write(buff[buff_p]);
      buff_p++;
      if (buff_p >= sizeof(buff)) {
        buff_p = 0;
      }
    }
    Serial.println("endTransmission");
    Wire.endTransmission();
    delay(500);
  }
  Serial.println("============================ TEST02 END");
  Serial.println("");
  delay(1000);

  // Test03
  Serial.println("============================ TEST03 START");
  led = 0;
  buff_p = 0;
  Serial.println("beginTransmission");
  Wire.beginTransmission(0x09);
  Serial.print("    LED=");
  Serial.println(led, HEX);
  Wire.write(led);
  for (i = led; i < 8; i++) {
    Serial.print("        DATA=");
    Serial.println(buff[buff_p], HEX);
    Wire.write(buff[buff_p]);
    buff_p++;
    if (buff_p >= sizeof(buff)) {
      buff_p = 0;
    }
  }
  Serial.println("endTransmission");
  Wire.endTransmission();
  delay(500);

  Serial.println("beginTransmission");
  Wire.beginTransmission(0x09);
  Serial.print("    DIMMER=");
  Serial.println(0x15, HEX);
  Wire.write(0x15);
  Serial.println("endTransmission");
  Wire.endTransmission();
  delay(3000);

  Serial.println("beginTransmission");
  Wire.beginTransmission(0x09);
  Serial.print("    DIMMER=");
  Serial.println(0x14, HEX);
  Wire.write(0x14);
  Serial.println("endTransmission");
  Wire.endTransmission();
  delay(3000);

  Serial.println("beginTransmission");
  Wire.beginTransmission(0x09);
  Serial.print("    DIMMER=");
  Serial.println(0x13, HEX);
  Wire.write(0x13);
  Serial.println("endTransmission");
  Wire.endTransmission();
  delay(3000);

  Serial.println("beginTransmission");
  Wire.beginTransmission(0x09);
  Serial.print("    DIMMER=");
  Serial.println(0x12, HEX);
  Wire.write(0x12);
  Serial.println("endTransmission");
  Wire.endTransmission();
  delay(3000);

  Serial.println("beginTransmission");
  Wire.beginTransmission(0x09);
  Serial.print("    DIMMER=");
  Serial.println(0x11, HEX);
  Wire.write(0x11);
  Serial.println("endTransmission");
  Wire.endTransmission();
  delay(3000);

  Serial.println("beginTransmission");
  Wire.beginTransmission(0x09);
  Serial.print("    DIMMER=");
  Serial.println(0x10, HEX);
  Wire.write(0x10);
  Serial.println("endTransmission");
  Wire.endTransmission();
  delay(3000);

  Serial.println("============================ TEST03 END");
  Serial.println("");
  delay(3000);

  // Test04
  Serial.println("============================ TEST04 START");
  Serial.println("beginTransmission");
  Wire.beginTransmission(0x09);
  Serial.print("    Refresh=");
  Serial.println(0x21, HEX);
  Wire.write(0x21);
  Serial.println("endTransmission");
  Wire.endTransmission();
  delay(3000);

  Serial.println("beginTransmission");
  Wire.beginTransmission(0x09);
  Serial.print("    Refresh=");
  Serial.println(0x20, HEX);
  Wire.write(0x20);
  Serial.println("endTransmission");
  Wire.endTransmission();
  delay(3000);

  Serial.println("============================ TEST04 END");
  Serial.println("");
  delay(3000);

  // Test05
  Serial.println("============================ TEST05 START");

  Serial.println("beginTransmission");
  Wire.beginTransmission(0x09);
  Serial.println("requestFrom");
  Wire.requestFrom(0x09, 5);
  while (Wire.available()) {
    byte val = Wire.read();
    Serial.print("    Read 1byte Data=");
    Serial.println(val, HEX);
  }
  Serial.println("endTransmission");
  Wire.endTransmission();
  Serial.println("============================ TEST05 END");
  Serial.println("");
  delay(3000);


}
