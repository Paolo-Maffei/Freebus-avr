#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "megaeib.h"
#include "megaEIBdecode.h"

//#include "lcd.h"
//#include <avr/signal.h>
//#include <inttypes.h>
//#include <avr/delay.h>
//#include <stdio.h>
//#include <math.h>

#define uchar unsigned char
#define uint unsigned int


#ifndef F_CPU
#define F_CPU 3686400     /**< clock frequency in Hz, used to calculate delay timer */
#endif


#define BAUD 19200
#define MYUBRR F_CPU/16/BAUD-1

//Prototypen
void do_cmd(uint ir_seq, uchar taste_lang);
void OnSendEIB(unsigned int wParam, unsigned int lParam);
void SendFTmsg(int8_t *pMsg, unsigned long dwToWrite);


// definitions für RC5 decoder
#define	xRC5_IN		PINB
#define	xRC5		PB1			// IR input low active

// definitions für IR-Empfang Kontroll LED
#define	LEDPORT		PORTC
#define	LEDPIN		PC5			// PIN für LED
#define LEDDDR		DDRC



#define RC5TIME 	1.778e-3		// 1.778msec
#define PULSE_MIN	(uchar)(F_CPU / 512 * RC5TIME * 0.4 + 0.5)
#define PULSE_1_2	(uchar)(F_CPU / 512 * RC5TIME * 0.8 + 0.5)
#define PULSE_MAX	(uchar)(F_CPU / 512 * RC5TIME * 1.2 + 0.5)
#define PAUSETIME (60 * RC5TIME) // Wiederholsequenzen nach 50 bit
								// hier 10 bit bzw. 17.88 ms Reserve
#define PAUSE_MIN   (uint)(F_CPU / 512 * PAUSETIME) //Mind. ISR Durchläufe f. Pausenlänge





/************************************************************************************************************/
/*                                   Global											                        */
/************************************************************************************************************/
static int8_t initTx1[] = {0x10, 0x40, 0x40, 0x16};

static int8_t initTx2[] = {0x68, 0x08, 0x08, 0x68,
							0x73,
							0xA9, 0x96, 0x18, 0x34, 0x56, 0x78, 0x0A,
							0xd6, 0x16};

static int8_t initTx3[] = {0x68, 0x06, 0x06, 0x68,
							0x53,
							0xA6, 0x01, 0x01, 0x16, 0x00,
							0x11, 0x16};
				   
volatile int8_t cRxBuf[32];
volatile unsigned int TxMsgLength;
volatile unsigned int RxCount;  		//Empfangszähler
volatile char bAckFlag = FALSE;    	// wird TRUE wenn ein ACK(=E5) empfangen wird
volatile char bInitBCU   = FALSE;    // wird TRUE wenn die BCU korrekt initialisiert wurde
volatile unsigned int RxMsgLength;
volatile unsigned short wGroupAddr;
volatile unsigned char  byDataLen;
volatile unsigned short wMsgCode;
volatile unsigned short wCmdCode;
volatile unsigned short wValue;
volatile unsigned int   nA,nB,nC;
volatile char bvalidMsg  = FALSE;    // wird TRUE wenn eine Framecodierung durchgeführt wurde
volatile char bvalidRead  = FALSE;    // wird TRUE wenn eine Anfrage gestellt wurde
volatile unsigned char byFCB = 1;   // FrameCounter-Bit

//volatile unsigned short Count_10ms;
char s[30];


//
// Globale Commando code für EIB Objekte
//
#define stepdown 	0	// step down (for blinds) 
#define up			0	// up (for blinds)
#define off 		0	// switch off
#define on 			1	// switch on
#define down		1	// down (for blinds)
#define stepup 		1	// step up (for blinds)
#define nope		99	// mache gar nix ! (sinnvoll bei unterdrücke React auf langen druck)
#define tgl_dimm	50	// toggle dimm direction nur für langen Tastendruck
#define tgl_sw		4	// toggle switch
#define tgl_dir		4	// toggle direction for blinds 
#define brighter	64  // 0b1100  = 1/8 brighter |, Aufpassen, darf nicht mit
#define darker		128 // 0b0100  = 1/8 darker   | toggles gleich sein

