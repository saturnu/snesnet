#include <snes.h>
#include "snesnet.h"


unsigned short recvByte(void){

	scanPads();

	return padsCurrent(1);
}

void sendByte(unsigned char byte){
	
	int i;
	for(i=0; i<8; i++){
		
		if(CHECK_BIT(byte,i))
			__sendBit(1);
		else
			__sendBit(0);
	}
	
}
		
void __sendBit(unsigned char type){
		
	if(type==1){
		REG_WRIO=0x40; //01
							
		WaitForVBlank();
		REG_WRIO=0xC0; //11
				
		WaitForVBlank();
		REG_WRIO=0x40; //01	
	}
	else{
	
		REG_WRIO=0x00; //00

		WaitForVBlank();
		REG_WRIO=0x80; //10
			
		WaitForVBlank();
		REG_WRIO=0x00; //00
	}

}
