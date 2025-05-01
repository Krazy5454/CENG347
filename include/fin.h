#ifndef FIN_H
#define FIN_H
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
//#include "Arduino.h"

const int CLK = PH4;
const int DIN = PH6;
const int CS = PH5;

#define VRX_PIN 0  // ADC0
#define VRY_PIN 1  // ADC1

void WriteByte ( unsigned char data );
void WriteData ( unsigned char addr, unsigned char data, unsigned char addr2, unsigned char data2 );
void InitLEDMatrix(void);
void update_matrix( bool show_piece );
void write_L();

void game_loop();
bool fall_piece(); //returns weather screen is above screen
void new_piece();
void update_piece();
bool check_input();
bool check_hit();
void reset();
void clear_lines(bool gravity_change);
void sand_animation();

void game_clks_init();

void pwm_init_servo();
void set_servo_angle(uint8_t angle);

uint16_t mi_random();

void ADC_init();

//joystick
uint16_t analogReadManual(uint8_t channel);

#endif