//
// Globale Variablen für EIB Objekte
//
// echte Werte
const uchar keys[] =			{       1,          2,         3,         4,         5,         6,         7,         8,         9,         0,         12,         13,         15,         16,         17,        32,        33 };
const uint short_sendGA[]=		{GA(1,3,0), GA(1,3,1), GA(4,1,1), GA(4,1,1), GA(4,1,0), GA(4,1,0), GA(4,1,2), GA(4,1,2), GA(4,1,3), GA(4,1,3), GA(10,0,8), GA(10,0,14), GA(10,0,2), GA(10,0,0), GA(1,3,21), GA(1,3,2), GA(1,3,5) };
const uint long_sendGA[]=		{GA(1,2,0), GA(1,2,1), GA(4,0,1), GA(4,0,1), GA(4,0,0), GA(4,0,0), GA(4,0,2), GA(4,0,2), GA(4,0,3), GA(4,0,3), GA(10,0,8), GA(10,0,14), GA(10,0,2), GA(10,0,0), GA(1,3,21), GA(1,3,2), GA(1,3,5) };
const uchar short_cmd[] =		{   tgl_sw,    tgl_sw,    stepup,  stepdown,    stepup,  stepdown,    stepup,  stepdown,    stepup,  stepdown,     tgl_sw,     tgl_sw,     tgl_sw,     tgl_sw,     tgl_sw,     tgl_sw,   tgl_sw };
const uchar long_cmd[] =		{ tgl_dimm,  tgl_dimm,        up,      down,        up,      down,        up,      down,        up,      down,       nope,       nope,       nope,       nope,       nope,       nope,     nope };

/*
//Testwerte
const uchar keys[] =			{       1,          2,         3 };
const uint short_sendGA[]=		{GA(1,2,1), GA(2,0,1), GA(1,1,1) };
const uint long_sendGA[]=		{GA(1,2,0), GA(2,0,0), GA(1,1,0) };
const uchar short_cmd[] =		{   tgl_sw,    tgl_sw,    tgl_sw };
const uchar long_cmd[] =		{   tgl_sw,    tgl_sw,    tgl_sw };
*/

/************************************************************************************************************/
/*                                   ISR für RC5 Empfang   								                        */
/************************************************************************************************************/						 
uchar	rc5_bit;				// bit value
uchar	rc5_time;				// count bit time
uint	rc5_tmp;				// shift bits in
uint	rc5_data;				// store result
uint    pause_ticks;			// Anzahl ISR Runden ohne IR Flanken		
volatile uchar   rc5_pause;		//Pause nach letztem decodierten RC5_data erkannt
uint rc5_data_old;
uchar repeat_counter;


SIGNAL (SIG_OVERFLOW0)
{
  uint tmp = rc5_tmp;				// for faster access

  TCNT0 = -2;					// 2 * 256 = 512 cycle

  //Kontroll LED
  if((xRC5_IN &(1<<xRC5))){LEDPORT |=(1<<LEDPIN);}//LED gegen VCC low active wie TSOP...
  else{LEDPORT &=~(1<<LEDPIN);}
  
  if( ++rc5_time > PULSE_MAX ){			// count pulse time
    if( !(tmp & 0x4000) && tmp & 0x2000 ){	// only if 14 bits received
    rc5_data = tmp;
	pause_ticks = 0;
    if(rc5_data==rc5_data_old){repeat_counter++;
		if(repeat_counter>50){repeat_counter=50;}//Überlauf verhindern
	}
	else{
		rc5_data_old=rc5_data;
		repeat_counter=0;	
	}
	}
	tmp = 0;
  }

  if( (rc5_bit ^ xRC5_IN) & 1<<xRC5 ){		// change detect
    rc5_bit = ~rc5_bit;						// 0x00 -> 0xFF -> 0x00
	pause_ticks =0; 						// Pausenzähler zurücksetzen
    if( rc5_time < PULSE_MIN )				// to short
      tmp = 0;

    if( !tmp || rc5_time > PULSE_1_2 ){		// start or long pulse time
      if( !(tmp & 0x4000) )					// not to many bits
        tmp <<= 1;							// shift
      if( !(rc5_bit & 1<<xRC5) )			// inverted bit
        tmp |= 1;							// insert new bit
      rc5_time = 0;							// count next pulse time
    }
  }
  if(++pause_ticks > PAUSE_MIN){		// Zeit für Wiederholsequenz überschritten
  	rc5_pause = 1;						// Pause erkannt
	pause_ticks = PAUSE_MIN+1; 			// Überlauf verhindern
  }
  else rc5_pause=0;
  
  rc5_tmp = tmp;
}

						 
/************************************************************************************************************/
/*                                   Sende Zeichen   								                        */
/************************************************************************************************************/						 
void usart_putc(unsigned char c) {
   // wait until UDR ready
	while(!(UCSRA & (1 << UDRE)));
	UDR = c;    // send character
}



