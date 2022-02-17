/*
 * controller.cpp
 * written by jp7dki
 */

#include <Arduino.h>
#include "controller.h"

// コンストラクタ
Controller::Controller(uint8_t pin)
{
  this->output_pin = pin;                 // 制御出力(出力ピン)を設定
  this->state = STATE_STOP;         // 初期状態は停止
}

void Controller::init(void)
{
  pinMode(this->output_pin, OUTPUT);      // ピンモードを設定
  set_output(OUTPUT_OFF);
}

void Controller::start(float target_temp)
{
  // controller start
  this->target_temp = target_temp;
  this->state = STATE_START;
  
}

uint8_t Controller::update(float temp)
{
  // controller update
  if(this->state != STATE_START){
    set_output(OUTPUT_OFF);
    return -1;
  }
  
  // 単純なONOFF制御する
  if(temp > this->target_temp){
    set_output(OUTPUT_OFF);
    return 0;
  }else{
    set_output(OUTPUT_ON);
    return 1;
  }

  return 0;
}

void Controller::stop(void)
{
  // controller stop
  this->state = STATE_STOP;
  
}

void Controller::set_output(uint8_t output)
{
  if(output == OUTPUT_ON){
    // Output on
    digitalWrite(this->output_pin, SSR_ON);
  }else{
    // Output off
    digitalWrite(this->output_pin, SSR_OFF);
  }
}
