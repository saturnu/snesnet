#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define SET_BIT(var,pos) ((var) |= 1 << pos)
#define CLEAR_BIT(var,pos) ((var) &= ~(1 << pos))
#define TOGGLE_BIT(var,pos) ((var) ^= 1 << pos)

#define REG_WRIO (*(vuint8*) 0x4201)
#define REG_RDIO (*(vuint8*) 0x4213)

void __sendBit(unsigned char type);
void sendByte(unsigned char byte);
unsigned short recvByte(void);
