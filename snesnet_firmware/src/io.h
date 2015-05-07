#include <avr/io.h>
#include <util/delay.h>

#define LEDgreenDDR   DDRC
#define LEDgreenPORT  PORTC
#define LEDgreen      PC0

#define LEDyellowDDR     DDRC
#define LEDyellowPORT    PORTC
#define LEDyellow        PC1


#define InputClockDDR  DDRD
#define InputClockPORT PORTD
#define InputClock     PD5

#define InputLatchDDR  DDRD
#define InputLatchPORT PORTD
#define InputLatch     PD6

#define InputDataDDR   DDRD
#define InputDataPORT  PORTD
#define InputDataPIN   PIND
#define InputData      PD7


#define Port0ClockDDR  DDRC
#define Port0ClockPORT PORTC
#define Port0ClockPIN  PINC
#define Port0Clock     PC2

#define Port0LatchDDR  DDRC
#define Port0LatchPORT PORTC
#define Port0LatchPIN  PINC
#define Port0Latch     PC3

#define Port0DataDDR   DDRC
#define Port0DataPORT  PORTC
#define Port0Data      PC4


#define Port1ClockDDR  DDRD
#define Port1ClockPORT PORTD
#define Port1ClockPIN  PIND
#define Port1Clock     PD4

#define Port1LatchDDR  DDRD
#define Port1LatchPORT PORTD
#define Port1LatchPIN  PIND
#define Port1Latch     PD3 //int1

#define Port1DataDDR   DDRC //PORTD
#define Port1DataPORT  PORTC //PORTD
#define Port1Data      PC5 //PD2 freed for int0 -> wiznet interrupt new (PC5)

// IO BIT
#define Port0IODDR   DDRB
#define Port0IOPORT  PORTB
#define Port0IOData  PB0

#define Port1IODDR   DDRB
#define Port1IOPORT  PORTB
#define Port1IOData  PB1
//IO BIT END

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define SET_BIT(var,pos) ((var) |= 1 << pos)
#define CLEAR_BIT(var,pos) ((var) &= ~(1 << pos))
#define TOGGLE_BIT(var,pos) ((var) ^= 1 << pos)


void     initInput(void);
void     initOutput(void);
uint8_t recvGPIO(uint8_t port);
uint16_t   recvInput(void);
void initIO(void);
//void     sendPorts(uint16_t port0, uint16_t port1);
void     sendPort_0(uint16_t port0);
void     sendPort_1(uint16_t port1);


void initLed(void);
void ledOff(void);
void ledOnGreen(void);
void ledOnYellow(void);
void ledToggleGreen(void);
void ledToggleYellow(void);
void ledSignal(uint8_t times);
