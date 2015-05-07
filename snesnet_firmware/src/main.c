/*
 * SNESNET v0.1 by saturnu 2015
 * snes network client firmware
Kuroneko!

       :\     /;               _
      ;  \___/  ;             ; ;
     ,:-"'   `"-:.            / ;
_   /,---.   ,---.\   _     _; /
_:>((  |  ) (  |  ))<:_ ,-""_,"
    \`````   `````/""""",-""
     '-.._ v _..-'      )
       / ___   ____,..  \
      / /   | |   | ( \. \
ctr  / /    | |    | |  \ \
     `"     `"     `"    `"

nyannyannyannyannyannyannyannyannyannyannyannyannyannyannyannyannyan
 */

#include <avr/io.h>
#include <stdlib.h> //strtol
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>

#include "io.h"
#include "trivium.h" //encryption


// Mega168 SPI I/O
#define SPI_PORT PORTB
#define SPI_DDR  DDRB
#define SPI_CS   PORTB2
// Wiznet W3150a+ Op Code

#define WIZNET_WRITE_OPCODE 0xF0
#define WIZNET_READ_OPCODE 0x0F

// Wiznet W3510a+ Register Addresses
#define MR         0x0000      // Mode Register
#define GAR        0x0001      // Gateway Address: 0x0001 to 0x0004
#define SUBR       0x0005      // Subnet mask Address: 0x0005 to 0x0008
#define SAR        0x0009      // Source Hardware Address (MAC): 0x0009 to 0x000E
#define SIPR       0x000F      // Source IP Address: 0x000F to 0x0012
#define RMSR       0x001A      // RX Memory Size Register
#define TMSR       0x001B      // TX Memory Size Register
#define S0_MR	   0x0400      // Socket 0: Mode Register Address
#define S0_CR	   0x0401      // Socket 0: Command Register Address
#define S0_IR	   0x0402      // Socket 0: Interrupt Register Address
#define S0_SR	   0x0403      // Socket 0: Status Register Address
#define S0_PORT    0x0404      // Socket 0: Source Port: 0x0404 to 0x0405
//add saturnu
#define S0_DPORT    0x0410      // Socket 0: deest Port: 0x0410 to 0x0411
#define S0_DIPR     0x040C      // dest IP Address: 0x040C to 0x040F
//
#define SO_TX_FSR  0x0420      // Socket 0: Tx Free Size Register: 0x0420 to 0x0421
#define S0_TX_RD   0x0422      // Socket 0: Tx Read Pointer Register: 0x0422 to 0x0423
#define S0_TX_WR   0x0424      // Socket 0: Tx Write Pointer Register: 0x0424 to 0x0425
#define S0_RX_RSR  0x0426      // Socket 0: Rx Received Size Pointer Register: 0x0425 to 0x0427
#define S0_RX_RD   0x0428      // Socket 0: Rx Read Pointer: 0x0428 to 0x0429
#define TXBUFADDR  0x4000      // W3510a+ Send Buffer Base Address
#define RXBUFADDR  0x6000      // W3510a+ Read Buffer Base Address

// S0_MR values
#define MR_CLOSE	  0x00    // Unused socket
#define MR_TCP		  0x01    // TCP
#define MR_UDP		  0x02    // UDP
#define MR_IPRAW	  0x03	  // IP LAYER RAW SOCK
#define MR_MACRAW	  0x04	  // MAC LAYER RAW SOCK
#define MR_PPPOE	  0x05	  // PPPoE
#define MR_ND		  0x20	  // No Delayed Ack(TCP) flag
#define MR_MULTI	  0x80	  // support multicating

