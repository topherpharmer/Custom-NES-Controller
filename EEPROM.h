#ifndef eeprom_h
#define eeprom_h

extern void eeprom_write(uint16_t address, uint8_t data);
extern uint8_t eeprom_read(uint16_t address);
extern void eeprom_interrupt_enable();
extern void eeprom_interrupt_disable();

#endif