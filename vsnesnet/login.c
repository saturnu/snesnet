#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <sys/types.h>


#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __SVR4
	#include <sys/stropts.h>
#endif


#include <pthread.h>
#include <sched.h>
#include <semaphore.h>

#include <fcntl.h>
#include "trivium.h"
#include "vsnesnet.h"

#define RCVBUFSIZE 128   /* Size of receive buffer */
//#define MAX_MSG_SIZE 257 // We will never send more than 256 bytes (+\0)

#define G(i) ((((*ctx)[(i)/8])>>(((i)%8)))&1)
#define S(i,v) ((*ctx)[(i)/8] = (((*ctx)[(i)/8]) & (uint8_t)~(1<<((i)%8))) | ((v)<<((i)%8)))
int sock;
struct sockaddr_in snesnetServAddr;


//port2 io
void addIOBit(uint8_t bit)
{
	static uint8_t in_byte=0;
	static uint8_t in_byte_cnt=0;
	
	if(bit){ //port1 io
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
		
		
		
		
		//if(login_state==3)
		if (send(sock, sbuff, strlen((char *)sbuff), 0) <= 0){
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


void *recv_server_data(void* val) {

	uint8_t inc_data[512];
	
	process_incoming(0xFF, 0xFF);
	
	usleep(10);
	while(1){


				uint16_t recv_size = recv(sock, inc_data, sizeof(uint8_t)*32, 0);
				if (recv_size <= 0)
				{
						printf("recv() failed or connection closed prematurely");
				break;
				}


				int ptr=0;
				
				//security cutoff
				if(recv_size%2 != 0) recv_size--;
				
				while(ptr<recv_size){
	
					uint8_t byte0 = inc_data[ptr];//(uint8_t)((inc_data[0] & 0xFF00) >> 8);
					uint8_t byte1 = inc_data[ptr+1];//(uint8_t)(inc_data[0] & 0x00FF);
					
					process_incoming(byte0, byte1);
					
					ptr+=2;
					usleep(10);
				//uint16_t bt2 = byte0 + (byte1 << 8) ;
				//printf("bt hack: [%04x]\n",bt2);
				}
				

			
	}

	return NULL;


}


void *readout_fifo(void* val) {

	int in_fd;
	mkfifo(FIFO_FILE, 0666); //0667 is not working (masked?) :>
	
	//root process change permission to others +w
	int mode=0x00;
	mode |= S_IWOTH;
	mode |= S_IRUSR;
	mode |= S_IWUSR;
	
	if(chmod(FIFO_FILE, mode))
	printf("error: chmod /tmp/snes_wrio.fifo\n");
	
	char in[RCVBUFSIZE];
	in_fd=open(FIFO_FILE, O_RDWR);
	
	
	if (in_fd==-1) {
        perror("error: fifo - file open()");
        exit(-1);
    }
	
	while(1){

    while (read(in_fd, in, RCVBUFSIZE)>0) {
        //printf("%s", in);
        
        if(in[0]==0x31)
        addIOBit(1);
        else
        addIOBit(0);
        
    }




		/*
		//incomming controller data
			if(!_SNESnet_master){ // i'm slave
				socklen_t len;
				unsigned char remote_mesg[8];
			
				len = sizeof(_SNESnet_client_addr);
				recvfrom(_SNESnet_socket_tcp, remote_mesg,32,0,(struct sockaddr *)&_SNESnet_server_addr,&len);
				
				unsigned char b0[2];
				unsigned char b1[2];
				b0[0] = remote_mesg[0];
				b0[1] = remote_mesg[1];
				b1[0] = remote_mesg[2];
				b1[1] = remote_mesg[3];
				
				unsigned char byte0 = strtol(b0, NULL, 16);
				unsigned char byte1 = strtol(b1, NULL, 16);
				
				process_incoming(byte0, byte1);
				
				//uint16_t bt = byte0 + (byte1 << 8) ;
				//printf("bt hack: [%d]\n",bt);
			}
			*/
	}

	return NULL;
}



void stop_snesnet()
{
  close(sock);
}

int init_login(char *confFile)
{
	
	const char *loginserver;
	const char *username;
	const char *password;
	const char *key;
	debug=0;
	/*
	 int *server_port;
	 int *tcp_min_port;
	 int *tcp_max_port;
*/


	// Initialise and read configuration file.
	config_t          conf;
	//config_setting_t *setting;
	config_init(&conf);


		if (! config_read_file(&conf, confFile)) {
		config_destroy(&conf);
		printf("... %s: wrong file format or file does not exist.\n", confFile);
		return -1;
	}
		if ( (! config_lookup_string(&conf, "loginserver", &loginserver) ) || (strlen(loginserver) == 0) ) {
		printf("... %s: loginserver is not set\n", confFile);
		return -1;
	}
		if ( (! config_lookup_string(&conf, "username", &username) ) || (strlen(username) == 0) ) {
		printf("... %s: username is not set\n", confFile);
		return -1;
	}
		if ( (! config_lookup_string(&conf, "password", &password) ) || (strlen(password) == 0) ) {
		printf("... %s: password is not set\n", confFile);
		return -1;
	}
		if ( (! config_lookup_string(&conf, "key", &key) ) || (strlen(key) == 0) ) {
		printf("... %s: key is not set\n", confFile);
		return -1;
	}
		if ( (! config_lookup_string(&conf, "js_dev", &js_dev) ) || (strlen(js_dev) == 0) ) {
		printf("... %s: js_dev is not set\n", confFile);
		return -1;
	}
		if ( (! config_lookup_int(&conf, "server_port", &_SNESnet_server_port) ) || _SNESnet_server_port == 0)  {
		printf("... %s: server_port is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_axis_type", &js_axis_type))   {
		printf("... %s: js_axis_type is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_button_type", &js_button_type))   {
		printf("... %s: js_button_type is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_axis_up_val", &js_axis_up_val))   {
		printf("... %s: js_axis_up_val is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_axis_down_val", &js_axis_down_val))   {
		printf("... %s: js_axis_down_val is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_axis_left_val", &js_axis_left_val))   {
		printf("... %s: js_axis_left_val is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_axis_right_val", &js_axis_right_val))   {
		printf("... %s: js_axis_right_val is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_axis_y_nr", &js_axis_y_nr))   {
		printf("... %s: js_axis_y_nr is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_axis_x_nr", &js_axis_x_nr))   {
		printf("... %s: js_axis_x_nr is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_button_a_nr", &js_button_a_nr))   {
		printf("... %s: js_button_a_nr is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_button_b_nr", &js_button_b_nr))   {
		printf("... %s: js_button_b_nr is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_button_x_nr", &js_button_x_nr))   {
		printf("... %s: js_button_x_nr is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_button_y_nr", &js_button_y_nr))   {
		printf("... %s: js_button_y_nr is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_button_l_nr", &js_button_l_nr))   {
		printf("... %s: js_button_l_nr is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_button_r_nr", &js_button_r_nr))   {
		printf("... %s: js_button_r_nr is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_button_st_nr", &js_button_st_nr))   {
		printf("... %s: js_button_st_nr is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "js_button_se_nr", &js_button_se_nr))   {
		printf("... %s: js_button_se_nr is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "disable_p1", &disable_p1))   {
		printf("... %s: disable_p1 is not set\n", confFile);
		return -1;
	}
		if ( ! config_lookup_int(&conf, "select_x", &select_x))   {
		printf("... %s: select_x is not set\n", confFile);
		return -1;
	}	
		if ( ! config_lookup_int(&conf, "mapping_mode", &mapping_mode))   {
		printf("... %s: mapping_mode is not set\n", confFile);
		return -1;
	}		
		if  (! config_lookup_int(&conf, "debug", &debug) )  {
		printf("... %s: debug is not set\n", confFile);
		printf("... default value set\n");
	}



		_SNESnet_master=0;	
		snprintf(_SNESnet_server, 128, loginserver); 
		snprintf(_SNESnet_username, 128, username); 
		snprintf(_SNESnet_password, 128, password); 
		snprintf(_SNESnet_key, 128, key); 


   // struct sockaddr_in snesnetServAddr; /* sockaddr_in for snesnet server address */
  //  unsigned int messageStringLen;      /* Length of string to echo */
    int totalBytesRcvd, bytesRcvd;      // (Total) Amount of data received from the server
    char messageBuffer[RCVBUFSIZE];     /* Buffer for message string */
    char sendMessage[MAX_MSG_SIZE];     // Buffer for outgoing messages
    pthread_t     t_fifo_inc;
    pthread_t     t_server_inc;
    
    
    printf("%s\n","Initialize _SNESnet...");
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
      printf("Could notcreate TCP/IP Socket!");
      return FALSE;
    }
    
    memset(&snesnetServAddr,0,sizeof(struct sockaddr_in));
    snesnetServAddr.sin_family      = AF_INET;             /* Internet address family */
    snesnetServAddr.sin_addr.s_addr = inet_addr(_SNESnet_server);   /* Server IP address */
    snesnetServAddr.sin_port        = htons(_SNESnet_server_port); /* Server port */

	if(debug){
	fprintf(stdout, "-> SNESnet key [%s]\n", _SNESnet_key);
	fprintf(stdout, "-> SNESnet username [%s]\n", _SNESnet_username);
	fprintf(stdout, "-> SNESnet password [%s]\n", _SNESnet_password);
	fprintf(stdout, "-> SNESnet server [%s]\n", _SNESnet_server);		
	fprintf(stdout, "-> SNESnet server_port [%d]\n", _SNESnet_server_port);	
	}

    if (connect(sock, (struct sockaddr *) &snesnetServAddr, sizeof(snesnetServAddr)) < 0)
    {
        printf("Connectection to SNESnet server failed\n");
	return FALSE;
    }

    printf("Connection to _SNESnet server established\n");


    totalBytesRcvd = 0;
    
    //stage two USER LOGIN
    snprintf(sendMessage,MAX_MSG_SIZE,"U%s\n",_SNESnet_username);

    printf("Try to login as %s\n",sendMessage);

    if (send(sock, sendMessage, strlen(sendMessage), 0) != strlen(sendMessage))
    {
        printf("Could not send user request to the server...\n");
	return FALSE;
    }

    totalBytesRcvd = 0;
    do 
    {
        if ((bytesRcvd = recv(sock, messageBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        {
          printf("recv() failed or connection closed prematurely");
	  return FALSE;
        }

        totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
        messageBuffer[bytesRcvd] = '\0';  /* Terminate the string! */
    }while (messageBuffer[totalBytesRcvd-1] != '\n');
    
    if (messageBuffer[0]!='I' &&  messageBuffer[1]!='V' &&  messageBuffer[2]!=' ')
    {
      printf("IV request failed");
      return FALSE;
    }

    // Now comes the trivium part

    //password
    char passwd[MAX_PASSWORD_LENGTH];
    
    //passwd="pass1";
    snprintf(passwd,MAX_PASSWORD_LENGTH,_SNESnet_password);

    trivium_ctx_t ctx;

    trivium_init(_SNESnet_key, 80, messageBuffer+3, 80, &ctx);

    trivium_enc(&ctx);

    int g=0;
    unsigned char passwd_[strlen(passwd)];
    for(g=0; g<strlen(passwd);g++)
    passwd_[g] = passwd[g] ^ trivium_getbyte(&ctx);


    char *raw_crypt = (char*) malloc(strlen(passwd)*2);
    memset(raw_crypt,0,strlen(passwd)*2);
    uint8_t *m = passwd_;
    int c = sizeof(passwd_);
    while(c--)
    {
        char chr[2];

        snprintf(chr, 3,"%02x", *(m++));
        strcat(raw_crypt, chr);
    }



    snprintf(sendMessage,MAX_MSG_SIZE,"P%s\n",raw_crypt);

    if (send(sock, sendMessage, strlen(sendMessage), 0) != strlen(sendMessage))
    {
        printf("send() sent a different number of bytes than expected\n");
	return FALSE;
    }

    totalBytesRcvd = 0;
    do
    {
        /* Receive up to the buffer size (minus 1 to leave space for
           a null terminator) bytes from the sender */
        if ((bytesRcvd = recv(sock, messageBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        {
           printf("recv() failed or connection closed prematurely\n");
           return FALSE;
        }

        totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
        messageBuffer[bytesRcvd] = '\0';  /* Terminate the string! */
        break;
    }while (messageBuffer[totalBytesRcvd-1] != '\n');

    // End of trivium part

    if(strcmp("OK\n",messageBuffer) == 0)
    {
        printf("Logged in succesfully\n");
    }
    else
    {
        printf("Username or password wrong!\n");
	return FALSE;
    }



		int rc;
		rc = pthread_create( &t_fifo_inc, NULL, &readout_fifo, NULL );
		if ( rc != 0 ) {
			printf("Couldn't create t_fifo_inc.\n");
		}
		
		
		rc = pthread_create( &t_server_inc, NULL, &recv_server_data, NULL );
		if ( rc != 0 ) {
			printf("Couldn't create t_server_inc.\n");
		}
		
		ready=1;
		

	  
	  
  //  close(sock);
    return 0;
}

uint8_t trivium_enc(trivium_ctx_t* ctx){
	uint8_t t1,t2,t3,z;

	t1 = G(65)  ^ G(92);
	t2 = G(161) ^ G(176);
	t3 = G(242) ^ G(287);
	z  = t1^t2^t3;
	t1 ^= (G(90)  & G(91))  ^ G(170);
	t2 ^= (G(174) & G(175)) ^ G(263);
	t3 ^= (G(285) & G(286)) ^ G(68);

	/* shift whole state and insert ts later */
	uint8_t i,c1=0,c2;
	for(i=0; i<36; ++i){
		c2=(((*ctx)[i])>>7);
		(*ctx)[i] = (((*ctx)[i])<<1)|c1;
		c1=c2;
	}
	/* insert ts */
	S(0, t3);
	S(93, t1);
	S(177, t2);

	return z?0x080:0x00;
}

uint8_t trivium_getbyte(trivium_ctx_t *ctx){
	uint8_t r=0, i=0;
	do{
		r>>=1;
		r |= trivium_enc(ctx);
	}while(++i<8);
	return r;
}

#define KEYSIZE_B ((keysize_b+7)/8)
#define IVSIZE_B  ((ivsize_b +7)/8)

//static const uint8_t rev_table[16] PROGMEM = {
	static const uint8_t rev_table[16]  = {
	0x00, 0x08, 0x04, 0x0C,   /* 0000 1000 0100 1100 */
	0x02, 0x0A, 0x06, 0x0E,   /* 0010 1010 0110 1110 */
	0x01, 0x09, 0x05, 0x0D,   /* 0001 1001 0101 1101 */
	0x03, 0x0B, 0x07, 0x0F    /* 0011 1011 0111 1111 */
};

void trivium_init(const void* key, uint16_t keysize_b,
                  const void* iv,  uint16_t ivsize_b,
                  trivium_ctx_t* ctx){
	uint16_t i;
	uint8_t c1,c2;
	uint8_t t1,t2;
	memset((*ctx)+KEYSIZE_B, 0, 35-KEYSIZE_B);
	c2=0;
	c1=KEYSIZE_B;
	do{
		t1 = ((uint8_t*)key)[--c1];
		t2 = (rev_table[t1&0x0f] << 4)|(rev_table[t1>>4]);
		(*ctx)[c2++] = t2;
	}while(c1!=0);

	c2=12;
	c1=IVSIZE_B;
	do{
		t1 = ((uint8_t*)iv)[--c1];
		t2 = (rev_table[t1&0x0f]<<4)|(rev_table[t1>>4]);
		(*ctx)[c2++] = t2;
	}while(c1!=0);

	for(i=12+IVSIZE_B; i>10; --i){
		c2=(((*ctx)[i])<<5);
		(*ctx)[i] = (((*ctx)[i])>>3)|c1;
		c1=c2;
	}

	(*ctx)[35] = 0xE0;

	for(i=0; i<4*288; ++i){
		trivium_enc(ctx);
	}
}