// S0_CR values
#define CR_OPEN          0x01	  // Initialize or open socket
#define CR_LISTEN        0x02	  // Wait connection request in tcp mode(Server mode)
#define CR_CONNECT       0x04	  // Send connection request in tcp mode(Client mode)
#define CR_DISCON        0x08	  // Send closing reqeuset in tcp mode
#define CR_CLOSE         0x10	  // Close socket
#define CR_SEND          0x20	  // Update Tx memory pointer and send data
#define CR_SEND_MAC      0x21	  // Send data with MAC address, so without ARP process
#define CR_SEND_KEEP     0x22	  // Send keep alive message
#define CR_RECV          0x40	  // Update Rx memory buffer pointer and receive data

// S0_SR values
#define SOCK_CLOSED      0x00     // Closed
#define SOCK_INIT        0x13	  // Init state
#define SOCK_LISTEN      0x14	  // Listen state
#define SOCK_SYNSENT     0x15	  // Connection state
#define SOCK_SYNRECV     0x16	  // Connection state
#define SOCK_ESTABLISHED 0x17	  // Success to connect
#define SOCK_FIN_WAIT    0x18	  // Closing state
#define SOCK_CLOSING     0x1A	  // Closing state
#define SOCK_TIME_WAIT	 0x1B	  // Closing state
#define SOCK_CLOSE_WAIT  0x1C	  // Closing state
#define SOCK_LAST_ACK    0x1D	  // Closing state
#define SOCK_UDP         0x22	  // UDP socket
#define SOCK_IPRAW       0x32	  // IP raw mode socket
#define SOCK_MACRAW      0x42	  // MAC raw mode socket
#define SOCK_PPPOE       0x5F	  // PPPOE socket
#define TX_BUF_MASK      0x07FF   // Tx 2K Buffer Mask:
#define RX_BUF_MASK      0x07FF   // Rx 2K Buffer Mask:
#define NET_MEMALLOC     0x05     // Use 2K of Tx/Rx Buffer
#define TCP_PORT         0x03E4   // TCP/IP src Port 996
#define TCP_DPORT        0x343F   // TCP/IP dest Port 13375


// Debugging Mode, 0 - Debug OFF, 1 - Debug ON
#define _DEBUG_MODE      0
#if _DEBUG_MODE
  #define BAUD_RATE 9600
#endif


// Define W3510a+ Socket Register and Variables Used
uint8_t sockreg;
#define MAX_BUF 512
uint8_t buf[MAX_BUF];

uint16_t local_port=100;
uint8_t login_state=0;

uint16_t port_0;
uint16_t port_1;



#if _DEBUG_MODE
void uart_init(void)
{
  UBRR0H = (((F_CPU/BAUD_RATE)/16)-1)>>8;		// set baud rate
  UBRR0L = (((F_CPU/BAUD_RATE)/16)-1);
  UCSR0B = (1<<RXEN0)|(1<<TXEN0); 				// enable Rx & Tx
  UCSR0C=  (1<<UCSZ01)|(1<<UCSZ00);  	        // config USART; 8N1
}

void uart_flush(void)
{
  unsigned char dummy;
  while (UCSR0A & (1<<RXC0)) dummy = UDR0;
}

int uart_putch(char ch,FILE *stream)
{
   if (ch == '\n')
    uart_putch('\r', stream);
   while (!(UCSR0A & (1<<UDRE0)));
   UDR0=ch;
   return 0;
}

int uart_getch(FILE *stream)
{
   unsigned char ch;
   while (!(UCSR0A & (1<<RXC0)));
   ch=UDR0;  

   /* Echo the Output Back to terminal */
   uart_putch(ch,stream);       

   return ch;
}

void ansi_cl(void)
{
  // ANSI clear screen: cl=\E[H\E[J
  putchar(27);
  putchar('[');
  putchar('H');
  putchar(27);
  putchar('[');
  putchar('J');
}

void ansi_me(void)
{
  // ANSI turn off all attribute: me=\E[0m
  putchar(27);
  putchar('[');
  putchar('0');
  putchar('m');
}
#endif


