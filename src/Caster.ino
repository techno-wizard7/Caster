#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#include <Adafruit_ST7789.h>

// Include Scheduler since we want to manage multiple tasks.
#include "mbed.h"
#include "Kernel.h"

#include <SPI.h>
#include <Scheduler.h>
#include "WiFiNINA.h"
#include "controller.h"
// #include <Arduino_GFX_Library.h>

#define TFT_CS 9 // GFX_NOT_DEFINED for display without CS pin
#define TFT_DC 8
#define TFT_RST 7
#define GFX_BL 6
// On Nano RP2040 Connect, RGB leds are connected to the wifi module
// The user APIs are the same, but we can't convert to int, so use defines

#define led1  LEDR
#define led2  LEDG
#define led3  LEDB

// #define LCDTFT
#ifdef LCDTFT
// // 1.3"/1.5" square IPS LCD 240x240
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);//, 18 /* SCK */, 23 /* MOSI */, spi0 /* MISO */);
Arduino_GFX *output_display = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col offset 2 */, 80 /* row offset 2 */);
Arduino_GFX *canvas = new Arduino_Canvas(240 /* width */, 240 /* height */, output_display);
#else
Adafruit_ST7789 *output_display = new  Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 *canvas(240, 240); // 16-bit, 120x30 pixels
#endif

Controller stick;


//#define IRQ_PIN   5
void setup() {
  // boot priorities
  Serial.begin(115200);
  Wire.begin();	  // I2C
  output_display->init(240, 240);
  canvas.fillScreen(0x00);
  // display.drawBitmap(0, 0, canvas.getBuffer(),canvas.width(), canvas.height(), 0xFFFF, 0x0000);     
  // if (!gfx->begin())
  // // if (!gfx->begin(80000000)) /* specify data bus speed */
  // {
  //   Serial.println("gfx->begin() failed!");
  // }

  //SD
  //periphrials
  stick.begin();
  //start sensors
  //connect wireless nets
  Scheduler.startLoop(controllerLoop);



}


void loop() {
  // put your main code here, to run repeatedly:
  switch(1){
    case 0: break;
    case 1: break;
    default:
      watchFace();
      break;
  }

}

void controllerLoop() {
  //check controller and save state in global for other functions
  stick.update();
 
}

void wirelessListener() {

}

/*core screens
-Watch 1 (minimal)
-watch 2 (hud)
-Quick select(two levels or more (to implement later))
-"load" program
-settings menu
-quick select assigners
-comms menu
*/
