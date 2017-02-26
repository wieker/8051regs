#define ALLOCATE_EXTERN
#include "fx2regs.h"

#define	NOP		__asm nop __endasm
#define	SYNCDELAY	NOP; NOP; NOP; NOP
#define SYNCDELAY3 {SYNCDELAY;SYNCDELAY;SYNCDELAY;}

static void initIOASOut(void)
{
    OED = 0xAA;
    IOD = 0x00;
    OEA = 0x03;
    IOA = 0x01;
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

    IOA=0x01;
}

void initSlaveFIFO() {
    EP6FIFOCFG = 0x08;  SYNCDELAY;

    FIFOPINPOLAR=0x1f;  SYNCDELAY;
    PORTACFG = 0x00;  SYNCDELAY;

    IFCONFIG = 0x03;  SYNCDELAY;
    REVCTL = 0x03;    SYNCDELAY;

    IOA=0x02;
}

void initEP2AsInput(int cpuProcessing) {
    EP2CFG = 0xa2;  SYNCDELAY;
    EP2FIFOCFG = 0x00 | (!cpuProcessing << 4);  SYNCDELAY;
}

void initEP6AsOutput(int cpuProcessing) {
    EP6CFG = 0xe0;  SYNCDELAY;
    if (cpuProcessing) {
        EP6FIFOCFG = 0x00;  SYNCDELAY;
    } else {
        EP6FIFOCFG = 0x08;  SYNCDELAY;
    }

    EP6AUTOINLENH = 0x02; SYNCDELAY;
    EP6AUTOINLENL = 0x00; SYNCDELAY;
}

void discardInAsCPUProcessed() {
    SYNCDELAY;  OUTPKTEND=0x82;
}