void SPI_Write(uint16_t addr,uint8_t data)
{
  // Activate the CS pin
  SPI_PORT &= ~(1<<SPI_CS);
  // Start Wiznet W3150a+ Write OpCode transmission
  SPDR = WIZNET_WRITE_OPCODE;
  // Wait for transmission complete
  while(!(SPSR & (1<<SPIF)));
  // Start Wiznet W3150a+ Address High Bytes transmission
  SPDR = (addr & 0xFF00) >> 8;
  // Wait for transmission complete
  while(!(SPSR & (1<<SPIF)));
  // Start Wiznet W3150a+ Address Low Bytes transmission
  SPDR = addr & 0x00FF;
  // Wait for transmission complete
  while(!(SPSR & (1<<SPIF)));   

  // Start Data transmission
  SPDR = data;
  // Wait for transmission complete
  while(!(SPSR & (1<<SPIF)));
  // CS pin is not active
  SPI_PORT |= (1<<SPI_CS);
}


unsigned char SPI_Read(uint16_t addr)
{
  // Activate the CS pin
  SPI_PORT &= ~(1<<SPI_CS);
  // Start Wiznet W3150a+ Read OpCode transmission
  SPDR = WIZNET_READ_OPCODE;
  // Wait for transmission complete
  while(!(SPSR & (1<<SPIF)));
  // Start Wiznet W3150a+ Address High Bytes transmission
  SPDR = (addr & 0xFF00) >> 8;
  // Wait for transmission complete
  while(!(SPSR & (1<<SPIF)));
  // Start Wiznet W3150a+ Address Low Bytes transmission
  SPDR = addr & 0x00FF;
  // Wait for transmission complete
  while(!(SPSR & (1<<SPIF)));   

  // Send Dummy transmission for reading the data
  SPDR = 0x00;
  // Wait for transmission complete
  while(!(SPSR & (1<<SPIF)));  

  // CS pin is not active
  SPI_PORT |= (1<<SPI_CS);
  return(SPDR);
}


void W3510a_Init(void)
{
  // Ethernet Setup
  unsigned char mac_addr[] = {0x00,0x16,0x36,0xDE,0x58,0xF6};
  unsigned char ip_addr[] = {192,168,123,223};
  unsigned char sub_mask[] = {255,255,255,0};
  unsigned char gtw_addr[] = {192,168,123,249};
  // Setting the Wiznet W3150a+ Mode Register: 0x0000
  SPI_Write(MR,0x80);            // MR = 0b10000000;
  // Setting the Wiznet W3510a+ Gateway Address (GAR): 0x0001 to 0x0004
  SPI_Write(GAR + 0,gtw_addr[0]);
  SPI_Write(GAR + 1,gtw_addr[1]);
  SPI_Write(GAR + 2,gtw_addr[2]);
  SPI_Write(GAR + 3,gtw_addr[3]);
  // Setting the Wiznet W3510a+ Source Address Register (SAR): 0x0009 to 0x000E
  SPI_Write(SAR + 0,mac_addr[0]);
  SPI_Write(SAR + 1,mac_addr[1]);
  SPI_Write(SAR + 2,mac_addr[2]);
  SPI_Write(SAR + 3,mac_addr[3]);
  SPI_Write(SAR + 4,mac_addr[4]);
  SPI_Write(SAR + 5,mac_addr[5]);
  // Setting the Wiznet W3510a+ Sub Mask Address (SUBR): 0x0005 to 0x0008
  SPI_Write(SUBR + 0,sub_mask[0]);
  SPI_Write(SUBR + 1,sub_mask[1]);
  SPI_Write(SUBR + 2,sub_mask[2]);
  SPI_Write(SUBR + 3,sub_mask[3]);
  // Setting the Wiznet W3510a+ IP Address (SIPR): 0x000F to 0x0012
  SPI_Write(SIPR + 0,ip_addr[0]);
  SPI_Write(SIPR + 1,ip_addr[1]);
  SPI_Write(SIPR + 2,ip_addr[2]);
  SPI_Write(SIPR + 3,ip_addr[3]);    

  // Setting the Wiznet W3510a+ RX and TX Memory Size (2KB),
  SPI_Write(RMSR,NET_MEMALLOC);
  SPI_Write(TMSR,NET_MEMALLOC);
}


