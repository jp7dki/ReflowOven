/*
 * temp_sensor.cpp
 * written by jp7dki
 */

#include <Arduino.h>
#include <SPI.h>
#include "temp_sensor.h"


// コンストラクタ
Temp_sensor::Temp_sensor(void)
{
  
}

void Temp_sensor::init(void)
{
  SPI.begin(SPI_SCK,SPI_MISO,SPI_MOSI,SPI_CS_N);
  pinMode(SPI_CS_N, OUTPUT);     // VSPI SS
  digitalWrite(SPI_CS_N, HIGH);  // Slave disable
}

float Temp_sensor::read_temp(void)
{
  uint32_t temp;
  
  SPI.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(SPI_CS_N, LOW);    // CS_N falling
  temp = SPI.transfer32(0x0000);    // read temp
  digitalWrite(SPI_CS_N, HIGH);   // CS_N rising
  SPI.endTransaction();

  if(temp&0x00000001 != 0){
    this->oc_fault = STATE_FAULT;
  }else{
    this->oc_fault = STATE_OK;
  }

  if(temp&0x00000002 != 0){
    this->scg_fault = STATE_FAULT;
  }else{
    this->scg_fault = STATE_OK;
  }

  if(temp&0x00000004 != 0){
    this->scv_fault = STATE_FAULT;
  }else{
    this->scv_fault = STATE_OK;
  }

  if(temp&0x00010000 != 0){
    this->sens_fault = STATE_FAULT;
  }else{
    this->sens_fault = STATE_OK;
  }
  
  this->thermocouple_temp = (float)(0x3FFF & (temp>>18)) * 0.25;
  this->internal_temp = (float)(0xFFF & (temp>>4)) * 0.0625;

  return thermocouple_temp;
}
