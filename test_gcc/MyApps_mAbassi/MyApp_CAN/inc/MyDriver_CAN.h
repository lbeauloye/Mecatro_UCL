#ifndef MYDRIVER_CAN_H_
#define MYDRIVER_CAN_H_

// SPI connected to the MCP2515
#define CAN_SPI_DEV 1
#define CAN_SPI_SLV 0

// Error Messages
#define   MYCAN_ERR_INIT          "Unable to init MCP2515"
#define   MYCAN_ERR_FRAME_TYPE    "Unknown frame type - unable to send msg"

// MCP2515 Registers
#define   MCP2515_TX_STD_FRAME        0x00
#define   MCP2515_TX_REMOTE_FRAME     0x01

// CAN configuration registers

#define   MCP2515_BFPCTRL    0x0C
#define   MCP2515_TXRTSCTRL  0x0D
#define   MCP2515_CANSTAT    0x0E
#define   MCP2515_CANCTRL    0x0F

#define   MCP2515_TEC        0x1C
#define   MCP2515_REC        0x1D

#define   MCP2515_CNF3       0x28
#define   MCP2515_CNF2       0x29
#define   MCP2515_CNF1       0x2A

#define   MCP2515_CANINTE    0x2B
#define   MCP2515_CANINTF    0x2C
#define   MCP2515_EFLG       0x2D

// CAN Receive Mask/Filter registers

#define   MCP2515_RXM0SIDH   0x20
#define   MCP2515_RXM0SIDL   0x21
#define   MCP2515_RXM0EID8   0x22
#define   MCP2515_RXM0EID0   0x23

#define   MCP2515_RXM1SIDH   0x24
#define   MCP2515_RXM1SIDL   0x25
#define   MCP2515_RXM1EID8   0x26
#define   MCP2515_RXM1EID0   0x27

#define   MCP2515_RXF0SIDH   0x00
#define   MCP2515_RXF0SIDL   0x01
#define   MCP2515_RXF0EID8   0x02
#define   MCP2515_RXF0EID0   0x03

#define   MCP2515_RXF1SIDH   0x04
#define   MCP2515_RXF1SIDL   0x05
#define   MCP2515_RXF1EID8   0x06
#define   MCP2515_RXF1EID0   0x07

#define   MCP2515_RXF2SIDH   0x08
#define   MCP2515_RXF2SIDL   0x09
#define   MCP2515_RXF2EID8   0x0A
#define   MCP2515_RXF2EID0   0x0B

#define   MCP2515_RXF3SIDH   0x10
#define   MCP2515_RXF3SIDL   0x11
#define   MCP2515_RXF3EID8   0x12
#define   MCP2515_RXF3EID0   0x13

#define   MCP2515_RXF4SIDH   0x14
#define   MCP2515_RXF4SIDL   0x15
#define   MCP2515_RXF4EID8   0x16
#define   MCP2515_RXF4EID0   0x17

#define   MCP2515_RXF5SIDH   0x18
#define   MCP2515_RXF5SIDL   0x19
#define   MCP2515_RXF5EID8   0x1A
#define   MCP2515_RXF5EID0   0x1B

// CAN Transmit Control/Header/Data registers

#define   MCP2515_TXB0CTRL   0x30
#define   MCP2515_TXB0SIDH   0x31
#define   MCP2515_TXB0SIDL   0x32
#define   MCP2515_TXB0EID8   0x33
#define   MCP2515_TXB0EID0   0x34
#define   MCP2515_TXB0DLC    0x35
#define   MCP2515_TXB0D0     0x36
#define   MCP2515_TXB0D1     0x37
#define   MCP2515_TXB0D2     0x38
#define   MCP2515_TXB0D3     0x39
#define   MCP2515_TXB0D4     0x3A
#define   MCP2515_TXB0D5     0x3B
#define   MCP2515_TXB0D6     0x3C
#define   MCP2515_TXB0D7     0x3D