void close(uint8_t sock)
{
   if (sock != 0) return;

   // Send Close Command
   SPI_Write(S0_CR,CR_CLOSE);
   // Waiting until the S0_CR is clear
   while(SPI_Read(S0_CR));
}


void disconnect(uint8_t sock)
{
   if (sock != 0) return;
   // Send Disconnect Command
   SPI_Write(S0_CR,CR_DISCON);
   // Wait for Disconecting Process
   while(SPI_Read(S0_CR));
}


uint8_t socket(uint8_t protocol, uint16_t port, uint8_t flag)
{
	uint8_t ret;

/*
#ifdef _DEBUG_MODE
	printf("socket()\r\n");
#endif
*/
		close(0);
		   SPI_Write(S0_MR,protocol | flag);
		
		if (port != 0) {
			 SPI_Write(S0_PORT,(uint8_t)((port & 0xff00) >> 8));
		 	 SPI_Write(S0_PORT + 1,(uint8_t)(port & 0x00ff));
		} else {
			local_port++; // if don't set the source port, set local_port number.
			 SPI_Write(S0_PORT,(uint8_t)((local_port & 0xff00) >> 8));
			 SPI_Write(S0_PORT + 1,(uint8_t)(local_port & 0x00ff));
		}
		SPI_Write(S0_CR,CR_OPEN);    // run sockinit Sn_CR
		/* +200804[woong]:wait to process the command... */
		 while(SPI_Read(S0_CR));
		/* ------- */
		ret = 1;

	return ret;
}

/* deactivated for codeshrink
uint8_t listen(uint8_t sock)
{
   uint8_t retval = 0;
   if (sock != 0) return retval;
   if (SPI_Read(S0_SR) == SOCK_INIT) {
     // Send the LISTEN Command
     SPI_Write(S0_CR,CR_LISTEN);

     // Wait for Listening Process
     while(SPI_Read(S0_CR));
     
     // Check for Listen Status
     if (SPI_Read(S0_SR) == SOCK_LISTEN)
       retval=1;
     else
       close(sock);
    }
    return retval;
}
*/

