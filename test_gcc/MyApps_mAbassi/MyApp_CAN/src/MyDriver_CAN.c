/*-----------------------------------------------------------*/

#include "MyApp_CAN.h"
#include "MyDriver_CAN.h"

#include "mAbassi.h"
#include "Platform.h"
#include "HWinfo.h" 
#include "dw_spi.h"

/* ------------------------------------------------------------------------------------------------ */

void    CAN_writeCommand(uint8_t theCommand)
{
    assert(spi_send(CAN_SPI_DEV, CAN_SPI_SLV, &theCommand, 1) == 0);
}

/* ------------------------------------------------------------------------------------------------ */

uint8_t CAN_readCommand(uint8_t theCommand)
{
    uint8_t  BufTX[3], BufRX[3];
    BufTX[0] = theCommand;
    BufTX[1] = 0x0;
    BufTX[2] = 0x0;
    assert(spi_send_recv(CAN_SPI_DEV, CAN_SPI_SLV, BufTX, 3, BufRX, 3) == 0);
    return(BufRX[1]);
}

/* ------------------------------------------------------------------------------------------------ */

void    CAN_writeReg(uint8_t theAddress, uint8_t theData)
{
    uint8_t  BufTX[3];
    BufTX[0] = MCP2515_CMD_WRITE;
    BufTX[1] = theAddress;
    BufTX[2] = theData;
    assert(spi_send(CAN_SPI_DEV, CAN_SPI_SLV, BufTX, 3) == 0);
}

/* ------------------------------------------------------------------------------------------------ */

uint8_t CAN_readReg(uint8_t theAddress)
{
    uint8_t  BufTX[3], BufRX[3];
    BufTX[0] = MCP2515_CMD_READ;
    BufTX[1] = theAddress;
    BufTX[2] = 0x0;
    assert(spi_send_recv(CAN_SPI_DEV, CAN_SPI_SLV, BufTX, 3, BufRX, 3) == 0);
    return(BufRX[2]);
}

/* ------------------------------------------------------------------------------------------------ */