/************************************************************************************************************/
/*                                   Sende String   								                        */
/************************************************************************************************************/
void uart_put_n (int8_t *s, unsigned n) {
  //  loop until n
  while (n--) {
    usart_putc(*s);
    s++;
  }
}



/************************************************************************************************************/
/*                            Auswertung des anstehenden Tastendrucks								                        */
/************************************************************************************************************/
void do_cmd(uint ir_seq, uchar taste_lang)
{
	uchar k;//Index
	uchar keycode;
	uchar t_bit;
	uint GA;
	uchar cmd;
	uchar d_start=0;
	uchar adresse;
	uchar EIBData=0;
	//welche Taste auf FB gedrückt ?
	adresse = ir_seq >> 6 & 0x1F;
	
	//reagiere nur auf device Adresse 10 !!!!
	if(adresse!=10){return;}
	
	keycode = ((ir_seq & 0x3F) | (~ir_seq >> 7 & 0x40));
	t_bit=( ir_seq >> 11 & 1);
	//suche Zuordnungen zu Taste
	for (k=0; k < sizeof(keys); k++){
		if (keycode==keys[k]){ //Taste gefunden
			if(taste_lang){
				GA = long_sendGA[k];
				cmd = long_cmd[k];
			}
			else{
				GA = short_sendGA[k];
				cmd = short_cmd[k];
			}
			switch (cmd){
				case 99:
				return;
				break;
				case 0:
				EIBData=0;
				break;
				case 1:
				EIBData=1;
				break;
				case 50:
				if(t_bit) {EIBData=0b0001;}//1/8 dunkler
				else{EIBData=0b1001;} //1/8 heller
				d_start=1;
				break;
				case 4:
				EIBData=t_bit;
				break;
				case 64:
				EIBData=0b1001; //heller
				d_start=1;
				break;
				case 128:
				EIBData=0b0001; //dunkler
				d_start=1;
				break;
			}
			if(taste_lang){
					OnSendEIB(GA,EIBData);
					while(!rc5_pause){}//Warte Pause ab
					if(d_start){
						OnSendEIB(GA,0);
						d_start=0;
					}
			}
			else{ 
				//put_s("short\n");
				OnSendEIB(GA,EIBData);
			}
		}
	}
	return;
}

/************************************************************************************************************/
/*									 OnSendEIB																*/
/*    								 sendet eine EIB Message über FT1.2 Protokoll							*/
/************************************************************************************************************/
void OnSendEIB(unsigned int wParam, unsigned int lParam)
{
   //-- Prototype of message
   int8_t cTxBuf[32] = {0x11, 0x0c, 0x11, 0xFF, 0x53, 0x00, 0xe1, 0x00, 0x80, 0x00};
   
   //-- set group address
   cTxBuf[4] = MSB(wParam);
   cTxBuf[5] = LSB(wParam);

   //-- set value (1 bit)
   cTxBuf[8] += (BYTE) (lParam & 0xFF);

   SendFTmsg(cTxBuf, 9);							 // send L_DATA.req: (Obj)ValueRead
  
}


