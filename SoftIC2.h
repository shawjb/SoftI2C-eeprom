#include <avr/interrupt.h>

/*

  SDA - PORTB2 - PIN7
  SCL - PORTB1 - PIN6

*/

class SoftIC2{
 
  public:
    SoftIC2(uint8_t _page_size, int _read_bound);
    uint8_t read_address(uint32_t address);
    void write_address(uint32_t address, uint8_t val);
    uint8_t continue_read();
    void begin_read(uint32_t address);
    int read_bound;
    void finish_read();
    void begin_write(uint32_t address);
    void finish_write();
    void continue_write(uint8_t val);
  // private:
    // 4 byte int
    // nothing     block     high      low
    // 00000000 00000000 00000000 00000000
    uint32_t current_address;
    uint8_t to_bound;
    uint8_t page_size;

    void SDA_high();
    void SDA_low();
    void SCL_high();
    void SCL_low();
    void SDA_in();
    void SDA_out();
    void SCL_in();
    void SCL_out();
    
    uint8_t SDA_read();
    void start_condition();
    void stop_condition();
    void wait();
    uint8_t ack();
    void send_ack();
    void send_bit(uint8_t val);
    void send_byte(uint8_t val);
    uint8_t read_byte();
};
