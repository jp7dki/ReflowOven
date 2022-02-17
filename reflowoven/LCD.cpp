/*
 * LCD.cpp
 * written by jp7dki
 */

#include <Arduino.h>
#include "LCD.h"
#include <Wire.h>


// コンストラクタ
Char_lcd::Char_lcd(uint8_t sda_pin, uint8_t scl_pin, uint8_t rst_n_pin){
  this->sda_pin = sda_pin;
  this->scl_pin = scl_pin;
  this->rst_n_pin = rst_n_pin;
}

// 初期化
void Char_lcd::init(void){
  Wire.begin(this->sda_pin, this->scl_pin);
  pinMode(this->rst_n_pin, OUTPUT);
  digitalWrite(this->rst_n_pin, LOW);
  delay(100);
  digitalWrite(this->rst_n_pin, HIGH);
  delay(200);

  // Lcd Initialization
  this->writecmd(0x38); delay(1);
  this->writecmd(0x39); delay(1);
  this->writecmd(0x14); delay(1);
  this->writecmd(0x7F); delay(1);
  this->writecmd(0x55); delay(2);
  this->writecmd(0x6C); delay(300);
  this->writecmd(0x38); delay(1);
  this->writecmd(0x0C); delay(2);
  this->writecmd(0x01); delay(2);
  
}

// LCDに表示
void Char_lcd::write_str(uint8_t row, String str){
  uint8_t i,n;
  n = str.length();

  if(row==0){
    this->writecmd(0x80);
  }else{
    this->writecmd(0xC0);
  }

  for(i=0; i < n; i++){
    this->writedata(str.charAt(i));
    delay(1);
  }
}

// LCDに表示
void Char_lcd::write_char(uint8_t row, char *str, uint8_t num){
  uint8_t i;
  if(row==0){
    this->writecmd(0x80);
  }else{
    this->writecmd(0xC0);
  }

  for(i=0; i < num; i++){
    this->writedata(str[i]);
    delay(1);
  }
}

// ディスプレイ表示
void Char_lcd::disp_on(void){
  this->writecmd(0x0C);
}


/* private */
uint8_t Char_lcd::writecmd(uint8_t cmd){
  Wire.beginTransmission(LCD_ADDR);
  Wire.write(0x00);
  Wire.write(cmd);
  return Wire.endTransmission();
}

uint8_t Char_lcd::writedata(uint8_t data){
  Wire.beginTransmission(LCD_ADDR);
  Wire.write(0x40);
  Wire.write(data);
  return Wire.endTransmission();
}
