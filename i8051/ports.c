#define ALLOCATE_EXTERN
#include "fx2regs.h"

#define	NOP		__asm nop __endasm
#define	SYNCDELAY	NOP; NOP; NOP; NOP
#define SYNCDELAY3 {SYNCDELAY;SYNCDELAY;SYNCDELAY;}

void discardInAsCPUProcessed();

static void initIOASOut(void)
{

    OED = 0xff;
    IOD = 0x00;
    OEA = 0xff;
    IOA = 0x00;
    OEB = 0xff;
    IOB = 0x00;
}

void clearFIFO() {
    FIFORESET = 0x80;  SYNCDELAY;
    FIFORESET = 0x82;  SYNCDELAY;
    FIFORESET = 0x84;  SYNCDELAY;
    FIFORESET = 0x86;  SYNCDELAY;
    FIFORESET = 0x88;  SYNCDELAY;
    FIFORESET = 0x00;  SYNCDELAY;
}

void initDefaultPortSetup() {
    CPUCS = 0x12;

    IFCONFIG = 0xc0;  SYNCDELAY;
    REVCTL = 0x03;    SYNCDELAY;

    EP4FIFOCFG = 0x00;  SYNCDELAY;
    EP8FIFOCFG = 0x00;  SYNCDELAY;
}

void initSlaveFIFO() {
    FIFOPINPOLAR=0x1f;  SYNCDELAY;
    IFCONFIG = 0x03;  SYNCDELAY;
    REVCTL = 0x03;    SYNCDELAY;
    PORTACFG = 0x00;  SYNCDELAY;
}

void initEP2AsInput(int cpuProcessing) {
    EP2CFG = 0xa0;  SYNCDELAY;
    EP2FIFOCFG = 0x00 | (!cpuProcessing << 4);  SYNCDELAY;
    discardInAsCPUProcessed();
    discardInAsCPUProcessed();
    discardInAsCPUProcessed();
    discardInAsCPUProcessed();
}

void initEP6AsOutput(int cpuProcessing) {
    EP6CFG = 0xe0;  SYNCDELAY;
    EP6FIFOCFG = 0x00 | (!cpuProcessing << 3);  SYNCDELAY;

    EP6AUTOINLENH = 0x02; SYNCDELAY;
    EP6AUTOINLENL = 0x00; SYNCDELAY;
}

void discardInAsCPUProcessed() {
    OUTPKTEND=0x82;   SYNCDELAY;
}

void finishCPUOutput(int discardOrSendToPC) {
    INPKTEND=0x06 | (discardOrSendToPC << 7);   SYNCDELAY;;
}

void sendToPCPartialPkt(int len) {
    EP6BCH=len>>8;    SYNCDELAY;
    EP6BCL=len&0xff;  SYNCDELAY;
}

int isInputNotEmpty() {
    return !(EP2CS & (1<<2));
}

int isOutputNotFull() {
    return !(EP6CS & (1<<3));
}

int receivePacket(unsigned char* dest, unsigned int size) {
    xdata const unsigned char *src=EP2FIFOBUF;
    unsigned int len = ((int)EP2BCH)<<8 | EP2BCL;
    unsigned int i;
    for(i=0; i<len && i < size; i++,src++)
    {
        dest[i] = *src;
    }

    discardInAsCPUProcessed();
}

void sendPacket(unsigned char* src, unsigned int size) {
    xdata unsigned char *dest=EP6FIFOBUF;
    unsigned int i;
    for(i=0; i < size; i++,dest++,src++)
    {
        *dest = *src;
    }
    sendToPCPartialPkt(size);
}

void main()
{
    char inbuf[10];
    int i, j, c = 0, k = 0;
    char outbuf[10];

    initDefaultPortSetup();
    initEP2AsInput(1);
    initEP6AsOutput(1);

    for(;;) {
        if (isInputNotEmpty()) {
            int len = receivePacket(inbuf, 10);
            while (!isOutputNotFull()) { }
            sendPacket(inbuf, 4);
            k ++;
        } else {
            for (j = 0; j < 1000; j ++)
            for (i = 0; i < 16000; i ++) { }
            outbuf[0] = 'S';
            outbuf[1] = EP2CS;
            outbuf[2] = c & 0xff;
            outbuf[3] = k & 0xff;
            sendPacket(outbuf, 4);
            c ++;
        }
    }
}