void    CAN_init(void)
{
    // Init the SPI
    assert(spi_init(CAN_SPI_DEV, CAN_SPI_SLV, 1000000, 8, SPI_PROTO_SPI
                    | SPI_CLK_CPHA1                  // Data is captured (read) on the active to inactive clock transition
                    | SPI_CLK_CPOL1                  // Idle state (inactive) of the clock is 1
                    | SPI_ALIGN_RIGHT                // The frames are right aligned in the buffer
                    | SPI_DATA_NONPACK               // The frames in the buffers are one frame per byte/16 bit words
                    | SPI_TX_RX                      // both transmission and reception are going on at the same time
                    | SPI_MASTER                     // The controller is the bus master
                    | SPI_XFER_POLLING               // No transfers use interrupts, polling is used
                    | SPI_EOT_POLLING                // When transmitting frames, uses polling to detect when all frames have been transferred
                    ) == 0);
    
    // Software Reset
    CAN_writeCommand(MCP2515_CMD_RESET);
    
    // Set Configuration Mode
    CAN_writeReg(MCP2515_CANCTRL, 0x80);
    
    /*
    Bit Timing Configuration
    125 kHz CAN baud rate with Fosc = 16 MHz
    
    Fosc        = 16MHz
    BRP         =   7  (divide by 8)
    Sync Seg    = 1TQ
    Prop Seg    = 1TQ
    Phase Seg1  = 3TQ
    Phase Seg2  = 3TQ
    
    TQ = 2 * (1/Fosc) * (BRP+1) = 1us
    CAN Bit Time = (1+1+3+3) 1 us = 8 us -> 125kHz
    Bus speed = 1/(Total # of TQ) * TQ
    
    The CAN Configuration Registers are then assembled as follows:
    CNF1 = SJW1 + (BRP-1) = 0x07
    CNF2 = (BTLMODE_SET + (PHSEG1-1)*8 + (PRSEG-1)) = 0x90
    CNF3 = (SOF_DISABLE + WAKFIL_DISABLE + (PHSEG2-1)) = 0x02
    */
    
    CAN_writeReg(MCP2515_CNF1, 0x07);
    CAN_writeReg(MCP2515_CNF2, 0x90);
    CAN_writeReg(MCP2515_CNF3, 0x02);
    
    // Set Normal Operation Mode
    CAN_writeReg(MCP2515_CANCTRL, 0x00);
    
    //Configure initialization of message transmission
    //TX0RTS, TX1RTS, TX2RTS not used : B0RTSM, B1RTSM, B2RTSM = 0
    
    CAN_writeReg(MCP2515_TXRTSCTRL, 0x00);
    
    //Configure Masks and Filters
    
    CAN_writeReg(MCP2515_RXM0SIDH, 0x00);
    CAN_writeReg(MCP2515_RXM0SIDL, 0x00);
    CAN_writeReg(MCP2515_RXM0EID8, 0x00);
    CAN_writeReg(MCP2515_RXM0EID0, 0x00);
    
    CAN_writeReg(MCP2515_RXM1SIDH, 0x00);
    CAN_writeReg(MCP2515_RXM1SIDL, 0x00);
    CAN_writeReg(MCP2515_RXM1EID8, 0x00);
    CAN_writeReg(MCP2515_RXM1EID0, 0x00);
    
    CAN_writeReg(MCP2515_RXF0SIDH, 0x00);
    CAN_writeReg(MCP2515_RXF0SIDL, 0x00);
    CAN_writeReg(MCP2515_RXF0EID8, 0x00);
    CAN_writeReg(MCP2515_RXF0EID0, 0x00);
    
    CAN_writeReg(MCP2515_RXF1SIDH, 0x00);
    CAN_writeReg(MCP2515_RXF1SIDL, 0x00);
    CAN_writeReg(MCP2515_RXF1EID8, 0x00);
    CAN_writeReg(MCP2515_RXF1EID0, 0x00);
    
    CAN_writeReg(MCP2515_RXF2SIDH, 0x00);
    CAN_writeReg(MCP2515_RXF2SIDL, 0x00);
    CAN_writeReg(MCP2515_RXF2EID8, 0x00);
    CAN_writeReg(MCP2515_RXF2EID0, 0x00);
    
    CAN_writeReg(MCP2515_RXF3SIDH, 0x00);
    CAN_writeReg(MCP2515_RXF3SIDL, 0x00);
    CAN_writeReg(MCP2515_RXF3EID8, 0x00);
    CAN_writeReg(MCP2515_RXF3EID0, 0x00);
    
    CAN_writeReg(MCP2515_RXF4SIDH, 0x00);
    CAN_writeReg(MCP2515_RXF4SIDL, 0x00);
    CAN_writeReg(MCP2515_RXF4EID8, 0x00);
    CAN_writeReg(MCP2515_RXF4EID0, 0x00);
    
    CAN_writeReg(MCP2515_RXF5SIDH, 0x00);
    CAN_writeReg(MCP2515_RXF5SIDL, 0x00);
    CAN_writeReg(MCP2515_RXF5EID8, 0x00);
    CAN_writeReg(MCP2515_RXF5EID0, 0x00);
    
    // Set Normal Operation Mode
    CAN_writeReg(MCP2515_CANCTRL, 0x00);
    
    // Configura Receive buffer RXB0 et RXB1
    // RXM = 11 : turn mask/filters off
    // BUKT = 1 : enable Rollover
    
    CAN_writeReg(MCP2515_RXB0CTRL, 0xff);
    CAN_writeReg(MCP2515_RXB1CTRL, 0xff);
    
    // Clear all interrupt flags
    // Disable all interrupts except
    
    CAN_writeReg(MCP2515_CANINTF, 0x00);
    CAN_writeReg(MCP2515_CANINTE, 0x00);
    
}

/* ------------------------------------------------------------------------------------------------ */

