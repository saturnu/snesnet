#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "trivium.h"

#define PASSWORD_MAX   30
#define PASSWORD_MIN   10

int main(int argc, char *argv[]){
	

//printf("%s\n\n",argv[1]);
//1 = key
//2 = iv
//3 = passwd

    // Now comes the trivium part
    //char key[] = "0AnL]d`50m";
    
int real_length_password=0;
int real_length_key=0;
int real_length_iv=0;
int h=0;

    //hex pw to string
	char passwd_hex[128]; //<- argv 3
	
    //password
    char passwd[PASSWORD_MAX]; //-> pw string
    
	if(strlen(argv[3]) >= 5 + PASSWORD_MIN * 2){ // min 10chr passwd

		if(strlen(argv[3]) <= 5 + PASSWORD_MAX * 2){ //max pw len

				//crypt passwd from client
				snprintf ( passwd_hex, 128, "%s", argv[3]); //PASS XXXX [5]
				real_length_password=strlen(passwd_hex)/2;
				
				//char
				 *passwd = (char*) malloc(strlen(passwd_hex)*2);


				h=0;
				for(h=0;h<strlen(passwd_hex);h+=2){
					char buf[5] = {'0', 'x', passwd_hex[h], passwd_hex[h+1], 0};
					passwd[h/2] = strtol(buf, NULL, 0);
				}

		}
	}
    
    
    
    char key_hex[20]; //<- argv 1
    char key_str[10];
    
				//crypt key_str from client
				snprintf ( key_hex, 128, "%s", argv[1]); //PASS XXXX [5]
				real_length_key=strlen(key_hex)/2;

				//char 
				*key_str = (char*) malloc(strlen(key_hex)*2);


				h=0;
				for(h=0;h<strlen(key_hex);h+=2){
					char buf[5] = {'0', 'x', key_hex[h], key_hex[h+1], 0};
					key_str[h/2] = strtol(buf, NULL, 0);
				}

    
    char iv_hex[20]; //<- argv 1
    char iv_str[10];
    
				//crypt iv_str from client
				snprintf ( iv_hex, 128, "%s", argv[2]); //PASS XXXX [5]
				real_length_iv=strlen(iv_hex)/2;

				//char 
				*iv_str = (char*) malloc(strlen(iv_hex)*2);


				h=0;
				for(h=0;h<strlen(iv_hex);h+=2){
					char buf[5] = {'0', 'x', iv_hex[h], iv_hex[h+1], 0};
					iv_str[h/2] = strtol(buf, NULL, 0);
				}
 
    

    trivium_ctx_t ctx;

    trivium_init(key_str, 80, iv_str, 80, &ctx);

    trivium_enc(&ctx);

    int g=0;
    unsigned char passwd_[real_length_password];
    for(g=0; g<real_length_password;g++)
    passwd_[g] = passwd[g] ^ trivium_getbyte(&ctx);

    //strlen of passwd_ might be wrong 'cause of string termination
    //dump(passwd_,strlen(passwd));


    char *raw_crypt = (char*) malloc(real_length_password*2);
    memset(raw_crypt,0,real_length_password*2);
    uint8_t *m = passwd_;
    int c = sizeof(passwd_);
    while(c--)
    {
        char chr[2];

        snprintf(chr, 3,"%02x", *(m++));
        strcat(raw_crypt, chr);
    }


    //just a reverse debug testrun
    //////////////
/*
    trivium_ctx_t ctxb;

    trivium_init(key_str, 80, iv_str, 80, &ctx);
    trivium_enc(&ctxb);

    g=0;
    char passwd_p[strlen(passwd)];
    for(g=0; g<strlen(passwd);g++)
    passwd_p[g] = passwd_[g] ^ trivium_getbyte(&ctxb);
*/
    //////////////
    /////////////


    printf("PASS %s",raw_crypt);
	
	
	
	return 0;
}
