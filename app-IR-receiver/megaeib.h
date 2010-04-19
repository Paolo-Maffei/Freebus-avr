//-----------------------------------------------------------------
// EIB / Header file for global definitions
//-----------------------------------------------------------------
// 19.Oct.03   MDf   created
// 10.Dec.03   MDf   added EIB frame decoding
// 01.Nov.05   PB	 MegaEIB
//-----------------------------------------------------------------
typedef unsigned char BYTE; // besser: uint8_t aus <inttypes.h>
typedef unsigned short WORD; // besser: uint16_t aus <inttypes.h>
typedef unsigned long DWORD; // besser: uint32_t aus <inttypes.h>
typedef unsigned long long QWORD; // besser: uint64_t aus <inttypes.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE 
#define FALSE 0
#endif

#ifndef _INCLUDE_EIB_
#define _INCLUDE_EIB_

#define OK        0        // basic return value

#define EIBPA     unsigned int     // Datatype for EIB Physical Address
#define EIBGA     unsigned int     // Datatype for EIB Group Address

#ifndef F_CPU
#define F_CPU 3686400L
#endif

//-----------------------------------------------------------------
// some common macros
//-----------------------------------------------------------------

#define MSB(x)	(((x) >> 8) & 0xff)     // most signif. byte of 2-byte integer
#define LSB(x)	((x) & 0xff)            // least signif. byte of 2-byte integer
#define MSW(x) (((x) >> 16) & 0xffff)  // most signif. word of 2-word integer
#define LSW(x) ((x) & 0xffff)          // least signif. byte of 2-word integer


//-----------------------------------------------------------------
// Macro for converting Phys.Addr (X.X.X) into EIBPA (=WORD) and
//           converting Grp.Addr  (X/X/X) into EIBGA (=WORD)
//-----------------------------------------------------------------
// PA :  z=zone  l=line  d=device
// GA :  a=1st   b=2nd   c=3rd      (use 3-level Group addresses)
//-----------------------------------------------------------------

#define PA(z,l,d)    (EIBPA) (((DWORD)(BYTE)(z & 0x0F)<<12)|((DWORD)(BYTE)(l & 0x0F)<<8)|(BYTE)(d & 0xFF))
#define GA(a,b,c)    (EIBGA) (((DWORD)(BYTE)(a & 0x1F)<<11)|((DWORD)(BYTE)(b & 0x07)<<8)|(BYTE)(c & 0xFF))


//-----------------------------------------------------------------
// Macro to Set Fields within the EIB Message Frame
//-----------------------------------------------------------------

//--- set physical address
#define SetPA(msg,pa)   msg[4] = MSB(pa); msg[5] = LSB(pa)

//--- set group address
#define SetGA(msg,ga)   msg[4] = MSB(ga); msg[5] = LSB(ga)

//--- set sequence count
#define SetSC(msg,sc)   msg[7] = (msg[7] & 0xC3) | ((sc & 0x0F) << 2)


//-----------------------------------------------------------------
// Data structure for query device information
//-----------------------------------------------------------------

typedef struct
{
   WORD     wMaskVersion;        // MaskVersion of this node
   BYTE     byPEItyp;            // 0109 PEI Typ
   BYTE     byAddressCount;      // 0116 no. of entries in the address table
   WORD     aAddressTable[128];  // address table

   BYTE     aDump[256];          // EEPROM data

} EIB_DEVICE_DATA;


//-----------------------------------------------------------------
// Service Primitives (valid for BCU2 !)
//-----------------------------------------------------------------

//--- Basic Services (usable on all layers)

#define PC_SET_VALUE_req      0x46
#define PC_GET_VALUE_req      0x4C
#define PC_GET_VALUE_con      0x4B

#define PEI_IDENTIFY_req      0x47
#define PEI_IDENTIFY_con      0x48
#define PEI_SWITCH_req        0x49
#define PEI_SWITCH_con        0x4A

//--- Link Layer

#define L_DATA_req            0x11
#define L_DATA_con            0x2E
#define L_DATA_ind            0x29

#define L_BUSMON_ind          0x2B
#define L_POLL_DATA_con       0x25

//--- Transport Layer

#define T_GROUP_DATA_req      0x32
#define T_POLL_DATA_req       0x33
#define T_POLL_DATA_con       0x75
#define T_DATA_GROUP_ind      0x7A
#define T_DATA_GROUP_con      0x7E
#define T_CONNECT_req         0x43
#define T_CONNECT_ind         0x85
#define T_CONNECT_con         0x86
#define T_DISCONNECT_req      0x44
#define T_DISCONNECT_con      0x88
#define T_DISCONNECT_ind      0x87
#define T_LOCAL_req           0x46
#define T_LOCAL_con           0x97
#define T_LOCAL_ind           0x92
#define T_DATA_CONNECTED_req  0x41
#define T_DATA_CONNECTED_con  0x8E
#define T_DATA_CONNECTED_ind  0x89
#define T_DATA_INDIVIDUAL_req 0x4A
#define T_DATA_INDIVIDUAL_con 0x9C
#define T_DATA_INDIVIDUAL_ind 0x94
#define T_BROADCAST_req       0x4C
#define T_BROADCAST_con       0x8F
#define T_BROADCAST_ind       0x8D