#define   MCP2515_TXB1CTRL   0x40
#define   MCP2515_TXB1SIDH   0x41
#define   MCP2515_TXB1SIDL   0x42
#define   MCP2515_TXB1EID8   0x43
#define   MCP2515_TXB1EID0   0x44
#define   MCP2515_TXB1DLC    0x45
#define   MCP2515_TXB1D0     0x46
#define   MCP2515_TXB1D1     0x47
#define   MCP2515_TXB1D2     0x48
#define   MCP2515_TXB1D3     0x49
#define   MCP2515_TXB1D4     0x4A
#define   MCP2515_TXB1D5     0x4B
#define   MCP2515_TXB1D6     0x4C
#define   MCP2515_TXB1D7     0x4D

#define   MCP2515_TXB2CTRL   0x50
#define   MCP2515_TXB2SIDH   0x51
#define   MCP2515_TXB2SIDL   0x52
#define   MCP2515_TXB2EID8   0x53
#define   MCP2515_TXB2EID0   0x54
#define   MCP2515_TXB2DLC    0x55
#define   MCP2515_TXB2D0     0x56
#define   MCP2515_TXB2D1     0x57
#define   MCP2515_TXB2D2     0x58
#define   MCP2515_TXB2D3     0x59
#define   MCP2515_TXB2D4     0x5A
#define   MCP2515_TXB2D5     0x5B
#define   MCP2515_TXB2D6     0x5C
#define   MCP2515_TXB2D7     0x5D

// CAN Receive Control/Header/Data registers

#define   MCP2515_RXB0CTRL   0x60
#define   MCP2515_RXB0SIDH   0x61
#define   MCP2515_RXB0SIDL   0x62
#define   MCP2515_RXB0EID8   0x63
#define   MCP2515_RXB0EID0   0x64
#define   MCP2515_RXB0DLC    0x65
#define   MCP2515_RXB0D0     0x66
#define   MCP2515_RXB0D1     0x67
#define   MCP2515_RXB0D2     0x68
#define   MCP2515_RXB0D3     0x69
#define   MCP2515_RXB0D4     0x6A
#define   MCP2515_RXB0D5     0x6B
#define   MCP2515_RXB0D6     0x6C
#define   MCP2515_RXB0D7     0x6D

#define   MCP2515_RXB1CTRL   0x70
#define   MCP2515_RXB1SIDH   0x71
#define   MCP2515_RXB1SIDL   0x72
#define   MCP2515_RXB1EID8   0x73
#define   MCP2515_RXB1EID0   0x74
#define   MCP2515_RXB1DLC    0x75
#define   MCP2515_RXB1D0     0x76
#define   MCP2515_RXB1D1     0x77
#define   MCP2515_RXB1D2     0x78
#define   MCP2515_RXB1D3     0x79
#define   MCP2515_RXB1D4     0x7A
#define   MCP2515_RXB1D5     0x7B
#define   MCP2515_RXB1D6     0x7C
#define   MCP2515_RXB1D7     0x7D

#define   MCP2515_CMD_RESET      0xC0
#define   MCP2515_CMD_WRITE      0x02
#define   MCP2515_CMD_READ       0x03
#define   MCP2515_CMD_RTS        0x80
#define   MCP2515_CMD_BITMOD     0x05
#define   MCP2515_CMD_STATUS     0xA0
#define   MCP2515_CMD_RXSTATUS   0xB0

void    CAN_writeCommand(uint8_t theCommand);
uint8_t CAN_readCommand(uint8_t theCommand);
void    CAN_writeReg(uint8_t theAddress, uint8_t theData);
uint8_t CAN_readReg(uint8_t theAddress);

void    CAN_init(void);
void    CAN_debug(void);
void    CAN_sendMsg(uint32_t theIdentifier, uint8_t *theData, uint8_t theLength, uint8_t theFrameType);
int CAN_readMsg(uint32_t *theIdentifier, uint8_t *theData, uint8_t *theLength);

#endif
