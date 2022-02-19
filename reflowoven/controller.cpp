/*
 * controller.cpp
 * written by jp7dki
 */

#include <Arduino.h>
#include "controller.h"

// コンストラクタ
Controller::Controller(float k, float k_p, float k_i, float k_d, float delta_t)
{
  this->state = STATE_STOP;         // 初期状態は停止
  this->k = k;                      // 制御パラメータを設定
  this->k_p = k_p;
  this->k_i = k_i;
  this->k_d = k_d;
  this->delta_t = delta_t;
}

void Controller::init(void)
{
  
}

void Controller::start()
{
  // controller start
  this->state = STATE_START;
  this->pre_err = 0.0;
  this->err = 0.0;
  this->integral = 0.0;  
}

uint8_t Controller::update(float feedback_temp, float target_temp)
{
  // controller update
  float pid_val;  
  
  if(this->state != STATE_START){
    this->duty = 0;
    return -1;
  }else{
    // PID制御を実行
    this->pre_err = this->err;
    this->err = target_temp - feedback_temp;
    this->integral += (this->err + this->pre_err) / 2.0 * this->delta_t;
  
    pid_val = this->k_p * this->err + this->k_i * integral + this->k_d * (this->err - this->pre_err)/this->delta_t;
  
    this->duty = int(k*pid_val);
    
    return 0;
  }
}

void Controller::stop(void)
{
  // controller stop
  this->state = STATE_STOP;
  this->duty = 0;
  
}