/************************************************************************************************************/
/*								     SendFTmsg														        */
/*   								 kapselt eine EIB-Message in das FT1.2 Protokoll und schreibt diese     */
/*    								 auf die serielle Schnittstelle											*/
/************************************************************************************************************/
void SendFTmsg(int8_t *pMsg, unsigned long dwToWrite)
{
   int8_t   pBuf[32];                  // temp transmit buffer
   BYTE     byDB;                      // data byte
   DWORD    dwIdx = 0;                 // buffer index
   DWORD    i;
   BYTE     byFT12csum = 0;            // FT12 protocol checksum : BYTE added

   //-- FT1.2 header with length
   //   length: add Fct.Code
   pBuf[dwIdx++] = 0x68;
   pBuf[dwIdx++] = (BYTE) dwToWrite+1;
   pBuf[dwIdx++] = (BYTE) dwToWrite+1;
   pBuf[dwIdx++] = 0x68;

   //-- FT1.2 function code = Send_UDAT
   //   toggle FrameCountBit
   pBuf[dwIdx++] = 0x53 | ((byFCB++ & 1) << 5);

   //-- copy MessageCode
   pBuf[dwIdx++] = *(pMsg++);

   //-- copy msg into buffer and calc checksums
   for (i=1; i < dwToWrite; i++)
   {
      byDB = *(pMsg++);
      pBuf[dwIdx++] = byDB;
      byFT12csum += byDB;
   }

   //-- add Fct.Code and Msg.Code and EIB.checksum
   byFT12csum += pBuf[4] + pBuf[5];

   //-- set FT12 checksum and add end field
   pBuf[dwIdx++] = byFT12csum;
   pBuf[dwIdx++] = 0x16;

   
   //-- send message via BCU
   bAckFlag = FALSE;
   
   //--- size = message + (4)header + checksum + 0x16
   TxMsgLength = pBuf[1] + 4 + 1 + 1;
   
   uart_put_n (pBuf, TxMsgLength);
   
   
   while (!bAckFlag) 
   {
		; //wait..
   }
	
}


/************************************************************************************************************/
/*                                   INIT	USART								                        */
/************************************************************************************************************/
void initUSART(unsigned int ubrr) {
		
	// USART	
    RxCount = 0;
	/* Set baud rate */
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;

	// Enable receiver and transmitter; enable RX interrupt
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);

	// Set frame format: 8data, 1stop bit, even / gerade Parity 
	UCSRC = (1<<URSEL)|(0<<UMSEL)|(1<<UCSZ0)|(1<<UCSZ1) |(1<<UPM1); 
	
	
}

/************************************************************************************************************/
/*                                   INIT BCU										                        */
/************************************************************************************************************/
void initBCU(void) {
	 
    //1. Reset-CMD senden ******************************************************
	bAckFlag = FALSE; 
	uart_put_n (initTx1, sizeof(initTx1));
	while (!bAckFlag) {
		; //wait..
	}	

	
	//2. Layer in der BCU setzen ***********************************************
	bAckFlag = FALSE; 
	uart_put_n (initTx2, sizeof(initTx2));
	while (!bAckFlag) {
		; //wait..
	}

	
	//3. Länge der MsgTabelle auf 0 setzen > alle Msgs vom Bus durchlassen *****
	bAckFlag = FALSE; 
	uart_put_n (initTx3, sizeof(initTx3));
	while (!bAckFlag) {
		; //wait..
	}

}


/************************************************************************************************************/
/*                                    Layer-2   EIB-Message dekodieren							            */
/************************************************************************************************************/

#define  EIB_MSG_SIZE      1
#define  SERVICE_CODE      5
#define  EIB_MSG           6
#define  CONTROL_FIELD     5+1
#define  EIB_NPCI          5+6
#define  EIB_BYTE7         5+7
#define  EIB_BYTE8         5+8

