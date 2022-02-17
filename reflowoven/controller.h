/*
 *  controller.h
 *  written by jp7dki
 */

#ifndef Controller_h
#define Controller_h

#define SSR_ON HIGH
#define SSR_OFF LOW

typedef enum{
  OUTPUT_ON,
  OUTPUT_OFF
}controller_output;

typedef enum{
  STATE_STOP,
  STATE_START
}controller_state;

// define controller class

class Controller
{
  public:
    Controller(uint8_t pin);                  // Constructor
    void init();
    void start(float target_val);     // start control
    uint8_t update(float temp);                  // controller update
    void stop();

  private:
    void set_output(uint8_t output);
    uint8_t output_pin;   // output pin
    controller_state state;    // Internal State
    float target_temp; // target value
    
};

#endif
