#ifndef usart_h
#define usart_h

#define MAX_CHARACTERS 30

typedef enum {EVEN, ODD, NONE}PARITY;
typedef enum {TRANSMIT, RECEIVE, BOTH}DATA_DIRECTION;

extern void usart_init(int baudRate, int characterSize, int stopBits, PARITY parityType, DATA_DIRECTION direction);
extern void usart_send (char stringIn[]);
extern int usart_completed();
extern void usart_enable_tx();
extern void usart_enable_rx();
extern void usart_disable_tx();
extern void usart_disable_rx();

#endif