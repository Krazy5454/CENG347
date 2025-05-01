#include "fin.h"

int main()
{
  //Serial.begin(115200);
  DDRD = 0x0F; //lcd
  DDRE = 0x10;
  PORTD = 0x0F;
  PORTE = 0x10;
  DDRB = 0x20; //servo port
  DDRF = 0x00; //ADCs


  EICRA = (1 << ISC01 ) | (1 << ISC11) | (1 << ISC21 ) | (1 << ISC31);
  EICRB = (1 << ISC41);
  EIMSK = (1 << INT0) | (1 << INT1) | (1 << INT2) | (1 << INT3) | (1 << INT4);
  sei();

  DDRH = ( 1 << CLK) | ( 1 << DIN ) | ( 1 << CS );
  InitLEDMatrix();
  update_matrix(true);
  ADC_init();
  game_clks_init();
  pwm_init_servo();
  reset();
  while(true){};
  return 1;
}