#include "SoftIC2.h"
#include <avr/interrupt.h>
#include <util/delay.h>
 
SoftIC2::SoftIC2(uint8_t _page_size, int _read_bound){
  current_address = 0;
  to_bound = 0;
  page_size = _page_size;
  read_bound = _read_bound;
}

void SoftIC2::SDA_high(){
  PORTB |= (0x01 << PORTB2);
}
void SoftIC2::SDA_low(){
  PORTB &= ~(0x01 << PORTB2);
}
void SoftIC2::SCL_high(){
  PORTB |= (0x01 << PORTB1);
}
void SoftIC2::SCL_low(){
  PORTB &= ~(0x01 << PORTB1);
}
void SoftIC2::SDA_in(){
  DDRB &= ~(0x01 << DDB2);
  PORTB |= 0x01 << PORTB2;
}
void SoftIC2::SDA_out(){
  DDRB |= 0x01 << DDB2;
  PORTB |= 0x01 << PORTB2;
}
void SoftIC2::SCL_in(){
  DDRB &= ~(0x01 << DDB1);
  PORTB |= 0x01 << PORTB1;
}
void SoftIC2::SCL_out(){
  DDRB |= 0x01 << DDB1;
  PORTB |= 0x01 << PORTB1;
}
uint8_t SoftIC2::SDA_read(){
  return PINB & (0x01 << PINB2);
}

void SoftIC2::start_condition(){
  SDA_out();
  SCL_out();
  wait();
  
  SDA_high();
  SCL_high();
  wait();

  SDA_low();
  wait();

  SCL_low();
}
void SoftIC2::stop_condition(){
  SDA_out();
  SCL_out();
  wait();

  SDA_low();
  SCL_low();
  wait();

  SCL_high();
  wait();

  SDA_high();
}
void SoftIC2::wait(){
  _delay_us(1);
}

uint8_t SoftIC2::ack(){
  uint8_t i;
  SDA_in();
  wait();

  SCL_high();
  wait();

  i = SDA_read();
  SCL_low();
  wait();

  return i;
}
void SoftIC2::send_ack(){
  SDA_out();
  
  SDA_low();
  wait();

  SCL_high();
  wait();

  SCL_low();
  wait();

  SDA_in();
}

void SoftIC2::send_bit(uint8_t val){
  if(val > 0){
    SDA_high();
  }
  else{
    SDA_low();
  }

  SCL_high();
  wait();

  SCL_low();
}

void SoftIC2::send_byte(uint8_t val){
  uint8_t i;
  SDA_out();
  for(i = 0; i < 8; i++){
    send_bit(val & (0x01 << (7 - i)));
  }
  SDA_in();
}

uint8_t SoftIC2::read_byte(){
  uint8_t r = 0;
  uint8_t i;
  for(i = 0; i < 8; i++){
    if(SDA_read()){
      r |= 0x01 << 7 - i;
    }
    wait();
    SCL_high();
    wait();
    SCL_low();
  }
  return r;
}

void SoftIC2::begin_read(uint32_t address){
  current_address = address;
  if(address > read_bound){
    to_bound = (0xffff + 1) - address;
  }else{
    to_bound = read_bound - address;
  }
  uint8_t upper = (address & 0xff00) >> 8;
  uint8_t lower = (address & 0xff);
  
  uint8_t header = 0b10100000;
  if(current_address & 0x10000){
    header |= (0x01 << 3);
  }
  start_condition();

  send_byte(header);
  ack();
  send_byte(upper);
  ack();
  send_byte(lower);
  ack();

  start_condition();
  // set header to read mode
  header |= (0x01 << 0);
  send_byte(header);
}

uint8_t SoftIC2::read_address(uint32_t address){
  begin_read(address);
  // we could use the continue_read but this avoids restarting the transfer
  // for boundary limits then abadoning it, saving us ~ 9 bits woo!
  send_ack();
  uint8_t result = read_byte();
  stop_condition();
  return result;
}

uint8_t SoftIC2::continue_read(){
  uint8_t result;
  send_ack();
  result = read_byte();
  if(to_bound == 0){
    finish_read();
    begin_read(current_address + 1);
  }
  to_bound--;
  current_address++;

  return result;
}
void SoftIC2::finish_read(){
  stop_condition();
}

void SoftIC2::begin_write(uint32_t address){
  current_address = address;
  uint8_t upper = (address & 0xff00) >> 8;
  uint8_t lower = (address & 0xff);
  uint8_t header = 0b10100000;
  if(current_address & 0x10000){
    header |= (0x01 << 3);
  }

  start_condition();
  send_byte(header);
  ack();
  send_byte(upper);
  ack();
  send_byte(lower);

  uint8_t current_page = current_address / page_size;
  to_bound = ((current_page + 1) * page_size) - current_address;
}
void SoftIC2::continue_write(uint8_t val){  
  ack();
  send_byte(val);
  current_address++;
  if(--to_bound == 0){
    finish_write();
    begin_write(current_address);
  }
}

void SoftIC2::write_address(uint32_t address, uint8_t val){
  begin_write(address);
  continue_write(val);
  finish_write();
}

void SoftIC2::finish_write(){
  uint8_t header = 0b10100000;
  if(current_address & 0x10000){
    header |= (0x01 << 3);
  }

  uint8_t result = 1;
  stop_condition();

  start_condition();
  send_byte(header);
  result = ack();
  while(result){
    start_condition();
    send_byte(header);
    result = ack();
  }
  stop_condition();
}
