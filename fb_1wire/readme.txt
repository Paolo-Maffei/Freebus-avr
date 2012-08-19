Adressaufteilung:
0x0111 --> ASSOCTABPTR --> hier liegt die Start-Adresse der Zuordnungen


0x0116 --> Start Adresstable --> COUNT_GRP_ADDRESSES --> min. 1 (physikalische Adresse)
0x0117 --> PA_ADDRESS_HIGH
0x0118 --> PA_ADDRESS_LOW
0x0119 --> START_ADDRESS_GROUP_ADR --> Start 1. Gruppenadresse (jeweils 2 Byte !!) --> high --> low

0x0160  Bit 0-7 zyklBase  Allgemein  
  0  130 ms
  17  260 ms
  34  520 ms
  51  1 s
  68  2,1 s
  85  4,2 s
  102  8,4 s
  119  17 s
  136  34 s
  153  1,1 min
  170  2,2 min
  187  4,5 min
  204  9 min
  221  18 min
  238  35 min
  255  1,2 h

0x0161 Bit 0-7  count sensors

//Sensor 0   16Bytes
0x0162 Serial Byte 0
0x0163 Serial Byte 1
0x0164 Serial Byte 2
0x0165 Serial Byte 3
0x0166 Serial Byte 4
0x0167 Serial Byte 5
0x0168 Serial Byte 6
0x0169 Serial Byte 7
0x016A ZyklSend Bit 7   ja/nein
		Bit 0-6 Factor 0-127
0x016B Flags
0x016C Messdiff
0x016D Messdiff
0x016E Grenzwert1
0x016F Grenzwert1
0x0170 Grenzwert2
0x0171 Grenzwert2

//Sensor 1 ....
