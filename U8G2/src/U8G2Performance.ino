#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

#define FONT u8g2_font_pxplusibmcgathin_8f
//#define FONT u8g2_font_artossans8_8u

#define WIRE_SUPPORTS_GET_CLOCK

//U8G2_SSD1306_128X64_NONAME_F_SW_I2C oled(U8G2_R2, D1, D2);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0);
//U8G2_SSD1306_128X64_1_SW_I2C u8g2(U8G2_R0, 13, 11, 10, 8);

long count = 0;
long last = micros();
long draw = 0;
long display = 0;
long total = 0;
bool whiteMode = false;
bool xMode = false;
uint8_t step = 32;

void (*function)();

uint8_t bitmapWidth = 16;
uint8_t bitmapHeight = 16;
uint8_t bitmap[] = {
  0x00, 0xFE, 0xFE, 0xFE, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0xFE, 0xFE, 0xFE, 0x00,
  0x00, 0x7F, 0x7F, 0x7F, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x7F, 0x7F, 0x7F, 0x00
};

void setup()
{
  setupSerial();
  setupOled();
}

void setupOled()
{
//  oled.setI2CAddress(0x3C * 2);
  oled.begin();
  //oled.flipVertical(true);
  //oled.flipHorizontal(true);
  oled.setFont(FONT);
  oled.clearBuffer();
  oled.sendBuffer();
}

void setupSerial()
{
  Serial.begin(230400);
  Serial.println();
}

void reportOLED(String name, long value, String units, uint8_t row)
{
  oled.setDrawColor(1);
  String v(value);
  v += ' ';
  v += units;
  v += ' ';
  oled.setCursor(0, row * 8 + 8);
  oled.print(v);
  oled.setDrawColor(whiteMode ? 1 : 0);
}

void reportSerial(String name, long value, String units)
{
  Serial.print(name);
  Serial.print(':');
  Serial.print(value);
  Serial.print(units);
  Serial.print(' ');
}

void lines()
{
  uint16_t xD =  xMode ? count % step : step - 1;
  uint16_t yD = !xMode ? count % step : 0;
  xD *= 128; 
  xD /= step;
  yD *= 64;
  yD /= step;
  if (xD > 127) xD = 127;
  if (yD > 63) yD = 63;
  oled.drawLine(xD, yD, 127 - xD, 63 - yD);  
}

void circles()
{
  uint16_t rD = count % step;
  rD *= 32;
  rD /= step;
  oled.drawCircle(63, 31, rD);  
}

void bitmaps()
{
  uint16_t xD =  xMode ? count % step : step - 1;
  uint16_t yD = !xMode ? count % step : 0;
  oled.drawBitmap(32 + xD, yD, bitmapWidth/8, bitmapHeight, bitmap);
}

void loop()
{
  long start = micros();

  if (count % (step * 4) == 0)
  {
    switch((count / (step * 4)) % 2)
    {
      case 0 : function = lines; break;
      case 1 : function = circles; break;
      //case 3 : function = bitmaps; break;
      default : function = lines; break;
    }
  }
  if (count % (step * 2) == 0)
  {
    whiteMode = !whiteMode;
    oled.setDrawColor(whiteMode ? 1 : 0);
  }
  if (count % step == 0)
  {
    xMode = !xMode;
  }

  start = micros();
  
  function();
  #ifdef WIRE_SUPPORTS_GET_CLOCK
    reportOLED("I2C", Wire.getClock() / 1000, "kHz", 3);
  #endif
  reportOLED("CPU", F_CPU / 1000000, "MHz", 4);
  reportOLED("draw", draw, "us", 5);
  reportOLED("display", display, "us", 6);
  reportOLED("total", total, "us", 7);

  draw = micros() - start;

  oled.sendBuffer();

  display = micros() - start;

  reportSerial("draw", draw, "us");
  reportSerial("display", display, "us");
  reportSerial("total", total, "us");
  Serial.println();

  total = micros() - last;
  last = micros();

  ++count;
}
