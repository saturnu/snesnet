
#include <snes.h>
#include "snesnet.h"

extern char patterns, patterns_end;
extern char palette, palette_end;
extern char map, map_end;

extern char snesfont;

int pos=0;

char str[26];

//---------------------------------------------------------------------------------


void clean(void){
	consoleDrawText(20,14,"         "); 
	consoleDrawText(22,19," "); 
	consoleDrawText(25,21," "); 
	consoleDrawText(22,23," ");
	consoleDrawText(20,21," ");
	consoleDrawText(12,20," ");
	consoleDrawText(15,20," ");  	
}

void addChar(int set_id, int key_id, char chr){
	int pad0 = padsCurrent(0);
	


	str[pos]=chr;
	str[pos+1]=0x00;
	consoleDrawText(3+pos,9,"%c",chr);

	
	
	if(pos<25) pos++;
	
	
	while((pad0&BIT(set_id))&&(pad0&BIT(key_id))){				
		scanPads();
		pad0  = padsCurrent(0);
	}
}


void sendString(void){
	
	consoleDrawText(7, 5, "sending...");
	
	int length = strlen(str);
	
	sendByte(length);
	
	int i;
	for(i=0;i<length;i++){
		sendByte(str[i]);
	}

}



int main(void) {
    // Initialize SNES 
	consoleInit();

    // Initialize text console with our font
	consoleInitText(0, 1, &snesfont);

	 
	// Copy tiles to VRAM
	bgInitTileSet(1, &patterns, &palette, 0, (&patterns_end - &patterns), (&palette_end - &palette), BG_16COLORS, 0x4000);

	// Copy Map to VRAM
	bgInitMapSet(1, &map, (&map_end - &map),SC_32x32, 0x0000);

	// Now Put in 16 color mode and disable other BGs except 1st one
	//setMode(BG_MODE1,0);  bgSetDisable(1); bgSetDisable(2);
		setMode(BG_MODE1,0);  bgSetDisable(2);
	
	// Just inform user
 //   consoleDrawText(1,1,"test text");
	
	// Wait for nothing :P
	//while(1) {
	//	WaitForVBlank();
	//}
	
	
	
	sendByte(0x02);	//send magic byte
	
	
	int pad0;
	int set=0;
		while(1) {
			set=0;

		// Refresh pad values
		scanPads();
		
		// Get current #0 pad
		pad0 = padsCurrent(0);
		clean();
		// Update scrolling with current pad
		switch (pad0) {
			case  KEY_UP	:
								set=1;
								consoleDrawText(22,14,"g");   //7 r
								consoleDrawText(22,19,"a");   //1 x
								 consoleDrawText(25,21,"b");  //2 a
								 consoleDrawText(22,23,"c");  //3 b
								 consoleDrawText(20,21,"d");  //4 y
								 consoleDrawText(12,20,"e");  //5 se 
								 consoleDrawText(15,20,"f");  //6 st
								 				 			
								break;
			case  KEY_RIGHT	:  
								set=2;
								consoleDrawText(22,14,"n"); 
								consoleDrawText(22,19,"h"); 
								 consoleDrawText(25,21,"i"); 
								 consoleDrawText(22,23,"j");
								 consoleDrawText(20,21,"k");
								 consoleDrawText(12,20,"l");
								 consoleDrawText(15,20,"m");  			
							break;
			case KEY_DOWN	:  
								set=3;
								consoleDrawText(22,14,"u"); 
								consoleDrawText(22,19,"o"); 
								 consoleDrawText(25,21,"p"); 
								 consoleDrawText(22,23,"q");
								 consoleDrawText(20,21,"r");
								 consoleDrawText(12,20,"s");
								 consoleDrawText(15,20,"t");  				
							break;
			case 	KEY_LEFT	:  
								set=4;
								consoleDrawText(20,14,"blank"); 
								consoleDrawText(22,19,"v"); 
								 consoleDrawText(25,21,"w"); 
								 consoleDrawText(22,23,"x");
								 consoleDrawText(20,21,"y");
								 consoleDrawText(12,20,"z");
								 consoleDrawText(15,20," ");  				
							break;
			case 	KEY_L	: 
			
							if(pos!=0){	pos--;
								str[pos]=0x00;
								consoleDrawText(3+pos,9,"                          ");
							}
						while(pad0 & KEY_L){
												
							scanPads();
							pad0 = padsCurrent(0);
						}
						
							break;		
			case 	KEY_R	: 
			
						sendString();
						while(pad0 & KEY_R){
												
							scanPads();
							pad0 = padsCurrent(0);
						}
						
							break;											
		}
		
/*
   KEY_A      = BIT(7),  //!< pad A button.
  KEY_B      = BIT(15),  //!< pad B button.
  KEY_SELECT = BIT(13),  //!< pad SELECT button.
  KEY_START  = BIT(12),  //!< pad START button.
  KEY_RIGHT  = BIT(8),  //!< pad RIGHT button.
  KEY_LEFT   = BIT(9),  //!< pad LEFT button.
  KEY_DOWN   = BIT(10),  //!< pad DOWN button.
  KEY_UP     = BIT(11),  //!< pad UP button.
  KEY_R      = BIT(4),  //!< Right shoulder button.
  KEY_L      = BIT(5),  //!< Left shoulder button.
  KEY_X      = BIT(6), //!< pad X button.
  KEY_Y      = BIT(14), //!< pad Y button.
  * */

									
			
		if(  (pad0&KEY_UP)&&(pad0&KEY_A)  ){ addChar(11,7,'b'); }
		if(  (pad0&KEY_UP)&&(pad0&KEY_B)  ){ addChar(11,15,'c'); }
		if(  (pad0&KEY_UP)&&(pad0&KEY_X)  ){ addChar(11,6,'a'); }
		if(  (pad0&KEY_UP)&&(pad0&KEY_Y)  ){ addChar(11,14,'d'); }		
		if(  (pad0&KEY_UP)&&(pad0&KEY_START)  ){ addChar(11,12,'f'); }		
		if(  (pad0&KEY_UP)&&(pad0&KEY_SELECT)  ){ addChar(11,13,'e'); }		
		if(  (pad0&KEY_UP)&&(pad0&KEY_R)  ){ addChar(11,4,'g'); }		
		
		if(  (pad0&KEY_DOWN)&&(pad0&KEY_A)  ){ addChar(10,7,'p'); }
		if(  (pad0&KEY_DOWN)&&(pad0&KEY_B)  ){ addChar(10,15,'q'); }
		if(  (pad0&KEY_DOWN)&&(pad0&KEY_X)  ){ addChar(10,6,'o'); }
		if(  (pad0&KEY_DOWN)&&(pad0&KEY_Y)  ){ addChar(10,14,'r'); }		
		if(  (pad0&KEY_DOWN)&&(pad0&KEY_START)  ){ addChar(10,12,'t'); }		
		if(  (pad0&KEY_DOWN)&&(pad0&KEY_SELECT)  ){ addChar(10,13,'s'); }		
		if(  (pad0&KEY_DOWN)&&(pad0&KEY_R)  ){ addChar(10,4,'u'); }	
		
		if(  (pad0&KEY_LEFT)&&(pad0&KEY_A)  ){ addChar(9,7,'w'); }
		if(  (pad0&KEY_LEFT)&&(pad0&KEY_B)  ){ addChar(9,15,'x'); }
		if(  (pad0&KEY_LEFT)&&(pad0&KEY_X)  ){ addChar(9,6,'v'); }
		if(  (pad0&KEY_LEFT)&&(pad0&KEY_Y)  ){ addChar(9,14,'y'); }		
		//if(  (pad0&KEY_LEFT)&&(pad0&KEY_START)  ){ addChar(9,12,'f'); }		
		if(  (pad0&KEY_LEFT)&&(pad0&KEY_SELECT)  ){ addChar(9,13,'z'); }		
		if(  (pad0&KEY_LEFT)&&(pad0&KEY_R)  ){ addChar(9,4,' '); }	
				
		if(  (pad0&KEY_RIGHT)&&(pad0&KEY_A)  ){ addChar(8,7,'i'); }
		if(  (pad0&KEY_RIGHT)&&(pad0&KEY_B)  ){ addChar(8,15,'j'); }
		if(  (pad0&KEY_RIGHT)&&(pad0&KEY_X)  ){ addChar(8,6,'h'); }
		if(  (pad0&KEY_RIGHT)&&(pad0&KEY_Y)  ){ addChar(8,14,'k'); }		
		if(  (pad0&KEY_RIGHT)&&(pad0&KEY_START)  ){ addChar(8,12,'m'); }		
		if(  (pad0&KEY_RIGHT)&&(pad0&KEY_SELECT)  ){ addChar(8,13,'l'); }		
		if(  (pad0&KEY_RIGHT)&&(pad0&KEY_R)  ){ addChar(8,4,'n'); }			
		
		
		

		consoleDrawText(7,3,"snesnet text demo");

        consoleDrawText(7,14,"back");
        if(set==0) consoleDrawText(20,14,"send");
        consoleDrawText(6,17,"set");
        
        
		WaitForVBlank();
	}
	
	return 0;
}