void    CAN_debug(void)
{
    printf("CNF1     = %.2x\n", CAN_readReg(MCP2515_CNF1));
    printf("CNF2     = %.2x\n", CAN_readReg(MCP2515_CNF2));
    printf("CNF3     = %.2x\n", CAN_readReg(MCP2515_CNF3));
    printf("CANCTRL  = %.2x\n", CAN_readReg(MCP2515_CANCTRL));
    printf("CANSTAT  = %.2x\n", CAN_readReg(MCP2515_CANSTAT));
    printf("TXB0CTRL = %.2x\n", CAN_readReg(MCP2515_TXB0CTRL));
    printf("TXB1CTRL = %.2x\n", CAN_readReg(MCP2515_TXB1CTRL));
    printf("CANINTE  = %.2x\n", CAN_readReg(MCP2515_CANINTE));
    printf("CANINTF  = %.2x\n", CAN_readReg(MCP2515_CANINTF));
    printf("EFLG     = %.2x\n", CAN_readReg(MCP2515_EFLG));
    printf("TEC      = %.2x\n", CAN_readReg(MCP2515_TEC));
    printf("REC      = %.2x\n", CAN_readReg(MCP2515_REC));
    
    printf("Status   = %.2x\n", CAN_readCommand(MCP2515_CMD_STATUS));
    printf("RxStatus = %.2x\n", CAN_readCommand(MCP2515_CMD_RXSTATUS));
    
    printf("\n");
}

/* ------------------------------------------------------------------------------------------------ */

void    CAN_sendMsg(uint32_t theIdentifier, uint8_t *theData, uint8_t theLength, uint8_t theFrameType)
{
    // Write the 11-bit Identifier (the Extended Mode is not implemented)
    CAN_writeReg(MCP2515_TXB0SIDH, (theIdentifier >> 3));
    CAN_writeReg(MCP2515_TXB0SIDL, (theIdentifier << 5) & 0xe0);    //  The 5 LSB = 0
    
    // Write the Data Length (4 LSB) and the RTR (Remote Transmission Request) bit
    if (theFrameType == MCP2515_TX_STD_FRAME)
        CAN_writeReg(MCP2515_TXB0DLC, (theLength & 0x0f));          //  RTR = 0
    else if (theFrameType == MCP2515_TX_REMOTE_FRAME)
        CAN_writeReg(MCP2515_TXB0DLC, (theLength | 0xf0));          // RTR = 1
    else
        return;                                                     // Illegal FrameType - Do nothing !

    // Write the Data
    int i, x = MCP2515_TXB0D0;
    for (i = 0; i < theLength; i++)
        CAN_writeReg(x++, *theData++);
        
    // Send Message (TXREQ = 1) with highest priority (TXP = 11)
        CAN_writeReg(MCP2515_TXB0CTRL, 0x0f);

    // Wait for end of transmission
    while ((CAN_readReg(MCP2515_TXB0CTRL) & 0x80) != 0x00);         // TXREQ (bit 8) = 0
    
}

/* ------------------------------------------------------------------------------------------------ */

int CAN_readMsg(uint32_t *theIdentifier, uint8_t *theData, uint8_t *theLength)
{
    int i, x;
    uint8_t rx_Status = CAN_readCommand(MCP2515_CMD_RXSTATUS);
    
    if ((rx_Status & 0x40) != 0) {                                // Message in RXB0
        *theIdentifier = (CAN_readReg(MCP2515_RXB0SIDH) << 3) | (CAN_readReg(MCP2515_RXB0SIDL) >> 5);
        *theLength = CAN_readReg(MCP2515_RXB0DLC);
        x = MCP2515_RXB0D0;
        for(i=0; i<*theLength; i++)
            *theData++ = CAN_readReg(x++);
        CAN_writeReg(MCP2515_CANINTF, CAN_readReg(MCP2515_CANINTF) & 0xfe); // Clear RX0IF
        
        return(1);
    }
    if ((rx_Status & 0x80) != 0) {                                // Message in RXB1
        *theIdentifier = (CAN_readReg(MCP2515_RXB1SIDH) << 3) | (CAN_readReg(MCP2515_RXB1SIDL) >> 5);
        *theLength = CAN_readReg(MCP2515_RXB1DLC);
        x = MCP2515_RXB1D0;
        for(i=0; i<*theLength; i++)
            *theData++ = CAN_readReg(x++);
        CAN_writeReg(MCP2515_CANINTF, CAN_readReg(MCP2515_CANINTF) & 0xfd); // Clear RX1IF
        return(1);
    }
    return(0);
}

/* ------------------------------------------------------------------------------------------------ */
