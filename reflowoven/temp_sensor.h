/*
 * temp_sensor.h
 * written by jp7dki
 */

#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#define SPI_CS_N 5
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23

static const int spiClk = 10000000;   // 1MHz

typedef enum{
  STATE_FAULT,
  STATE_OK
}sensor_state;

// temperature sensor class 
class Temp_sensor
{
  public:
    Temp_sensor();        // Constructor
    void init();
    float read_temp();

  private:
    float thermocouple_temp;
    float internal_temp;
    uint8_t oc_fault;
    uint8_t scg_fault;
    uint8_t scv_fault;
    uint8_t sens_fault;
  
};

#endif