void DecodeEIBmsg(void)
{
   int i;
   
   //------------------------------------------
   // L_DATA.ind
   //------------------------------------------
   // SP  : only L_DATA_ind
   // CF.5: repeat flag=1 (first attempt)

   if ( (cRxBuf[SERVICE_CODE] == L_DATA_ind) &&
        ((cRxBuf[CONTROL_FIELD] & 0xF0) == 0xB0) )
   {
      
      //-- message dekodieren
      wGroupAddr = (cRxBuf[9] << 8) | cRxBuf[10];
      byDataLen  =  (cRxBuf[EIB_NPCI] & 0x0F);
      wMsgCode   = ((cRxBuf[EIB_BYTE7] & 0xC3) << 8) | cRxBuf[EIB_BYTE8];

      wCmdCode   =  EIBNET_UNKNOWN;

      wMsgCode &= 0xFFC0;

      //-- search matching TPCI/APCI code 
      for (i=0; i < EIB_CMD_COUNT; i++)
      {
         if ((wMsgCode & EIBdecodeTable[i].wFrameMask) == EIBdecodeTable[i].wFrameCode)
         {
            wCmdCode = EIBdecodeTable[i].wEIBcmd;
         }
      }

      nA = (wGroupAddr & 0xf800) >> 11;
      nB = (wGroupAddr & 0x0700) >> 8;
      nC = (wGroupAddr & 0x00ff);

      //-- Value lesen
      switch (byDataLen)
      {
         case 1: wValue = cRxBuf[13] & 0x3F; break;
         case 2: wValue = cRxBuf[14];        break;
         case 3: wValue = (cRxBuf[14]<<8) | (cRxBuf[15]); break;
      
         default: wValue = 0xFFFF;
      }
      //---- nur auf diese CmdCodes reagieren
      if ( (wCmdCode == EIBNET_VALUE_RSP) ||
           (wCmdCode == EIBNET_VALUE_WRITE) )
      {
         //-- auf dieses Event reagieren
         //DispatchMsg(wGroupAddr, wValue);
      }
      //*******************************
	  
	  bvalidMsg = TRUE; //decodierung ok..

   }
}




/************************************************************************************************************/
/*                                   SIGNAL (SIG_UART_RECV)   // USART RX interrupt                       */
/************************************************************************************************************/
SIGNAL (SIG_UART_RECV) { 

char cRx; // Variable für empfangenes Zeichen
	
   cRx = UDR;
   cRxBuf[RxCount++] = cRx; // Füge empfangenes Zeichen zum Empfangspuffer hinzu

   //-- Ack von BCU empfangen
   if ((RxCount == 0x0001) && (cRx == 0xE5)) {
		RxCount = 0;
		RxMsgLength = 0;
		bAckFlag=TRUE;
		cRxBuf[RxCount] = 0x00;
		//return TRUE;
   }
  	//-- 
   	if ((RxCount >= 4) && (cRxBuf[0] == 0x68) && (cRxBuf[1] == cRxBuf[2]) && (cRxBuf[3] == 0x68))
   	{
        //--- size = message + (4)header + checksum + 0x16
      	RxMsgLength = cRxBuf[1] + 4 + 1 + 1;
   	}

    //-- MsG Ende
   	if ((cRx == 0x16) && (RxCount >= RxMsgLength))
   	{	  
      	DecodeEIBmsg();
	  
        RxCount = 0;
        RxMsgLength = 0;
        //return (TRUE);
   	}
		
}



/************************************************************************************************************/
/*                                   Bestätigung AcK senden							                        */
/************************************************************************************************************/
void SendAck(void)
{
   usart_putc(0xE5); 
}




/************************************************************************************************************/
/*                                   main										                        */
/************************************************************************************************************/

int main(void){

	uint i;
	
	LEDDDR |=(1<<LEDPIN); 		// DDR von LED als Ausgang
  	
	TCCR0 = 1<<CS02;			//divide by 256
 	TIMSK = 1<<TOIE0;			//enable timer interrupt

	initUSART(MYUBRR);			// init USART 
	sei(); 			 			// enable interrupts
	
	//Achtung nur zum debuggen auskommentieren
	initBCU();					// init BCU
 	
	for(;;){					// main loop
    	cli();
    	i = rc5_data;// in rc5_data ist immer die letzte IR-Sequenz enthalten
		sei();
		
		if(i && rc5_pause){ //Einzeltastendruck
			do_cmd(i,0);
			rc5_data=0;
		}
		if(i && (repeat_counter>4)){ //Langer Tastendruck, schon mehr als 4 Sequenzen
			do_cmd(i,1);
			rc5_data=0;
		}
		
	}
}
