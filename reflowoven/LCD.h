/*
 * LCD.h
 * written by jp7dki
 */

#ifndef LCD_H
#define LCD_H

#define LCD_ADDR 0x3E

class Char_lcd
{
  public:
    Char_lcd(uint8_t sda_pin, uint8_t scl_pin, uint8_t rst_n_pin);
    void init();
    void write_str(uint8_t row, String str);
    void write_char(uint8_t row, char *str, uint8_t num);
    void disp_on();
    void return_home();
    String temp2str(float temp);

  private:
    uint8_t writecmd(uint8_t cmd);
    uint8_t writedata(uint8_t data);
    uint8_t sda_pin;
    uint8_t scl_pin;
    uint8_t rst_n_pin;
  
};

#endif
