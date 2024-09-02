#ifndef _display_
#define _display_
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include "dither.h"
#include "sienar4pt7b.h"
#include "nasa.h"

void timeWindow(Adafruit_GFX *out, char* time; int16_t x, int16_t y, int16_t size, bool window=true){
  out->setFont(&SienarAFChariot10pt7b); // Use custom font
  out->fillRect(8, 6, 204, 58, 0xB5B6);
  out->fillRect(10, 11, 200, 50, 0x5AEC);
  out->drawRect(10, 11, 200, 50, 0x9CD2);
  out->setTextSize(2);
  out->setCursor(20, 50);
  out->setTextColor(0xF81F);
  out->println("00:00:00");
}


void defaultWatchFace(Adafruit_GFX *out, int argc, char *argv[]){
  drawDitherGradient(out, 0, 0, 240, 240, 6, 0x5BBE, 0xC05C, 80, 40);
  testfastlines(out, 0x62a0ea, 0x1c71d8);
  
  out->fillRoundRect(-4, 205-6, 29+4+4, 60, 6, 0xBA71);
  out->fillRect(-4, 205, 29+4+4, 60, 0xD434);
  out->fillRect(-4, 205, 29+4, 60, 0x5AEC);
  out->fillCircle(14, 224, 12, ST77XX_BLACK);
  out->fillCircle(13, 223, 12, ST77XX_RED);

  out->setFont(&Nasa21_l23X12pt7b);
  out->setTextSize(1);
  // out->setCursor(9, 230);
  out->setCursor(5, 230);
  // out->drawPixel(5, 230, ST77XX_GREEN);
  // out->drawPixel(13, 230, ST77XX_GREEN);
  // out->drawPixel(13, 223, ST77XX_GREEN);
  delay(1000);
  out->setTextColor(ST77XX_WHITE);
  // out->print("8");
  out->print("88+");
}

void fastGrid(Adafruit_GFX *out, uint16_t color1, uint16_t color2, int spacing = 30;) {
  for (int16_t y=0; y < out->height(); y+=spacing) {
    out->drawFastHLine(0, y, out->width(), color1);
  }
  for (int16_t x=0; x < out->width(); x+=spacing) {
    out->drawFastVLine(x, 0, out->height(), color2);
  }
}

//Select screen (best draft. Writes to display or buffer passed)
void printSelectScreen(Adafruit_GFX *out, int x){
  out->fillScreen(ST77XX_BLACK);
  int unitX = out->width()/7;
  int unitY = out->height()/7;
  for(int i = 1; i <= 3; i++){
    for(int j = 1; j <= 3; j++){
      if( i == j && i == 2)
        drawCrosshair1(out, unitX);
      else{
        out->drawRect(unitX*(2*i-1), unitY*(2*j-1), unitX, unitY, ST77XX_GREEN);
        if(i+j*3-3 == x)
          out->fillRect(unitX*(2*i-1), unitY*(2*j-1), unitX, unitY, ST77XX_GREEN);
      }
    }
  }
}

void drawCrosshair(Adafruit_GFX *out, int x){
  out->drawCircle(out->width()/2-1, out->height()/2-1, x/2, ST77XX_GREEN);
  out->drawCircle(out->width()/2-1, out->height()/2-1, x/4, ST77XX_GREEN);
  out->drawFastHLine((out->width()/2-1-x/2), out->height()/2-1, x, ST77XX_RED);
  out->drawFastVLine((out->width()/2-1), out->height()/2-1-x/2, x, ST77XX_RED);
}
#endif