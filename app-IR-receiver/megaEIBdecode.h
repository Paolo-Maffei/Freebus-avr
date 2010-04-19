//-----------------------------------------------------------------
// EIB decoding table for EIB frames
//-----------------------------------------------------------------
// 10.Dec.03   MDf   created
//-----------------------------------------------------------------

#ifndef _INCLUDE_EIBDECODE_
#define _INCLUDE_EIBDECODE_

//-----------------------------------------------------------------

typedef struct
{
   unsigned short  wEIBcmd;          // schematic command
   unsigned short  wFrameMask;       // AND-bitmask
   unsigned short  wFrameCode;       // resulting code
}
EIBNET_DECODER;

//-----------------------------------------------------------------
// This is used to decode Byte.7 and Byte.8 (TPCI/APCI) of an
// EIB message. These bytes are treated as WORD, &
//-----------------------------------------------------------------

#define EIB_CMD_COUNT   42

EIBNET_DECODER EIBdecodeTable[EIB_CMD_COUNT] =
{
   {EIBNET_VALUE_READ,           0xFFC0, 0x0000},
   {EIBNET_VALUE_RSP,            0xFFC0, 0x0040},
   {EIBNET_VALUE_WRITE,          0xFFC0, 0x0080},
                                         
   {EIBNET_WRITE_PHYSADDR,       0xFFFF, 0x00C0},
   {EIBNET_READ_PHYSADDR_REQ,    0xFFFF, 0x0100},
   {EIBNET_READ_PHYSADDR_RSP,    0xFFFF, 0x0140},
                                         
   {EIBNET_READ_SERNO_REQ,       0xFFFF, 0x03DC},
   {EIBNET_READ_SERNO_RSP,       0xFFFF, 0x03DD},
   {EIBNET_SET_SERNO_REQ,        0xFFFF, 0x03DE},
                                         
   {EIBNET_SERVICE_INFO_REQ,     0xFFFF, 0x03DF},
                                         
   {EIBNET_READ_ADC_REQ,         0xFFC0, 0x4180},
   {EIBNET_READ_ADC_RSP,         0xFFC0, 0x41C0},
                                         
   {EIBNET_READ_MEM_REQ,         0xFFF0, 0x4200},
   {EIBNET_READ_MEM_RSP,         0xFFF0, 0x4240},
   {EIBNET_WRITE_MEM_REQ,        0xFFF0, 0x4280},
                                         
   {EIBNET_USR_READ_MEM_REQ,     0xFFFF, 0x42C0},
   {EIBNET_USR_READ_MEM_RSP,     0xFFFF, 0x42C1},
   {EIBNET_USR_WRITE_MEM_REQ,    0xFFFF, 0x42C2},
   {EIBNET_USR_WRITE_MEMBIT_REQ, 0xFFFF, 0x42C4},
                                         
   {EIBNET_USR_READ_MFACT_REQ,   0xFFFF, 0x42C5},
   {EIBNET_USR_READ_MFACT_RSP,   0xFFFF, 0x42C6},
                                         
   {EIBNET_READ_MASKVERSION_REQ, 0xFFC0, 0x4300},
   {EIBNET_READ_MASKVERSION_RSP, 0xFFC0, 0x4340},
                                         
   {EIBNET_RESTART_REQ,          0xFFC0, 0x4380},
                                         
   {EIBNET_WRITE_MEMBIT_REQ,     0xFFFF, 0x43D0},
                                         
   {EIBNET_AUTHORIZE_REQ,        0xFFFF, 0x43D1},
   {EIBNET_AUTHORIZE_RSP,        0xFFFF, 0x43D2},
                                         
   {EIBNET_SETKEY_REQ,           0xFFFF, 0x43D3},
   {EIBNET_SETKEY_RSP,           0xFFFF, 0x43D4},
                                         
   {EIBNET_PROPERTY_VALUE_READ,  0xFFFF, 0x43D5},
   {EIBNET_PROPERTY_VALUE_RSP,   0xFFFF, 0x43D6},
   {EIBNET_PROPERTY_VALUE_WRITE, 0xFFFF, 0x43D7},
                                         
   {EIBNET_PROPERTY_DESCR_READ,  0xFFFF, 0x43D8},
   {EIBNET_PROPERTY_DESCR_RSP,   0xFFFF, 0x43D9},
                                         
   {EIBNET_OPEN,                 0xFF00, 0x8000},
   {EIBNET_CLOSE,                0xFF00, 0x8001},
   {EIBNET_OPEN_CONF,            0xFF00, 0x8002},
   {EIBNET_reserved,             0xFF00, 0x8003},
   {EIBNET_reserved,             0xFF00, 0xC000},
   {EIBNET_CLOSE_CONF,           0xFF00, 0xC001},
   {EIBNET_TL_ACK,               0xFF00, 0xC002},
   {EIBNET_TL_NACK,              0xFF00, 0xC003}
};


//-----------------------------------------------------------------

#endif
