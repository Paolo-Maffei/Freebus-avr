#ifndef cbi
#define cbi(sfr, bit)     (_SFR_BYTE(sfr) &= ~_BV(bit)) 
#endif
#ifndef sbi
#define sbi(sfr, bit)     (_SFR_BYTE(sfr) |= _BV(bit))  
#endif

typedef union _dw2w4b_ {
  uint32_t dw;
  uint16_t w[2];
  uint8_t  b[4];
} dw2w4b;

typedef union _w2b_ {
  uint16_t w;
  uint8_t b[2];
} w2b;