//--- Application Layer

#define A_USER_DATA_req       0x82
#define A_USER_DATA_con       0xD1
#define A_USER_DATA_ind       0xD2
#define A_RW_GROUP_req        0x72
#define A_POLL_DATA_req       0x73
#define A_POLL_DATA_con       0xE5
#define A_CONNECT_ind         0xD5
#define A_DISCONNECT_ind      0xD7
#define A_GROUP_DATA_ind      0xEA
#define A_GROUP_DATA_con      0xEE

//--- Application Layer

#define N_DATA_req            0x21
#define N_DATA_ind            0x49
#define N_DATA_con            0x4E
#define N_DATA_GROUP_req      0x22
#define N_DATA_GROUP_ind      0x3A
#define N_DATA_GROUP_con      0x3E
#define N_POLL_DATA_req       0x23
#define N_POLL_DATA_con       0x35
#define N_BROADCAST_req       0x2C
#define N_BROADCAST_ind       0x4D
#define N_BROADCAST_con       0x4F

#define U_VALUE_WRITE_req     0x71
#define U_VALUE_READ_req      0x74
#define U_VALUE_READ_con      0xE4
#define U_FLAGS_READ_req      0x7C
#define U_FLAGS_READ_con      0xEC
#define U_EVENT_ind           0xE7

#define M_INT_OBJ_DATA_req    0x9A
#define M_INT_OBJ_DATA_con    0xDC
#define M_INT_OBJ_DATA_ind    0xD4

#define TM_TIMER_ind          0xC1



//-----------------------------------------------------------------
// EIBnet commands and EIB frame decoding
// decode byte.7 (TPCI) and byte.8 (APCI) of an EIB message
// see EIBdecode.h for decoding schemes
//-----------------------------------------------------------------

//--- control commands 

#define  EIBCMD_SEND_RAW               0x8000
#define  EIBCMD_SEND_FT                0x8010
#define  EIBCMD_ACK                    0x80E5


//--- EIB decoded commands

#define  EIBNET_UNKNOWN                0xFFFF

#define  EIBNET_CONF                   0x8800

#define  EIBNET_VALUE_READ             0x1000
#define  EIBNET_VALUE_RSP              0x1001
#define  EIBNET_VALUE_WRITE            0x1002

#define  EIBNET_WRITE_PHYSADDR         0x1003
#define  EIBNET_READ_PHYSADDR_REQ      0x1010
#define  EIBNET_READ_PHYSADDR_RSP      0x1011

#define  EIBNET_READ_SERNO_REQ         0x1020
#define  EIBNET_READ_SERNO_RSP         0x1021
#define  EIBNET_SET_SERNO_REQ          0x1022

#define  EIBNET_SERVICE_INFO_REQ       0x1030
                             
#define  EIBNET_READ_ADC_REQ           0x1031
#define  EIBNET_READ_ADC_RSP           0x1032
                             
#define  EIBNET_READ_MEM_REQ           0x1040
#define  EIBNET_READ_MEM_RSP           0x1041
#define  EIBNET_WRITE_MEM_REQ          0x1042
                             
#define  EIBNET_USR_READ_MEM_REQ       0x1050
#define  EIBNET_USR_READ_MEM_RSP       0x1051
#define  EIBNET_USR_WRITE_MEM_REQ      0x1052
#define  EIBNET_USR_WRITE_MEMBIT_REQ   0x1053
                             
#define  EIBNET_USR_READ_MFACT_REQ     0x1060
#define  EIBNET_USR_READ_MFACT_RSP     0x1061
                             
#define  EIBNET_READ_MASKVERSION_REQ   0x1070
#define  EIBNET_READ_MASKVERSION_RSP   0x1071
                             
#define  EIBNET_RESTART_REQ            0x1080
                             
#define  EIBNET_WRITE_MEMBIT_REQ       0x1090
                             
#define  EIBNET_AUTHORIZE_REQ          0x10A0
#define  EIBNET_AUTHORIZE_RSP          0x10A1
                             
#define  EIBNET_SETKEY_REQ             0x10B0
#define  EIBNET_SETKEY_RSP             0x10B1
                             
#define  EIBNET_PROPERTY_VALUE_READ    0x10C0
#define  EIBNET_PROPERTY_VALUE_RSP     0x10C1
#define  EIBNET_PROPERTY_VALUE_WRITE   0x10C2
                             
#define  EIBNET_PROPERTY_DESCR_READ    0x10D0
#define  EIBNET_PROPERTY_DESCR_RSP     0x10D1
                             
#define  EIBNET_OPEN                   0x10F0
#define  EIBNET_CLOSE                  0x10F1
#define  EIBNET_OPEN_CONF              0x10F2
#define  EIBNET_reserved               0x10F3

#define  EIBNET_CLOSE_CONF             0x10F5
#define  EIBNET_TL_ACK                 0x10F6
#define  EIBNET_TL_NACK                0x10F7


//-----------------------------------------------------------------


#endif
