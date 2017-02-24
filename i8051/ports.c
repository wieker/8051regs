#define ALLOCATE_EXTERN
#include "fx2regs.h"

#define	NOP		__asm nop __endasm
#define	SYNCDELAY	NOP; NOP; NOP; NOP
#define SYNCDELAY3 {SYNCDELAY;SYNCDELAY;SYNCDELAY;}

static void Initialize(void)
{

    OED = 0xff;
    IOD = 0x00;
    OEA = 0xff;
    IOA = 0x00;
    OEB = 0xff;
    IOB = 0x00;


    FIFORESET = 0x80;  SYNCDELAY;
    FIFORESET = 0x82;  SYNCDELAY;
    FIFORESET = 0x84;  SYNCDELAY;
    FIFORESET = 0x86;  SYNCDELAY;
    FIFORESET = 0x88;  SYNCDELAY;
    FIFORESET = 0x00;  SYNCDELAY;
}

void initDefault() {
    CPUCS = 0x12;

    IFCONFIG = 0xc0;  SYNCDELAY;
    REVCTL = 0x03;    SYNCDELAY;

    EP4FIFOCFG = 0x00;  SYNCDELAY;
    EP8FIFOCFG = 0x00;  SYNCDELAY;
}

void initSlaveFIFO() {
    ;
}

void initEP2AsInput(int cpuProcessing) {
    EP2CFG = 0xa0;  SYNCDELAY;
    EP2FIFOCFG = 0x00;  SYNCDELAY;
}

void initEP6AsOutput(int cpuProcessing) {
    EP6CFG = 0xe0;  SYNCDELAY;
    EP6FIFOCFG = 0x00;  SYNCDELAY;
}

void discardInAsCPUProcessed() {
    ;
}

void finishCPUOutput(int discardOrSendToPC) {
    ;
}

void sendToPCPartialPkt(int len) {
    ;
}

void main()
{
    Initialize();

    for(;;) {}
}
