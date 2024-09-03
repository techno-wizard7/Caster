#define SerialPort Serial
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
// #include "lib/QuickContextMenu/src/Menu.h"
#include "Menu.h"
#include "display.h"
#include <Adafruit_ST7789.h>
#include <SPI.h>

#ifdef TARGET_RP2040
#include "mbed.h"
#include "Kernel.h"
#include <Scheduler.h> // Include Scheduler since we want to manage multiple tasks.
#include "WiFiNINA.h"
#include <LSM6DSOSensor.h>
// On Nano RP2040 Connect, RGB leds are connected to the wifi module
// The user APIs are the same, but we can't convert to int, so use defines

#define led1  LEDR
#define led2  LEDG
#define led3  LEDB
// #define DEV_I2C Wire
#endif
#include "controller.h"
// #include <Arduino_GFX_Library.h> //Faster than Adafruit and better primatives, but uncooperative with Nano Connect

#define TFT_CS 9 // GFX_NOT_DEFINED for display without CS pin
#define TFT_DC 8
#define TFT_RST 7
#define TFT_BACKLIGHT  6 // Display backlight pin

// #define LCDTFT
#ifdef LCDTFT// For when/if Arduino_GFX_Library adds support to ArduinoCore-Mbed (Depending on time/resources I may just port a custom fork that can handle the RP2040. Or adapt code to function on multiple target hardware configurations)
  // // 1.5" square IPS LCD 240x240
  Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);//, 18 /* SCK */, 23 /* MOSI */, spi0 /* MISO */);
  Arduino_GFX *output_display = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col offset 2 */, 80 /* row offset 2 */);
  Arduino_GFX *canvas = new Arduino_Canvas(240 /* width */, 240 /* height */, output_display);
#else
  Adafruit_ST7789* output_display = new  Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
  GFXcanvas16 *canvas = new GFXcanvas16(240, 240); // 16-bit, 120x30 pixels
#endif
  bool isDisplayVisible = false;

// #define GAMEPAD
#ifdef GAMEPAD
#define BUTTON_X         6
#define BUTTON_Y         2
#define BUTTON_A         5
#define BUTTON_B         1
#define BUTTON_SELECT    0
#define BUTTON_START    16
#define JOY_X    14
#define JOY_Y    15
GamepadController pad;
#else

SerialController pad;
#endif
LSM6DSOSensor accGyr(&Wire);
volatile int mems_event = 0;
void INT1Event_cb();
unsigned long previousMillis;
long idleTime = 10000;

//clock functions (need to be stored better, but the code is borrowed to test if it works)
#define SIXTIETH 0.016666667
#define TWELFTH 0.08333333
#define SIXTIETH_RADIAN 0.10471976
#define TWELFTH_RADIAN 0.52359878
#define RIGHT_ANGLE_RADIAN 1.5707963

static uint8_t conv2d(const char *p)
{
  uint8_t v = 0;
  return (10 * (*p - '0')) + (*++p - '0');
}
static int16_t hh, mm, ss;
static unsigned long targetTime; // next action time



Menu* rootMenu(new Menu("Root Menu"));
//#define IRQ_PIN   5
void setup() {
  // boot priorities
  Serial.begin(115200);
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, LOW);
  Wire.begin();	  // I2C
  output_display->init(240, 240);
  output_display->enableDisplay(isDisplayVisible);
  canvas->fillScreen(0x00);
  // display.drawBitmap(0, 0, canvas.getBuffer(),canvas.width(), canvas.height(), 0xFFFF, 0x0000);     
  // if (!gfx->begin())
  // // if (!gfx->begin(80000000)) /* specify data bus speed */
  // {
  //   Serial.println("gfx->begin() failed!");
  // }

  //SD
  //ReadSettings
  toggleDisplay();
  //periphrials

  pad.begin();

  hh = conv2d(__TIME__);
  mm = conv2d(__TIME__ + 3);
  ss = conv2d(__TIME__ + 6);

  targetTime = ((millis() / 1000) + 1) * 1000;

  //Start Sensors

  //LSM6DSOX IMU
  attachInterrupt(A3, INT1Event_cb, RISING);
  // Initlialize components.
  accGyr.begin();
  accGyr.Enable_X();
  // Enable Wake Up Detection.
  accGyr.Enable_Wake_Up_Detection(LSM6DSO_INT1_PIN);

  //connect wireless nets
    Scheduler.startLoop(controllerLoop);

}



void loop() {
  //OG loop will handle base functions, data accumulation, task manager, and managing the root menu. There's also the base watch face.
  unsigned long cur_millis = millis();
  if (cur_millis >= targetTime)
  {
    targetTime += 1000;
    ss++; // Advance second
    if (ss == 60)
    {
      ss = 0;
      mm++; // Advance minute
      if (mm > 59)
      {
        mm = 0;
        hh++; // Advance hour
        if (hh > 23)
        {
          hh = 0;
        }
      }
    }
  }
  
  if(isDisplayVisible)
    checkIdle();
  else
    checkWakeEvent();
  char ** buffer;
  size_t bufferSize = snprintf(NULL, 0, "%d:%d:%d", hh, mm, ss) + 1;
  *buffer = (char*)malloc(bufferSize);
  snprintf(*buffer, bufferSize, "%d:%d:%d", hh, mm, ss);
  defaultWatchFace(canvas, 1, buffer);

}

void controllerLoop() {
  //check controller and save state in global for other functions
  //will be seperate thread to reduce lag on
  pad.update();
 
}

void wirelessListener() {

}

void displayWrite(){
  if(isDisplayVisible)
    output_display->drawRGBBitmap(0, 0, canvas->getBuffer(),canvas->width(), canvas->height());
  // display.drawRGBBitmap(0, 0, canvas.getBuffer(),canvas.width(), canvas.height());
}

void toggleDisplay(){
  isDisplayVisible = !isDisplayVisible;
  output_display->enableDisplay(isDisplayVisible);
  digitalWrite(TFT_BACKLIGHT, HIGH & isDisplayVisible); // Backlight on
  #if (SerialDebugging)
  Serial.print("button pressed @ ");
  Serial.print(millis());
  Serial.print(", display is now ");
  Serial.println((isDisplayVisible ? "ON" : "OFF"));
  #endif
}

void checkIdle(){
  if(isDisplayVisible && ((millis() - previousMillis) > idleTime))
    toggleDisplay();
}

void checkWakeEvent(){
  if (mems_event && !isDisplayVisible) {
    mems_event = 0;
    LSM6DSO_Event_Status_t status;
    accGyr.Get_X_Event_Status(&status);
    if (status.WakeUpStatus)
    {
      toggleDisplay();
      previousMillis = millis();
      SerialPort.println("Wake up Detected!");
      return;
    }
  }
  if(pad.change() && !isDisplayVisible)
    toggleDisplay();
}

void INT1Event_cb()
{
  mems_event = 1;
}
/*core screens
-Watch 1 (casual)
-watch 2 (hud)
-Quick select(two levels or more (to implement later))
-"load" program
-settings menu
-quick select assigners
-comms menu
*/
