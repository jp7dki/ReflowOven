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

#define K 500.0
#define KP 0.006294076
#define KI 0.0000572189
#define KD 0.100075815
#define DT 0.5

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
float temp_preheat = 120.0;         // プリヒート温度
float temp_heat = 150.0;              // 本加熱温度
uint16_t time_preheat = 60;         // プリヒート時間
uint16_t time_heat = 10;            // 本加熱時間
uint16_t time_cool = 60;            // 冷却時間
uint16_t pwm_period = 500;          // PWM周期(ms)
uint16_t pwm_duty = 0;              // PWMのデューティ

uint32_t time_count;

hw_timer_t * timer0 = NULL;         // hardware timer
volatile boolean flg = false;
volatile int counter = 0;
volatile int output = 0;

volatile boolean temp_flg = false;
volatile float temp_ave = 0.0;

volatile SemaphoreHandle_t timerSemaphore;    // 割り込み発生状態を表すセマフォ
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;   //排他制御を行うための変数


void IRAM_ATTR interrupt0(){

  portENTER_CRITICAL(&timerMux);    // 排他制御開始
  
  counter++;
  if(counter > controller->duty){
    // pin set low
    output = 0;
  }
  if(counter > pwm_period){
    // pin set high
    output = 1;
    
    counter = 0;
    flg=true;
  }
  if(output == 1){
    digitalWrite(SSR_OUT, HIGH);
  }else{
    digitalWrite(SSR_OUT, LOW);
  }
  temp_flg = true;
  portEXIT_CRITICAL(&timerMux);     // 排他制御終了
  xSemaphoreGiveFromISR(timerSemaphore, NULL);    // セマフォを開放
  
}

void setup() 
{
  
  #ifdef DBG
  Serial.begin(115200);     // debug out
  #endif
  temp_sensor = new Temp_sensor();
  temp_sensor->init();
  controller = new Controller(K,KP,KI,KD,DT);
  controller->init();
  
  char_lcd = new Char_lcd(SDA_PIN, SCL_PIN, RST_PIN);
  char_lcd->init();
  char_lcd->disp_on();

  controller->stop();

  pinMode(START_IN, INPUT_PULLUP);     // start button input
  pinMode(SSR_OUT, OUTPUT);
  digitalWrite(SSR_OUT, LOW);

  reflow_state = REFLOW_STATE_STOP;        // initial state

  timerSemaphore = xSemaphoreCreateBinary();    //バイナリセマフォを作成
  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &interrupt0, true);
  timerAlarmWrite(timer0, 1000, true);
  timerAlarmEnable(timer0);
}

void loop() 
{
  float temp;
  uint8_t cont;
  float target_temp=0.0;

  if(xSemaphoreTake(timerSemaphore,0) == pdTRUE){
    if(temp_flg==true){
      temp_ave += temp_sensor->read_temp();
      temp_flg = false;
    }
    
    if(flg==true){

      //temp = temp_sensor->read_temp();
      temp = temp_ave / pwm_period;
      temp_ave = 0;

      // state machine control
      switch(reflow_state){
        case REFLOW_STATE_STOP:
          // 待機
          char_lcd->write_str(0,"STOP");
          if(digitalRead(START_IN)== 0){
            target_temp = temp_preheat;
            next_reflow_state = REFLOW_STATE_PREHEATING_RISE;
            controller->start();
          }else{
            target_temp = 0;
          }
          break;
    
        case REFLOW_STATE_PREHEATING_RISE:
          // プリヒート温度まで加熱
          char_lcd->write_str(0,"PRE RISE");
          if(temp >= temp_preheat-5.0){
            next_reflow_state = REFLOW_STATE_PREHEATING;
            time_count = 0;
          }
          target_temp = temp_preheat;
          
          break;
          
        case REFLOW_STATE_PREHEATING:
          // プリヒート動作
          char_lcd->write_str(0,"PREHEAT");
          if(time_count >= time_preheat*2){
            next_reflow_state = REFLOW_STATE_HEATING_RISE;
            target_temp = temp_heat;
          }else{
            time_count += 1;
            target_temp = temp_preheat;
          }
          break;
    
        case REFLOW_STATE_HEATING_RISE:
          // 本加熱温度まで上昇
          char_lcd->write_str(0,"MAINRISE");
          if(temp >= temp_heat-5.0){
            next_reflow_state = REFLOW_STATE_HEATING;
            time_count = 0;
          }
          target_temp = temp_heat;
    
          break;
          
        case REFLOW_STATE_HEATING:
          // 本加熱
          char_lcd->write_str(0,"HEAT");
          if(time_count >= time_heat*2){
            next_reflow_state = REFLOW_STATE_COOLING;
            target_temp = 0;
            controller->stop();
            time_count = 0;
          }else{
            time_count += 1;
            target_temp = temp_heat;
          }
          
          break;
    
        case REFLOW_STATE_COOLING:
          // 冷却
          char_lcd->write_str(0,"COOLING");
          if(time_count >= time_cool){
            next_reflow_state = REFLOW_STATE_STOP;
          }else{
            time_count += 1;
          }
          target_temp = 0;
          
          break;
      }
    
      controller->update(temp, target_temp);
      reflow_state = next_reflow_state;
    
      disp_temp(temp);
    
      #ifdef DBG
      printf("thermocouple:%.3f, target_temp: %.3f, duty:%d, state:%d\n",temp,target_temp, controller->duty,reflow_state);
      #endif
      
      //  delay(LOOP_DELAY);
      flg=false;

      
    }
    

  }
  

  
}

void disp_temp(float temp){
  char buf[9];
  dtostrf((double)temp,8,2,buf);
  //buf[0] = 'T';
  char_lcd->write_char(1,buf,8);
}
