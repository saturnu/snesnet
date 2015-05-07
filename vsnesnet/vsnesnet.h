#include <libconfig.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 


#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define SET_BIT(var,pos) ((var) |= 1 << pos)
#define CLEAR_BIT(var,pos) ((var) &= ~(1 << pos))
#define TOGGLE_BIT(var,pos) ((var) ^= 1 << pos)


#define MAX_USERNAME_SIZE 33
#define KEY_LENGTH 11
#define IV_LENGTH 11
#define MAX_PASSWORD_LENGTH 256
#define MAX_MSG_SIZE 1024
#define FIFO_FILE "/tmp/snes_wrio.fifo"

#define TRUE 1
#define FALSE 0

int init_login(char *confFile);
void stop_snesnet();
int process_incoming(uint8_t a, uint8_t b);
const char *byte_to_binary(int x);

typedef uint8_t trivium_ctx_t[36]; /* 288bit */
extern int sock; /* Socket descriptor */

uint8_t trivium_enc(trivium_ctx_t* ctx);
uint8_t trivium_getbyte(trivium_ctx_t* ctx);
void trivium_init(const void* key, uint16_t keysize_b, const void* iv,  uint16_t ivsize_b, trivium_ctx_t* ctx);


int	_SNESnet_master;		
int	_SNESnet_server_port;

char	_SNESnet_server[128];	
char	_SNESnet_username[128];	
char	_SNESnet_password[128];
char	_SNESnet_key[128];

char	_SNESnet_master_addr[128];				
int	_SNESnet_master_port;
int	_SNESnet_socket_tcp;

struct  sockaddr_in _SNESnet_server_addr;
struct  sockaddr_in _SNESnet_client_addr;

const char *js_dev;
int debug;
int ready;
int fdx;

//cheap controller mapping
int js_axis_type;
int js_button_type;

int js_axis_up_val;
int js_axis_down_val;
int js_axis_left_val;
int js_axis_right_val;

int js_axis_y_nr;
int js_axis_x_nr;

int js_button_a_nr;
int js_button_b_nr;
int js_button_x_nr;
int js_button_y_nr;
int js_button_l_nr;
int js_button_r_nr;
int js_button_st_nr;
int js_button_se_nr;

int disable_p1;
int select_x;

int mapping_mode;

