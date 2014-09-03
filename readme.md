SoftI2C-eeprom
==============
Quick and dirty I2C for the ATTiny85

This was specifically written for use with the ATTiny85 to talk to the microchip 24LCxx series of serial EEPROMs. I had a project that took up both timers so a bit bang approach was needed. This does not support clock stretching.

Don't forget your external pull up resistors the internal pull up resistors don't seem to cut it.  

PB1 => SCL  
PB2 => SDA  

Example Usage
------------------------

`SoftIC2 d = SoftIC2(PAGE_SIZE, MEMORY_BOUND);`  

`d.write_address(ADDRESS_TO_WRITE, VALUE);`  
This will write to single address and will include the start and stop conditions  


`begin_write(ADDRESS_TO_WRITE);`  
Send the start condition with the write flag.

`continue_write(VALUE);`  
Writes to the current address pointer in the epprom.

`begin_read(ADDRESS_TO_READ);`  
Starts a read from the eeprom and sets the internal register

`continue_read();`  
Reads the current value of the read address

`stop_condition();`  
Finishes a read or write



