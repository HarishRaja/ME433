#include<xc.h> // processor SFR definitions
#include<sys/attribs.h> // __ISR macro

// DEVCFG0
#pragma config DEBUG = 0 // no debugging
#pragma config JTAGEN = 0 // no jtag
#pragma config ICESEL = 0b11 // use PGED1 and PGEC1
#pragma config PWP = 0b111111111 // no write protect
#pragma config BWP = 1 // not boot write protect
#pragma config CP = 1 // no code protect

// DEVCFG1
#pragma config FNOSC = 0b011 // use primary oscillator with pll
#pragma config FSOSCEN = 0 // turn off secondary oscillator
#pragma config IESO = 0 // no switching clocks
#pragma config POSCMOD = 0b10 // high speed crystal mode
#pragma config OSCIOFNC = 1 // free up secondary osc pins
#pragma config FPBDIV = 0b00 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = 0b11 // do not enable clock switch
#pragma config WDTPS = 0b00000 // slowest wdt
#pragma config WINDIS = 1 // no wdt window
#pragma config FWDTEN = 0 // wdt off by default
#pragma config FWDTWINSZ = 0b11 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 40MHz
#pragma config FPLLIDIV = 0b001 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = 0b101 // multiply clock after FPLLIDIV
#pragma config UPLLIDIV = 0b001 // divide clock after FPLLMUL
#pragma config UPLLEN = ON // USB clock on
#pragma config FPLLODIV = DIV_2 // divide clock by 1 to output on pin

// DEVCFG3
#pragma config USERID = 0xFFFF // some 16bit userid
#pragma config PMDL1WAY = 1 // not multiple reconfiguration, check this
#pragma config IOL1WAY = 1// not multimple reconfiguration, check this
#pragma config FUSBIDIO = 1 // USB pins controlled by USB module
#pragma config FVBUSONIO = 1 // controlled by USB module

int readADC(void);

int main() {

    // startup
    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that
    // kseg0 is cacheable (0x3) or uncacheable (0x2)
    // see Chapter 2 "CPU for Devices with M4K Core"
    // of the PIC32 reference manual
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // no cache on this chip!

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to be able to use TDI, TDO, TCK, TMS as digital
    DDPCONbits.JTAGEN = 0;

    __builtin_enable_interrupts();

    // set up USER pin as input
    ANSELBbits.ANSB13 = 0;
    ANSELBbits.ANSB15 = 0;
    TRISBbits.TRISB13=1;

    // set up LED1 pin as a digital output
    TRISBbits.TRISB7 = 0;
    int light = 1;
    LATBbits.LATB7 = light;



    RPB15Rbits.RPB15R  = 0b0101;

    // set up LED2 as OC1 using Timer2 at 1kHz
    
    T2CONbits.TCKPS = 0;
    T2CONbits.TCS = 0;
    PR2 = 9999;
    TMR2 = 0;

    OC1CONbits.OCTSEL = 0;
    OC1CONbits.OCM = 0b110;
    OC1RS = 5000;
    OC1R = 5000;
    
    T2CONbits.ON  = 1;
    OC1CONbits.ON = 1;

    // set up A0 as AN0
    ANSELAbits.ANSA0 = 1;
    AD1CON3bits.ADCS = 3;
    AD1CHSbits.CH0SA = 0;
    AD1CON1bits.ADON = 1;

    _CP0_SET_COUNT(0);


    while (1) {
        // invert pin every 0.5s, set PWM duty cycle % to the pot voltage output %
        int pot = readADC();
        OC1RS = pot*35;

        if(PORTBbits.RB13 == 0)
        {
            LATBbits.LATB7 = !LATBbits.LATB7;
        }
        
        if(_CP0_GET_COUNT()>=1000000)
        {
            light = !light;
            LATBbits.LATB7 = light;
            _CP0_SET_COUNT(0);
        }
    }
}

int readADC(void) {
    int elapsed = 0;
    int finishtime = 0;
    int sampletime = 20;
    int a = 0;

    AD1CON1bits.SAMP = 1;
    elapsed = _CP0_GET_COUNT();
    finishtime = elapsed + sampletime;
    while (_CP0_GET_COUNT() < finishtime) {
    }
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE) {
    }
    a = ADC1BUF0;
    return a;
}