uint8_t connect(uint8_t * addr, uint16_t port)
{
	uint8_t ret;
	/*
#ifdef _DEBUG_MODE
	printf("connect()\r\n");
#endif
*/
	if 
		(
			((addr[0] == 0xFF) && (addr[1] == 0xFF) && (addr[2] == 0xFF) && (addr[3] == 0xFF)) ||
		 	((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
		 	(port == 0x00) 
		) 
 	{
 		ret = 0;
#ifdef _DEBUG_MODE
	printf("Fail[invalid ip,port]\r\n");
#endif
	}
	else
	{

		ret = 1;
		// set destination IP
		SPI_Write( (S0_DIPR + 0), addr[0] );
		SPI_Write( (S0_DIPR + 1), addr[1] );
		SPI_Write( (S0_DIPR + 2), addr[2] );
		SPI_Write( (S0_DIPR + 3), addr[3] );

		SPI_Write( S0_DPORT + 0, (uint8_t)((port & 0xff00) >> 8));
		SPI_Write( S0_DPORT + 1, (uint8_t)(port & 0x00ff));
		
		SPI_Write( S0_CR, CR_CONNECT ); 

		// wait for completion
	    while(SPI_Read(S0_CR))
		{

			if (SPI_Read(S0_SR) == SOCK_CLOSED)
			{
				/*
#ifdef _DEBUG_MODE
			printf("SOCK_CLOSED.\r\n");
#endif
*/
				ret = 0; break;
			}
		}
		  SPI_Write(SUBR + 0, 0x00);
		  SPI_Write(SUBR + 1, 0x00);
		  SPI_Write(SUBR + 2, 0x00);
		  SPI_Write(SUBR + 3, 0x00);
		  
	}

	return ret;
}


uint16_t send(uint8_t sock,const uint8_t *buf,uint16_t buflen)
{
    uint16_t ptr,offaddr,realaddr,txsize,timeout;   

    if (buflen <= 0 || sock != 0) return 0;
    /*
#if _DEBUG_MODE
    printf("Send Size: %d\n",buflen);
#endif
*/
    // Make sure the TX Free Size Register is available
    txsize=SPI_Read(SO_TX_FSR);
    txsize=(((txsize & 0x00FF) << 8 ) + SPI_Read(SO_TX_FSR + 1));
    
    /*
#if _DEBUG_MODE
    printf("TX Free Size: %d\n",txsize);
#endif
*/
    timeout=0;
    while (txsize < buflen) {
      _delay_ms(1);
     txsize=SPI_Read(SO_TX_FSR);
     txsize=(((txsize & 0x00FF) << 8 ) + SPI_Read(SO_TX_FSR + 1));
     // Timeout for approx 1000 ms
     if (timeout++ > 1000) {
		 /*
#if _DEBUG_MODE
       printf("TX Free Size Error!\n");
#endif
* */
       // Disconnect the connection
       disconnect(sock);
       return 0;
     }
   }	

   // Read the Tx Write Pointer
   ptr = SPI_Read(S0_TX_WR);
   offaddr = (((ptr & 0x00FF) << 8 ) + SPI_Read(S0_TX_WR + 1));
   /*
#if _DEBUG_MODE
    printf("TX Buffer: %x\n",offaddr);
#endif	
*/

    while(buflen) {
      buflen--;
      // Calculate the real W3510a+ physical Tx Buffer Address
      realaddr = TXBUFADDR + (offaddr & TX_BUF_MASK);
      // Copy the application data to the W5100 Tx Buffer
      SPI_Write(realaddr,*buf);
      offaddr++;
      buf++;
    }

    // Increase the S0_TX_WR value, so it point to the next transmit
    SPI_Write(S0_TX_WR,(offaddr & 0xFF00) >> 8 );
    SPI_Write(S0_TX_WR + 1,(offaddr & 0x00FF));	

    // Now Send the SEND command
    SPI_Write(S0_CR,CR_SEND);

    // Wait for Sending Process
    while(SPI_Read(S0_CR));	

    return 1;
}


uint16_t recv(uint8_t sock,uint8_t *buf,uint16_t buflen)
{
    uint16_t ptr,offaddr,realaddr;   	

    if (buflen <= 0 || sock != 0) return 1;   

    // If the request size > MAX_BUF,just truncate it
    if (buflen > MAX_BUF)
      buflen=MAX_BUF - 2;
    // Read the Rx Read Pointer
    ptr = SPI_Read(S0_RX_RD);
    offaddr = (((ptr & 0x00FF) << 8 ) + SPI_Read(S0_RX_RD + 1));
    /*
#if _DEBUG_MODE
    printf("RX Buffer: %x\n",offaddr);
#endif	
*/

    while(buflen) {
      buflen--;
      realaddr=RXBUFADDR + (offaddr & RX_BUF_MASK);
      *buf = SPI_Read(realaddr);
      offaddr++;
      buf++;
    }
    *buf='\0';        // String terminated character

    // Increase the S0_RX_RD value, so it point to the next receive
    SPI_Write(S0_RX_RD,(offaddr & 0xFF00) >> 8 );
    SPI_Write(S0_RX_RD + 1,(offaddr & 0x00FF));	

    // Now Send the RECV command
    SPI_Write(S0_CR,CR_RECV);
    _delay_us(5);    // Wait for Receive Process

    return 1;
}


uint16_t recv_size(void)
{
  return ((SPI_Read(S0_RX_RSR) & 0x00FF) << 8 ) + SPI_Read(S0_RX_RSR + 1);
}

/*
int strindex(char *s,char *t)
{
  uint16_t i,n;

  n=strlen(t);
  for(i=0;*(s+i); i++) {
    if (strncmp(s+i,t,n) == 0)
      return i;
  }
  return -1;
}
*/

//called by incoming thread	
int process_incoming(void* rec_buffer, int size){

	uint8_t* state = (uint8_t*)rec_buffer;

	int ptr=0;
	
	//printf("pi size %d\n", size);
	
	//security cutoff
	//if(size%2 != 0) size--;
	
	while(ptr<size){

	//16 bit snes controller
	//111111              bitpos (msb <- lsb)
	//54321098 76543210
	//00000000 00000000   16bit low(0) = enable
	//zzzzRLXA RLDUSSBY   mapping
	//4321     ieoptl
	
		// Software reset: L + R + Select + Start.
		// 0b 11110011 11110011

//down?
//0b1111111111001111

	//check if lsb 3 - z1 is high
	//->special command and no controller input
	/*
	if(CHECK_BIT(state[ptr],4) == 0){
			
			char sbuff[2];
			sbuff[1]='\0';
			
			//do something special here
			if(recvGPIO(0)){ //test only: default 0 bit 6
				sbuff[0]='B';
			}else{
				sbuff[0]='A';
			}
			
			//sending...
			if (send(sockreg, sbuff, strlen((char *)sbuff)) <= 0){
					 //error
			 }
		*/
		
	//}else{
		//just controller data
		port_1=0xFFFF;
		port_1 = (state[ptr] << 8) + state[ptr+1];
		
	//	printf("size %d value %04x\n", size, port_1);
		//_delay_ms(20);
	//}
	
	ptr+=2;
	
	_delay_us(10);

	}
	
	return 0;
}


//controller_1 latch
ISR(INT1_vect)
{
	//start shiftig controller bits out
	sendPort_1(port_1);
}	

//port2 io
ISR(INT0_vect)
{
	static uint8_t in_byte=0;
	static uint8_t in_byte_cnt=0;
	
	if(recvGPIO(0)){ //port1 io
		SET_BIT(in_byte,in_byte_cnt);
	}else{
		CLEAR_BIT(in_byte,in_byte_cnt);
	}
/*
		#if _DEBUG_MODE
				printf(": in_byte_cnt: %d\n",in_byte_cnt);
		#endif
*/

	if(in_byte_cnt==7){
		char sbuff[5];
		sbuff[0]='C';
		snprintf(sbuff+1, 3,"%02x", (char)in_byte);
		//sbuff[1]=(char)in_byte; //bug 0b00000000 -> endline
		sbuff[3]='\n';
		sbuff[4]='\0';
		
		if(login_state==3)
		if (send(sockreg, sbuff, strlen((char *)sbuff)) <= 0){
				 //error
		 }else{
			 /*
		 		#if _DEBUG_MODE
				printf(": sent\n");
		#endif
		*/
		 }
		in_byte_cnt=0; 
	}else{
		
		in_byte_cnt++;
	}

}	


#if _DEBUG_MODE
// Assign I/O stream to UART
FILE uart_str = FDEV_SETUP_STREAM(uart_putch, uart_getch, _FDEV_SETUP_RW);
#endif


int main(void){
  uint8_t sockstat;
  uint16_t rsize;
 // char radiostat0[10],radiostat1[10],temp[4];
  //int getidx,postidx;
  port_0=0xFFFF;
  port_1=0xFFFF;

//TODO set in EEPROM

	//server ip
	uint8_t dest_ip_addr[] = {192,168,123,222};
	
	//login data
	char username[] = "Uclient0\n"; //Uxxx\n
	char password[] = "oisjoseoisiusde";	 //Pxxx\n 
	char key[] = "0AnL]d`50m";		 

  // Reset Port D
  //DDRD = 0xFF;       // Set PORTD as Output 
  DDRD = 0b11110011; 
  PORTD = 0x00;	     

#if _DEBUG_MODE
  // Define Output/Input Stream
  stdout = stdin = &uart_str;
  // Initial UART Peripheral
  uart_init();
  // Clear Screen
  ansi_me();
  ansi_cl();
  ansi_me();
  ansi_cl();
  uart_flush();
#endif

  // Initial the AVR ATMega168 SPI Peripheral
  // Set MOSI (PORTB3),SCK (PORTB5) and PORTB2 (SS) as output, others as input
  SPI_DDR = (1<<PORTB3)|(1<<PORTB5)|(1<<PORTB2);
  // CS pin is not active
  SPI_PORT |= (1<<SPI_CS);
  // Enable SPI, Master Mode 0, set the clock rate fck/2
  SPCR = (1<<SPE)|(1<<MSTR);
  SPSR |= (1<<SPI2X);  

	/* config end */
	
 	//io bits
	initIO();
	
	// Initial the W3510a+ Ethernet
	W3510a_Init();
	
	// Initial variable used
	sockreg=0;
	
	// init input ports
	initInput();
	// init output ports
	initOutput();

	sei();  // Enable Interrupt (for port1 latch PD3/INT1) and iobit port2
 
	initLed();
 	ledOnYellow(); //user signal bootup ok waiting for connection
 
 /*
	#if _DEBUG_MODE
	  printf("TCP Client Debug Mode\n\n");
	#endif
*/

 uint8_t status=0;
 uint8_t sock_state=0;
	for(;;){

    sockstat=SPI_Read(S0_SR);
    
		/*
    	#if _DEBUG_MODE
				  printf("Socket sockstat: %d\n",sockstat); //what is 1? ^^
		#endif
		*/
	
		switch (sockstat)
	{
		
		//0x17
	case SOCK_ESTABLISHED:						/* if connection is established */
	
		//port_0 = recvInput();
		
		if(sock_state==1)
		{
		/*
		#if _DEBUG_MODE
				printf(": Connected");
		#endif
		*/

			ledOnGreen();		
			sock_state = 2;
		}
		
		
		_delay_ms(1); //bugfix delay
		rsize=recv_size();
		
		if(login_state==3){ //check rx buffer
			  //  rsize=recv_size();

				//write to snes button port  
				if (rsize > 0) {
				_delay_ms(1); //bugfix delay, 
				  if (recv(sockreg,buf,rsize) <= 0){
					  }else //read tcp data to buffer , u8 buffer
				    	process_incoming(buf, rsize);
			  
			 
					// Disconnect the socket
					//disconnect(sockreg);
				  
				  
					} else{
					_delay_us(10);    // Wait for request

					}
			}//ls3
			else
		if(login_state==0){ 
				char sbuff[4];
				
				//send username plain - non hex
				if (send(sockreg, username, strlen((char *)username)) <= 0){
				 //sending error signal
					 ledSignal(20);
					 ledOnYellow(); 
				 }
				 login_state=1;
			 }//ls0
			 else
			 if(login_state==1){

				//wait for iv or error
			//	rsize=recv_size();
				if (rsize > 0) {
					if (recv(sockreg,buf,rsize) <= 0){
						//no data in rx buffer
					}else{ //data in rx buffer
							//data found check for errors
							//secure strcmp workaround
							if(rsize >= 5)
							if(buf[0]=='E' && buf[1]=='R' && buf[2]=='R' && buf[3]=='O' && buf[4]=='R') {
								//signal error disconnect
								ledSignal(20);
								ledOnYellow(); 
							}else{
								if(buf[0]=='I' && buf[1]=='V') {
								//should now be iv
									//crypt passwort
										
										trivium_ctx_t ctx;
										//key = key
										//buf+3 = iv bits
										trivium_init(key, 80, buf+3, 80, &ctx);
										trivium_enc(&ctx);

										unsigned char passwd_[strlen(password)];
										
										for(int g=0; g<strlen(password);g++)
										passwd_[g] = password[g] ^ trivium_getbyte(&ctx);

										//strlen of passwd_ might be wrong 'cause of string termination
										//dump(passwd_,strlen(passwd));


										char *raw_crypt = (char*) malloc(strlen(password)*2);
										memset(raw_crypt,0,strlen(password)*2);
										uint8_t *m = passwd_;
										int c = sizeof(passwd_);
										while(c--)
										{
											char chr[2];

											snprintf(chr, 3,"%02x", *(m++));
											strcat(raw_crypt, chr);
										}
										
										snprintf(buf, MAX_BUF, "P%s\n",raw_crypt);
										free(raw_crypt);
										
										
										if (send(sockreg, buf, strlen((char *)buf)) <= 0){
											//error
										}
									
									_delay_ms(100);
									
									 login_state=2;
								}else{
								    ledSignal(20);
							    	ledOnYellow(); 
								}
					
					
					/*
					if (send(sockreg, password, strlen((char *)password)) <= 0){
							 //error
					 }
					 */
								}
						}
					}
			 }//ls1
			 else
			  if(login_state==2){

					//rsize=recv_size();
					/*
						#if _DEBUG_MODE
						printf("size: %d\n", rsize);
						#endif
					*/
					
					if (rsize > 0) {
						if (recv(sockreg,buf,rsize) <= 0){
							//no data in rx buffer
											//	#if _DEBUG_MODE
											//	printf("auth: 0\n");
											//	#endif
						}else{
							//'OK' or 'ERROR'
							if(rsize >= 5){
									if(buf[0]=='E' && buf[1]=='R' && buf[2]=='R' && buf[3]=='O' && buf[4]=='R') {
											ledSignal(20);
											ledOnYellow(); 
													#if _DEBUG_MODE
													printf("auth: error\n");
													#endif
													
									}
								}else if(rsize >= 3){
											if(buf[0]=='O' && buf[1]=='K'){
												//auth ok?
												
														#if _DEBUG_MODE
														printf("auth: ok\n");
														#endif
														
												login_state=3;
										}
									}
							  
							}
					}
					_delay_ms(100);
					/*
					if (send(sockreg, password, strlen((char *)password)) <= 0){
							 //error
					 }
					 */
					
			 }//ls2

		
		break;
	case SOCK_CLOSE_WAIT:     // If the client request to close
		
		/*
		#if _DEBUG_MODE
			printf(": CLOSE_WAIT");	
		#endif
		*/
		disconnect(0);
		sock_state = 0;
		break;
		
		//0x00
	case SOCK_CLOSED:   // if a socket is closed
	
		ledOnYellow(); 
	
		if(!sock_state)
		{
		/*
		#if _DEBUG_MODE
				printf(": Loop-Back TCP Client Started.");
		#endif
			*/
			
			sock_state = 1;
		}
		
		if(socket(MR_TCP,local_port++,0x00) == 0)    // reinitialize the socket 
		{
			/*
		#if _DEBUG_MODE
				printf("\a: Fail to create socket.");
		#endif
		*/
			sock_state = 0;
		}
		else{
		_delay_ms(1);
		connect(dest_ip_addr,TCP_DPORT);
		}
		break;
	
	  case SOCK_FIN_WAIT:
      case SOCK_CLOSING:
      case SOCK_TIME_WAIT:
      case SOCK_LAST_ACK:
        // Force to close the socket
	close(sockreg);
	ledOnYellow(); 
	
	#if _DEBUG_MODE
		printf("Socket Close!\n");
	#endif
	
	}
    
  }
  return 0;
}
