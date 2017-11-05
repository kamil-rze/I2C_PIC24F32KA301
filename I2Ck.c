/*
 * File:   I2Ck.c
 * Author: kr120r
 *
 * Created on 10 July 2017, 08:40
 */

//#define FCY 8000000UL

#include <xc.h>
#include "I2Ck.h"
//#include <libpic30.h>

#define SLAVE_ADDRESS   0x50

/*
To accomplish this message, the user software will sequence through the following steps:
1. Assert a Start condition on SDAx and SCLx. "I2C1_Start"
2. Send the I2C device address byte to the slave with a write indication.
3. Wait for and verify an Acknowledge from the slave.
4. Send the serial memory address high byte to the slave.
5. Wait for and verify an Acknowledge from the slave.
6. Send the serial memory address low byte to the slave.
7. Wait for and verify an Acknowledge from the slave.
8. Assert a Repeated Start condition on SDAx and SCLx.
9. Send the device address byte to the slave with a read indication.
10. Wait for and verify an Acknowledge from the slave.
11. Enable the master reception to receive serial memory data.
12. Generate an ACK or NACK condition at the end of a received byte of data.
13. Generate a Stop condition on SDAx and SCLx.
 */

// Enable I2C Module
void I2C1_Enable(){
    I2C1CONbits.I2CEN = 1;
}

// Disable I2C Module
void I2C1_Disable(){
    I2C1CONbits.I2CEN = 0;
}

// Check if bus is idle
void I2C1_Idle(){
    // Wait till bus is idle P = 1
    while((I2C1CON & 0x001F) || I2C1STATbits.TRSTAT);
}

// Start Event
void I2C1_Start(){
    // Generate Start Event
    I2C1CONbits.SEN = 1;
    // wait here till SEN = 0 <- Start event competed
    while(I2C1CONbits.SEN);
}

// Stop Event
void I2C1_Stop(){
    // Generate Start Event
    I2C1CONbits.PEN = 1;
    // wait here till SEN = 0 <- Start event competed
    while(I2C1CONbits.PEN);
    //__delay_us(150);
}

// Restart Event
void I2C1_Restart(){
    // Generate Start Event
    I2C1CONbits.RSEN = 1;
    // wait here till SEN = 0 <- Start event competed
    while(I2C1CONbits.RSEN);
}

// Acknowledge Insert
void I2C1_Ack(){
    // Generate Start Event
    I2C1CONbits.ACKDT = 0;
    I2C1CONbits.ACKEN = 1;
    while(I2C1CONbits.ACKEN);
}

// NotAcknowledge Insert
void I2C1_NAck(){
    // Generate Start Event
    I2C1CONbits.ACKDT = 1;
    I2C1CONbits.ACKEN = 1;
    while(I2C1CONbits.ACKEN);
}



// Send Data
void I2C1_Send(unsigned char data){
    // To send byte out data needs to be copied to Transmit register
    IFS1bits.MI2C1IF = 0;
    I2C1TRN = data;
    while(I2C1STATbits.TBF);
    while(IFS1bits.MI2C1IF);    //Wait for ninth clock cycle
    IFS1bits.MI2C1IF = 0;        //Clear interrupt flag
    while(I2C1STATbits.ACKSTAT);
}

// Receive Data
unsigned char I2C1_Receive(){
    char result;
    I2C1CONbits.RCEN = 1;
    while(I2C1CONbits.RCEN);
    return result = I2C1RCV;
}

// Write Complete check
void I2C1_WriteCmpt(unsigned char sAddress){
    LATB = 0x00;
    while(1){
        I2C1_Idle();
        I2C1_Start();
        
        // To send byte out data needs to be copied to Transmit register
        IFS1bits.MI2C1IF = 0;
        I2C1TRN = sAddress << 1;
        while(I2C1STATbits.TBF);
        while(IFS1bits.MI2C1IF);    //Wait for ninth clock cycle
        IFS1bits.MI2C1IF = 0;        //Clear interrupt flag
        while(I2C1STATbits.ACKSTAT){
            LATB++;
            if(LATB == 0x70){
                LATB = 0x00;
            }
            break;
        }
        I2C1_Idle();
        I2C1_Stop();
        if(I2C1STATbits.ACKSTAT == 0){
            I2C1_Idle();
            I2C1_Stop();
            break;
        }    
    }
}

// Write Command with write-check 16-bit addressing
unsigned char I2C1_iWrite(unsigned char sAddress, unsigned int rAddress, 
                          unsigned char rData){
    unsigned char result;
//    unsigned char i;
    
    I2C1_Enable();
    I2C1_Idle();
    I2C1_Start();
    I2C1_Send(sAddress << 1);
    I2C1_Idle();
    I2C1_Send(rAddress >> 8);
    I2C1_Idle();
    I2C1_Send(rAddress & 0xFF);
    I2C1_Idle();
    I2C1_Send(rData);
    I2C1_Idle();
    I2C1_Stop();
    
    I2C1_WriteCmpt(SLAVE_ADDRESS);
    
    
    I2C1_Idle();
    I2C1_Start();
    I2C1_Send(sAddress << 1);
    I2C1_Idle();
    I2C1_Send(rAddress >> 8);
    I2C1_Idle();
    I2C1_Send(rAddress & 0xFF);
    I2C1_Idle();
    I2C1_Restart();
    I2C1_Send((sAddress << 1) + 1);
    I2C1_Idle();
    result = I2C1_Receive();
    I2C1_Idle();
    I2C1_NAck();
    I2C1_Stop();
    
    return result;
}

// Write Command with write-check 8-bit addressing
unsigned char I2C1_iWrite8(unsigned char sAddress, unsigned char rAddress, 
                          unsigned char rData){
    unsigned char result;
    
    I2C1_Enable();
    I2C1_Idle();
    I2C1_Start();
    I2C1_Send(sAddress << 1);
    I2C1_Idle();
    I2C1_Send(rAddress);
    I2C1_Idle();
    I2C1_Send(rData);
    I2C1_Idle();
    I2C1_Stop();
    
    //I2C1_WriteCmpt(SLAVE_ADDRESS);
    //for(i = 0; i < 10; i++);
    
    I2C1_Idle();
    I2C1_Start();
    I2C1_Send(sAddress << 1);
    I2C1_Idle();
    I2C1_Send(rAddress);
    I2C1_Idle();
    I2C1_Restart();
    I2C1_Send((sAddress << 1) + 1);
    I2C1_Idle();
    result = I2C1_Receive();
    I2C1_Idle();
    I2C1_NAck();
    I2C1_Stop();
    
    return result;
}


unsigned char I2C1_iRead8(unsigned char sAddress, unsigned char rAddress)
{
    char result;
    
    I2C1_Idle();
    I2C1_Start();
    I2C1_Send(sAddress << 1);
    I2C1_Idle();
    I2C1_Send(rAddress);
    I2C1_Idle();
    I2C1_Restart();
    I2C1_Send((sAddress << 1) + 1);
    I2C1_Idle();
    result = I2C1_Receive();
    I2C1_Idle();
    I2C1_NAck();
    I2C1_Stop();
    
    return result;
}

/**
 End of File
*/
