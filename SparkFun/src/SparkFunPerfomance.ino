#include <Wire.h>  // Include Wire if you're using I2C
#include <SPI.h>  // Include SPI if you're using SPI
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library

//////////////////////////
// MicroOLED Definition //
//////////////////////////
#define PIN_RESET D8  // Connect RST to pin 9 (req. for SPI and I2C)
#define DC_JUMPER 0

#define WIRE_SUPPORTS_GET_CLOCK

MicroOLED oled(PIN_RESET, DC_JUMPER);

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
  Serial.println("Setup");
  setupSerial();
  setupOled();
  Serial.println("Setup done");
}

void setupOled()
{
  oled.begin();
  oled.contrast(255);
  //oled.flipVertical(true);
  //oled.flipHorizontal(true);
  //oled.setFont(0);
  oled.clear(0);
  oled.display();
}

void setupSerial()
{
  Serial.begin(230400);
  Serial.println();
}

void reportOLED(String name, long value, String units, uint8_t row)
{
  oled.setCursor(0, (row-2) * 8);
  oled.print(value);
  oled.print(' ');
  oled.print(units);
  oled.print(' ');
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
  uint16_t xD =  xMode ? count % step : step;
  uint16_t yD = !xMode ? count % step : 0;
  xD *= 128; 
  xD /= step;
  yD *= 64;
  yD /= step;
  if (xD > 127) xD = 127;
  if (yD > 63) yD = 63;
  oled.line(xD, yD, 127 - xD, 63 - yD);  
}

void circles()
{
  uint16_t rD = count % step;
  rD *= 32;
  rD /= step;
  oled.circle(63, 31, rD);  
}

void bitmaps()
{
  uint16_t xD =  xMode ? count % step : step - 1;
  uint16_t yD = !xMode ? count % step : 0;
  //oled.drawBitmap(bitmap, 32 + xD, yD, bitmapWidth, bitmapHeight);
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
      //case 2 : function = bitmaps; break;
      default : function = lines; break;
    }
  }
  if (count % (step * 2) == 0)
  {
    whiteMode = !whiteMode;
    oled.setColor(whiteMode ? 1 : 0);
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

  oled.display();

  display = micros() - start;

  reportSerial("draw", draw, "us");
  reportSerial("display", display, "us");
  reportSerial("total", total, "us");
  Serial.println();

  total = micros() - last;
  last = micros();

  ++count;
}
