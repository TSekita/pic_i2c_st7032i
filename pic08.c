
// PIC16F18857 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator mode selection bits (Oscillator not enabled)
//#pragma config RSTOSC = HFINT1  // Power-up default value for COSC bits (HFINTOSC (1MHz))
#pragma config RSTOSC = HFINT32 // Power-up default value for COSC bits (HFINTOSC with OSCFRQ= 32 MHz and CDIV = 1:1)
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (FSCM timer disabled)

// CONFIG2
#pragma config MCLRE = ON       // Master Clear Enable bit (MCLR pin is Master Clear function)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config LPBOREN = OFF    // Low-Power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) set to 1.9V on LF, and 2.45V on F Devices)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = OFF     // Peripheral Pin Select one-way control (The PPSLOCK bit can be cleared and set only once in software)
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will not cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled, SWDTEN is ignored)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4
#pragma config WRT = OFF        // UserNVM self-write protection bits (Write protection off)
#pragma config SCANE = not_available// Scanner Enable bit (Scanner module is not available for use)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low Voltage programming enabled. MCLR/Vpp pin function is MCLR.)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (Program Memory code protection disabled)
#pragma config CPD = OFF        // DataNVM code protection bit (Data EEPROM code protection disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 32000000 // Internal oscillator Hz
#define ST7032i_ADDR 0x7C // ST7032i slave address is 0x7C

// I2C initialization

void I2C1_Init(void) {
    // PPS: RC3 = SCL, RC4 = SDA
    RC3PPS = 0x14; // SCL output
    RC4PPS = 0x15; // SDA output
    SSP1CLKPPS = 0x13; // SCL input
    SSP1DATPPS = 0x14; // SDA input

    // Configure I2C pins
    TRISCbits.TRISC3 = 1; // input
    TRISCbits.TRISC4 = 1; // input
    ANSC3 = 0; // digital
    ANSC4 = 0; // digital

    // Configure MSSP module as I2C Master, 100kHz
    SSP1CON1 = 0x28; // I2C master mode
    SSP1CON2 = 0x00;
    SSP1CON3 = 0x00;
    SSP1ADD = (_XTAL_FREQ / (4 * 100000)) - 1;
}

// Send I2C start condition

void I2C1_Start(void) {
    SSP1IF = 0;
    SSP1CON2bits.SEN = 1;
    while (SSP1IF == 0) {
    }
    SSP1IF = 0;
    return;
}

// Send I2C stop condition

void I2C1_Stop(void) {
    SSP1IF = 0;
    SSP1CON2bits.PEN = 1;
    while (SSP1IF == 0) {
    }
    SSP1IF = 0;
    return;
}

// Send one byte over I2C

void I2C1_Write(uint8_t data) {
    SSP1IF = 0;
    SSP1BUF = data;
    while (SSP1IF == 0) {
    }
    SSP1IF = 0;
    return;
}

// Send command to ST7032i

void LCD_Command(uint8_t cmd) {
    I2C1_Start();
    I2C1_Write(ST7032i_ADDR); // 8-bit address
    I2C1_Write(0x00); // Control byte: Co=0, RS=0 (command)
    I2C1_Write(cmd);
    I2C1_Stop();
    __delay_ms(2);
}

// Send data (character) to ST7032i

void LCD_Data(uint8_t data) {
    I2C1_Start();
    I2C1_Write(ST7032i_ADDR);
    I2C1_Write(0x40); // Control byte: Co=0, RS=1 (data)
    I2C1_Write(data);
    I2C1_Stop();
    __delay_us(50);
}

// Initialize ST7032i LCD

void ST7032i_Init(void) {
    __delay_ms(50); // Wait for LCD power-up

    LCD_Command(0x38); // Function set: 8-bit, 2-line, normal instruction
    LCD_Command(0x39); // Function set: extended instruction
    LCD_Command(0x14); // Internal OSC frequency
    LCD_Command(0x70); // Contrast set low byte
    LCD_Command(0x56); // Power/Icon/Contrast high byte
    LCD_Command(0x6C); // Follower control
    __delay_ms(200); // Wait for voltage stable

    LCD_Command(0x38); // Function set: normal instruction
    LCD_Command(0x0C); // Display ON, Cursor OFF, Blink OFF
    LCD_Command(0x01); // Clear display
    __delay_ms(2); // Wait for clear display
}

// Display string on LCD

void LCD_Print(const char *str) {
    while (*str) {
        LCD_Data(*str++);
    }
}

// Main routine

void main(void) {
    I2C1_Init();
    __delay_ms(200);

    ST7032i_Init();
    __delay_ms(200);

    LCD_Command(0x02);
    LCD_Print("Hello,");
    LCD_Command(0xC0); // Move to 2nd line (0x40 address)
    LCD_Print("World!");
    while (1) {
        LCD_Command(0x02);
        LCD_Print("Hello,");
        LCD_Command(0xC0); // Move to 2nd line (0x40 address)
        LCD_Print("World!");
        //        LCD_Command(0x18);
        __delay_ms(50);
    }
}