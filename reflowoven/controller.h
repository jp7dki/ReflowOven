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
    Controller(float k, float k_p, float k_i, float k_d, float delta_t);                  // Constructor
    void init();
    void start();     // start control
    uint8_t update(float feedback_temp, float target_temp);                  // controller update
    void stop();
    int duty;

  private:
    controller_state state;    // Internal State
    float k, k_p, k_i, k_d;    // PID parameter
    float delta_t;          // PID parameter (control period[s])
    float pre_err, err;     // PID error
    float integral;         // 積分値
    
};

#endif
