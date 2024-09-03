#ifndef _controllers_
#define _controllers_

#include <Adafruit_seesaw.h>
#include "math.h"

class GamepadController
{
  Adafruit_seesaw ss;
  #define BUTTON_X         6
  #define BUTTON_Y         2
  #define BUTTON_A         5
  #define BUTTON_B         1
  #define BUTTON_SELECT    0
  #define BUTTON_START    16
  #define JOY_X    14
  #define JOY_Y    15
  uint32_t button_mask = (1UL << BUTTON_X) | (1UL << BUTTON_Y) | (1UL << BUTTON_START) |
                       (1UL << BUTTON_A) | (1UL << BUTTON_B) | (1UL << BUTTON_SELECT);
  //#define IRQ_PIN   5
  uint32_t buttons, last_buttons = button_mask;
  int x = 0, y = 0, last_x = 0, last_y = 0;

  public:
    GamepadController() {};

    void begin(){
      if(!ss.begin(0x50)){
        Serial.println("ERROR! seesaw not found");
        // while(1) delay(1);
      }
      // Serial.println("pad connected");
      uint32_t version = ((ss.getVersion() >> 16) & 0xFFFF);
      if (version != 5743) {
        Serial.print("Wrong firmware loaded? ");
        Serial.println(version);
        while(1) delay(10);
      }
      // Serial.println("Found Adafruit Product 5743");
      ss.pinModeBulk(button_mask, INPUT_PULLUP);
      ss.setGPIOInterrupts(button_mask, 1);

      #if defined(IRQ_PIN)
        pinMode(IRQ_PIN, INPUT);
      #endif
    }

    int getDegree(){
      int degree = atan(y/x);
      if(degree < 0) degree = 180+ abs(degree);
      return degree;
    }

    int getDial(int div) {
      if (div > 0 && div <=360)
        return floor(div * getDegree() / 360);
      return -1;
    }

    bool isAPressed(){
      if (! (buttons & (1UL << BUTTON_A))) return true;
      return false;
    }

    bool isBPressed(){
      if (! (buttons & (1UL << BUTTON_B))) return true;
      return false;
    }
    bool isXPressed(){
      if (! (buttons & (1UL << BUTTON_X))) return true;
      return false;
    }
    bool isYPressed(){
      if (! (buttons & (1UL << BUTTON_Y))) return true;
      return false;
    }

    bool isSelectPressed(){
      if (! (buttons & (1UL << BUTTON_SELECT))) return true;
      return false;
    }

    bool isStartPressed(){
      if (! (buttons & (1UL << BUTTON_START))) return true;
      return false;
    }

    void update(){
      
    // Reverse x/y values to match joystick orientation
    x = 1023 - ss.analogRead(JOY_X);
    y = 1023 - ss.analogRead(JOY_Y);
    
    if ( (abs(x - last_x) > 3)  ||  (abs(y - last_y) > 3)) {
      // Serial.print("x: "); Serial.print(x); Serial.print(", "); Serial.print("y: "); Serial.println(y);
      last_x = x;
      last_y = y;
    }

  #if defined(IRQ_PIN)
    if(!digitalRead(IRQ_PIN)) {
      return;
    }
  #endif

      last_buttons = buttons;
      buttons = ss.digitalReadBulk(button_mask);

      //Serial.println(buttons, BIN);

    }
};

class SerialController
{
  // uint32_t button_mask = (1UL << BUTTON_X) | (1UL << BUTTON_Y) | (1UL << BUTTON_START) |
  //                      (1UL << BUTTON_A) | (1UL << BUTTON_B) | (1UL << BUTTON_SELECT);
  //#define IRQ_PIN   5
  uint8_t buttons, last_buttons;  // = button_mask;
  int x = 0, y = 0, last_x = 0, last_y = 0;
  bool fresh = true;

  public:
    SerialController() {};

    void begin(){
    }

    int getDegree(){
      int degree = atan(y/x);
      if(degree < 0) degree = 180+ abs(degree);
      return degree;
    }

    int getDial(int div) {
      if (div > 0 && div <=360)
        return floor(div * getDegree() / 360);
      return -1;
    }

    bool isAPressed(){
      if (! (buttons & 0x1)) return true;
      return false;
    }

    bool isBPressed(){
      if (! (buttons & 0x2)) return true;
      return false;
    }
    bool isXPressed(){
      if (! (buttons & 0x4)) return true;
      return false;
    }
    bool isYPressed(){
      if (! (buttons & 0x8)) return true;
      return false;
    }

    bool isSelectPressed(){
      if (! (buttons & 0x10)) return true;
      return false;
    }

    bool isStartPressed(){
      if (! (buttons & 0x20)) return true;
      return false;
    }

    bool change(){
      return fresh;
    }

    void update(){
      fresh = false;
      uint8_t mask = 0x00;
      char* buffer;
      int i = 0;
      if (Serial.available() > 0) {
        i = Serial.readBytes(buffer, 3);
      }
      if(i == 3){
        fresh = !fresh;
        mask =(uint8_t)buffer[0];
        x = (int) buffer[1];
        y = (int) buffer[2];
      }
      free(buffer);
      if ( (abs(x - last_x) > 3)  ||  (abs(y - last_y) > 3)) {
        last_x = x;
        last_y = y;
      }
        last_buttons = buttons;
        buttons = mask;
    }
};
#endif;