void finishCPUOutput(int discardOrSendToPC) {
    SYNCDELAY;  INPKTEND=0x06 | (discardOrSendToPC << 7);
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
    return len;
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

void stop_sampling()
{
    GPIFABORT = 0xff;
    SYNCDELAY3;
    INPKTEND = 6;
}

void start_sampling()
{
    int i;
    clearFIFO();

    for (i = 0; i < 1000; i++);
    while (!(GPIFTRIG & 0x80)) {
    ;
    }
    SYNCDELAY3;
    GPIFTCB1 = 0x28;
    SYNCDELAY3;
    GPIFTCB0 = 0;
    GPIFTRIG = 6;
}

const struct samplerate_info {
    BYTE rate;
    BYTE wait0;
    BYTE wait1;
    BYTE opc0;
    BYTE opc1;
    BYTE out0;
    BYTE ifcfg;
} samplerates[] = {
    { 48,0x80,   0, 3, 0, 0x00, 0xea },
    { 30,0x80,   0, 3, 0, 0x00, 0xaa },
    { 24,   1,   0, 2, 1, 0x10, 0xca },
    { 16,   1,   1, 2, 0, 0x10, 0xca },
    { 12,   2,   1, 2, 0, 0x10, 0xca },
    {  8,   3,   2, 2, 0, 0x10, 0xca },
    {  4,   6,   5, 2, 0, 0x10, 0xca },
    {  2,  12,  11, 2, 0, 0x10, 0xca },
    {  1,  24,  23, 2, 0, 0x10, 0xca },
    { 50,  48,  47, 2, 0, 0x10, 0xca },
    { 20, 120, 119, 2, 0, 0x10, 0xca },
    { 10, 240, 239, 2, 0, 0x10, 0xca }
};

BYTE set_samplerate(BYTE rate)
{
    BYTE i = 0;
    while (samplerates[i].rate != rate) {
    i++;
    if (i == sizeof(samplerates)/sizeof(samplerates[0]))
        return 0;
    }

    IFCONFIG = samplerates[i].ifcfg;

    AUTOPTRSETUP = 7;
    AUTOPTRH2 = 0xE4;
    AUTOPTRL2 = 0x00;

    /* The program for low-speed, e.g. 1 MHz, is
     * wait 24, CTL2=0, FIFO
     * wait 23, CTL2=1
     * jump 0, CTL2=1
     *
     * The program for 24 MHz is
     * wait 1, CTL2=0, FIFO
     * jump 0, CTL2=1
     *
     * The program for 30/48 MHz is:
     * jump 0, CTL2=Z, FIFO, LOOP
     */

    EXTAUTODAT2 = samplerates[i].wait0;
    EXTAUTODAT2 = samplerates[i].wait1;
    EXTAUTODAT2 = 1;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;

    EXTAUTODAT2 = samplerates[i].opc0;
    EXTAUTODAT2 = samplerates[i].opc1;
    EXTAUTODAT2 = 1;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;

    EXTAUTODAT2 = samplerates[i].out0;
    EXTAUTODAT2 = 0x11;
    EXTAUTODAT2 = 0x11;
    EXTAUTODAT2 = 0x00;
    EXTAUTODAT2 = 0x00;
    EXTAUTODAT2 = 0x00;
    EXTAUTODAT2 = 0x00;
    EXTAUTODAT2 = 0x00;

    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;
    EXTAUTODAT2 = 0;

    for (i = 0; i < 96; i++)
    EXTAUTODAT2 = 0;
    return 1;
}

void initGPIFMode() {
    int i = 0, gg, p, q;
    // in idle mode tristate all outputs
    GPIFIDLECTL = 0x00;
    GPIFCTLCFG = 0x80;
    GPIFWFSELECT = 0x00;
    GPIFREADYSTAT = 0x00;
    EP6GPIFFLGSEL = 0x01;

    EP6FIFOCFG = 0x08;  SYNCDELAY;

    IFCONFIG = 0xc2;  SYNCDELAY;
    REVCTL = 0x03;    SYNCDELAY;

    IOA=0x03;

    stop_sampling();
    set_samplerate(24);
    start_sampling();
    IOD = 0xA0;

    for (gg = 0; gg < 1000; gg ++) {
        for (p = 0; p < 1200; p ++)
            for (q = 0; q < 10; q ++);
    }

    IOD = 0x00;
    stop_sampling();
    for (i = 0; i < 4; i ++) {
        finishCPUOutput(0);
    }
    initDefaultPortSetup();
    initEP6AsOutput(1);
    IOA=0x01;
}

void process(char* command, int size) {
    unsigned char i = 0;
    int gg;
    int p, q, l;
    switch (*command) {
    case 'P':
        return;
    case 'D':
        for (gg = 0; gg < 1000; gg ++) {
            IOD = i;
            for (p = 0; p < 120; p ++)
                        ;
            i += 64;
        }
        initDefaultPortSetup();
        initEP6AsOutput(1);
        return;
    case 'S':
        initSlaveFIFO();
        IOD = 0xA0;
        for (gg = 0; gg < 1; gg++) {
            while (!(IOD & 0x01)) {
                IOD = 0xA0;
            }
            while (IOD & 0x01) {
                IOD = 0xA2;
            }
        }
        IOD = 0x00;
        initDefaultPortSetup();
        initEP6AsOutput(1);
        return;
    case 'G':
        initGPIFMode();
        return;
    }
}

void main()
{
    char inbuf[10];
    int i = 0, j = 0, c = 0, k = 0;
    char outbuf[10];

    initDefaultPortSetup();
    initEP2AsInput(1);
    initEP6AsOutput(1);
    initIOASOut();

    for(;;) {
        if (isInputNotEmpty()) {
            int len = receivePacket(inbuf, 10);
            outbuf[0] = 'S';
            outbuf[1] = len & 0xff;
            outbuf[2] = inbuf[0];
            outbuf[3] = inbuf[1];
            while (!isOutputNotFull()) { }
            sendPacket(outbuf, 4);
            k ++;
            process(inbuf, len);
        }

        i ++;
        if (i == 160) {
            i = 0;
            j ++;
        }
        if (j == 1000) {
            outbuf[0] = 'S';
            outbuf[1] = EP2CS;
            outbuf[2] = c & 0xff;
            outbuf[3] = k & 0xff;
            while (!isOutputNotFull()) { }
            sendPacket(outbuf, 4);
            c ++;
            j = 0;
        }
    }
}
