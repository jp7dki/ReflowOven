/*
 * Reflow Oven for esp-wroom-32 Ver.1.0
 * Written by jp7dki
 * 
 */
#include <Arduino.h>
#include "controller.h"
#include "temp_sensor.h"
#include "LCD.h"

// for debug
#define DBG        

#define SSR_OUT 17
#define START_IN  16

#define SCL_PIN 22
#define SDA_PIN 21
#define RST_PIN 4

#define LOOP_DELAY 100      // ループディレイ(おおよその制御周期) [ms]

// state machine
typedef enum{
  REFLOW_STATE_STOP,
  REFLOW_STATE_PREHEATING_RISE,      // プリヒート温度までの加熱
  REFLOW_STATE_PREHEATING,           // プリヒート
  REFLOW_STATE_HEATING_RISE,         // 本加熱までの加熱
  REFLOW_STATE_HEATING,              // 本加熱中
  REFLOW_STATE_COOLING
}Reflow_state;

Controller *controller;
Temp_sensor *temp_sensor;
Char_lcd *char_lcd;

Reflow_state reflow_state;
Reflow_state next_reflow_state;
float temp_preheat = 150.0;         // プリヒート温度
float temp_heat = 220;              // 本加熱温度
uint16_t time_preheat = 60;         // プリヒート時間
uint16_t time_heat = 10;            // 本加熱時間
uint16_t time_cool = 60;            // 冷却時間

uint32_t time_count;

void setup() 
{
  #ifdef DBG
  Serial.begin(115200);     // debug out
  #endif
  temp_sensor = new Temp_sensor();
  temp_sensor->init();
  controller = new Controller(SSR_OUT);
  controller->init();
  
  char_lcd = new Char_lcd(SDA_PIN, SCL_PIN, RST_PIN);
  char_lcd->init();
  char_lcd->disp_on();

  controller->start(13.5);

  pinMode(START_IN, INPUT_PULLUP);     // start button input

  reflow_state = REFLOW_STATE_STOP;        // initial state
}

void loop() 
{
  // put your main code here, to run repeatedly:
  float temp;
  uint8_t cont;

  temp = temp_sensor->read_temp();

  // state machine control
  switch(reflow_state){
    case STATE_STOP:
      // 待機
      char_lcd->write_str(0,"STOP");
      if(digitalRead(START_IN)== 0){
        next_reflow_state = REFLOW_STATE_PREHEATING_RISE;
        controller->start(temp_preheat);
      }
      break;

    case REFLOW_STATE_PREHEATING_RISE:
      // プリヒート温度まで加熱
      char_lcd->write_str(0,"PRE RISE");
      if(temp >= temp_preheat){
        next_reflow_state = REFLOW_STATE_PREHEATING;
        time_count = 0;
      }
      
      break;
      
    case REFLOW_STATE_PREHEATING:
      // プリヒート動作
      char_lcd->write_str(0,"PREHEAT");
      if(time_count >= (time_preheat*1000)/(float)LOOP_DELAY){
        next_reflow_state = REFLOW_STATE_HEATING_RISE;
        controller->start(temp_heat);
      }else{
        time_count += 1;
      }
      break;

    case REFLOW_STATE_HEATING_RISE:
      // 本加熱温度まで上昇
      char_lcd->write_str(0,"MAINRISE");
      if(temp >= temp_heat){
        next_reflow_state = REFLOW_STATE_HEATING;
        time_count = 0;
      }

      break;
      
    case REFLOW_STATE_HEATING:
      // 本加熱
      char_lcd->write_str(0,"HEAT");
      if(time_count >= (time_heat*1000)/(float)LOOP_DELAY){
        next_reflow_state = REFLOW_STATE_COOLING;
        controller->stop();
        time_count = 0;
      }else{
        time_count += 1;
      }
      
      break;

    case REFLOW_STATE_COOLING:
      // 冷却
      char_lcd->write_str(0,"COOLING");
      if(time_count >= (time_cool*1000)/(float)LOOP_DELAY){
        next_reflow_state = REFLOW_STATE_STOP;
      }else{
        time_count += 1;
      }
      
      break;
  }

  reflow_state = next_reflow_state;
  cont = controller->update(temp);

  disp_temp(temp);

  #ifdef DBG
  printf("thermocouple:%.3f, cont:%d, state:%d\n",temp,cont,reflow_state);
  #endif
  
  delay(LOOP_DELAY);
  
}

void disp_temp(float temp){
  char buf[9];
  dtostrf((double)temp,8,2,buf);
  //buf[0] = 'T';
  char_lcd->write_char(1,buf,8);
}
