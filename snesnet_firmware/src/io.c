#include "io.h"
#include <avr/interrupt.h>

void initLed() {
	LEDgreenDDR |= (1 << LEDgreen);
	LEDyellowDDR   |= (1 << LEDyellow);
}


void ledOff() {
	LEDgreenPORT &= ~(1 << LEDgreen);
	LEDyellowPORT   &= ~(1 << LEDyellow);
}


void ledOnGreen() {
	LEDyellowPORT   &= ~(1 << LEDyellow);
	LEDgreenPORT |=  (1 << LEDgreen);
}


void ledOnYellow() {
	LEDgreenPORT &= ~(1 << LEDgreen);
	LEDyellowPORT   |=  (1 << LEDyellow);
}


void ledToggleGreen() {
	LEDyellowPORT   &= ~(1 << LEDyellow);
	LEDgreenPORT ^=  (1 << LEDgreen);
}


void ledToggleYellow() {
	LEDgreenPORT &= ~(1 << LEDgreen);
	LEDyellowPORT   ^=  (1 << LEDyellow);
}


void ledSignal(uint8_t times) {
	ledOff();

	while (times > 0) {
		ledToggleGreen();
		_delay_ms(100);
		ledToggleGreen();
		ledToggleYellow();
		_delay_ms(100);
		ledToggleYellow();
		times--;
	}

	ledOff();
}



uint8_t recvGPIO(uint8_t port){

	if(port) return (PINB & (1<<PINB1));
	else return (PINB & (1<<PINB0));
}


void initIO(){
	
	//snes cpu/io bits
	DDRB &= ~(1 << PB0);
	DDRB &= ~(1 << PB1); 

	PORTB |= (1 << PB0); //pullup on
	PORTB |= (1 << PB1); 

	//int0
	DDRD &= ~(1 << DDD2);     // Clear the PD2 pin - io port2
	PORTD |= (1 << PORTD2);   // pullup on
	
	//latch on rising edge
	EICRA |= (1 << ISC01);    
	EICRA |= (1 << ISC00);

    EIMSK |= (1 << INT0);     // Turns on INT10
}


void initInput() {
	InputClockDDR  |= (1 << InputClock);
	InputLatchDDR  |= (1 << InputLatch);

	InputDataDDR   &= ~(1 << InputData);
	InputDataPORT  |=  (1 << InputData);

	InputClockPORT |=  (1 << InputClock);

	InputLatchPORT &= ~(1 << InputLatch);
}


void initOutput() {
	
	//port0 - green plug
	//not connected to int0 PD2 pin 32
	Port0ClockDDR  &= ~(1 << Port0Clock);
	Port0LatchDDR  &= ~(1 << Port0Latch);

	Port0DataDDR  |=  (1 << Port0Data);
	Port0DataPORT |=  (1 << Port0Data);

	Port0ClockPORT |= (1 << Port0Clock);
	Port0LatchPORT |= (1 << Port0Latch);



    //port1 - red plug
	Port1ClockDDR  &= ~(1 << Port1Clock);
	
	Port1DataDDR  |=  (1 << Port1Data);
	Port1DataPORT |=  (1 << Port1Data);

	Port1ClockPORT |= (1 << Port1Clock);
	
	
	//int1 test for port1 PD3 - result: working
	DDRD &= ~(1 << DDD3);     // Clear the PD3 pin - port1

	PORTD |= (1 << PORTD3);
	//latch on rising edge
	EICRA |= (1 << ISC11);    
	EICRA |= (1 << ISC10);

    EIMSK |= (1 << INT1);     // Turns on INT1	
	

}


uint16_t recvInput() {
	uint16_t input = 0;

	// load shift register
	InputLatchPORT |= (1 << InputLatch); //set - high  ___|"" ...
	_delay_us(12);
	InputLatchPORT &= ~(1 << InputLatch); //clear - low  ... ""|___
	_delay_us(3);
	//first bit should now be readable

	for (int i = 0; i < 16; i++) {
		
		InputClockPORT &= ~(1 << InputClock); //clear bit clk ... ""|___

		if (InputDataPIN & (1 << InputData)) //if data high
			input |= (1 << i); //set bit
			_delay_us(6);
			
		
		InputClockPORT |= (1 << InputClock); //set bit clk - shift on rising edge  ___|"" ...
		_delay_us(6);
	}

	return input;
}


void sendPort_0(uint16_t port0) {

	for (int i = 0; i < 16; i++) {
		
		//set first bit after latch
		if ((port0 & (1 << i)) == 0)
			Port0DataPORT &= ~(1 << Port0Data);
		else
			Port0DataPORT |= (1 << Port0Data);
		
		//wait for falling edge - readout point	
		loop_until_bit_is_clear(Port0ClockPIN, Port0Clock); //first half clk 6us
		loop_until_bit_is_set(Port0ClockPIN, Port0Clock); //second half clk 6us
	}
	
	//set port low after shift out
	Port0DataPORT &= ~(1 << Port0Data);
	
}


void sendPort_1(uint16_t port1) {


	for (int i = 0; i < 16; i++) {

		if ((port1 & (1 << i)) == 0)
			Port1DataPORT &= ~(1 << Port1Data);
		else
			Port1DataPORT |= (1 << Port1Data);

		loop_until_bit_is_clear(Port1ClockPIN, Port1Clock); 
		loop_until_bit_is_set(Port1ClockPIN, Port1Clock);
	}
	
	//set port low after shift out
	Port1DataPORT &= ~(1 << Port1Data);
	
}


