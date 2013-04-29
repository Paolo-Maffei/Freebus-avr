/* $Id$ */
/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ /
 *  /_/   /_/ |_/_____/_____/_____/\____//____/
 *
 *  Copyright (c) 2008 Matthias Fechner <matthias@fechner.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *  fuses hfuse=0xD9  lfuse=0xAE
 */
/**
* @file   fb_2-dimmer_app.c
* @author
* @date   Sun Aug 24 21:30:01 2008
*
* @brief  Dimmer application based on the Gira 1032 2 channel dimmer.
*/
// funktiionen die der org dimmer hat
  //@DODO r�ckmeldeobjekte
  //@DODO zeitfunktionen
  //@DODO Sperrfunktion
  //@DODO Lichtzene
  //@DODO Status R�ckmelden
  //@DODO wertre�ckmeldeobjekt vorhanden
  //@DODO lesenobjekte 1Byte
//funktionen die der Dimmer nie k�nnen wird
  // kurzschluss melden
  // lastausfall melden


#include "fb_2-dimmer_app.h"
#include "util/twi.h"

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega88__)
#define TIMSK TIMSK2  ///< @todo add documentation
#define TCCR2 TCCR2B  ///< @todo add documentation
#define OCR2 OCR2B    ///< @todo add documentation
#define GICR EIMSK    ///< @todo add documentation
#endif


#define MAXDIMMWERT 200   ///< @todo add documentation
#define MAXZEIT 60000	///< maximalzeit bis zum abbruch i2c



static uint8_t zldurch2=0;			///< Timer durch 2 (char)

unsigned char dimmenaufab=0;  ///< @todo add documentation
unsigned char aufab=0;  ///< @todo add documentation
unsigned char dimmgeschwindikeit=0;  ///< @todo add documentation
unsigned char mindimmwert_k1=0x10;		///< minimaldimmwert von der applikation Ok
unsigned char anspringen_k1=1;		///< andimmen (0) oder anspringen (1) K1
unsigned char einschathellikeit_k1=MAXDIMMWERT;  ///< @todo add documentation
uint8_t gK1helldunkel=0;				///< 9=heller 0=stop 1=dunkler
static uint8_t K1dimmwert=0;  ///< @todo add documentation
static uint8_t K1dimmwert_ausgang=0;  ///< @todo add documentation
unsigned char mk1=0; ///< merker Kanal1 zum �bertragen uber i2c
unsigned char mk2=0; ///< merker Kanal2 zum �bertragen uber i2c


unsigned char mindimmwert_k2=0x10;		///< minimaldimmwert von der applikation Ok
unsigned char anspringen_k2=1;		///< andimmen (0) oder anspringen (1) K1
unsigned char einschathellikeit_k2=MAXDIMMWERT;  ///< @todo add documentation
uint8_t gK2helldunkel=0;				///< 9=heller 0=stop 1=dunkler
static uint8_t K2dimmwert=0;  ///< @todo add documentation
static uint8_t K2dimmwert_ausgang=0;  ///< @todo add documentation

//             hellikeit    0,grund,10,20,30,40,50, 60, 70, 80, 90,Max
unsigned char hellikeit[]={0,25,40,53,67,80,95,120,140,160,180,200,0};   ///< @todo add documentation





extern struct grp_addr_s grp_addr;

static uint8_t portValue;                 /**< defines the port status. LSB IO0 and MSB IO8, ports with delay can be set to 1 here
                                              but will be switched delayed depending on the delay */
uint16_t currentTime;              ///< defines the current time in 10ms steps (2=20ms)
static uint8_t currentTimeOverflow;       ///< the amount of overflows from currentTime


void switchObjects(void);
void timerOverflowFunction(void);


/**
* Timer1 is used as application timer. It increase the variable currentTime every 130ms and currentTimeOverflow if
* currentTime runs over 16-bit.
*
* @return
*/
void timerOverflowFunction()
{
     uint8_t timerActive;

     // check if programm is running
     if(read_memory(0x010D)!=0xFF)
          return;

     timerActive=read_memory(0x01EA);
     if(currentTime==0xFFFF)
     {
          currentTime=0;
          currentTimeOverflow++;
     }else
     {
          currentTime++;
     }

	//Dimmgeschwindikeit
	dimmgeschwindikeit=gK1helldunkel&0x07;
     if(K1dimmwert <= (MAXDIMMWERT-dimmgeschwindikeit)&&(gK1helldunkel&8)!=0)	//heller 9( bit 3 heller dunkler ,bit 0-2 geschwindikeit)
		{
		if(K1dimmwert<mindimmwert_k1)
			K1dimmwert=mindimmwert_k1;
		else
			K1dimmwert+=dimmgeschwindikeit;
		}
     if(K1dimmwert >= (mindimmwert_k1+dimmgeschwindikeit) && (gK1helldunkel&8)==0)	//dunkler
		{
		K1dimmwert-=dimmgeschwindikeit;
		}

	dimmgeschwindikeit=gK2helldunkel&0x07;
     if(K2dimmwert <= (MAXDIMMWERT-dimmgeschwindikeit)&&(gK2helldunkel&8)!=0)	//heller 9( bit 3 heller dunkler ,bit 0-2 geschwindikeit)
		{
		if(K2dimmwert<mindimmwert_k2)
			K2dimmwert=mindimmwert_k2;
		else
			K2dimmwert+=dimmgeschwindikeit;
		}
     if(K2dimmwert >= (mindimmwert_k2+dimmgeschwindikeit) && (gK2helldunkel&8)==0)	//dunkler
		K2dimmwert-=dimmgeschwindikeit;

     return;
}

/**
* ISR is called if on TIMER1 the comparator B matches the defined condition.
*
*/
ISR(TIMER1_COMPB_vect)
{
     return;
}

/**
* Set parameters to defined values if controller got power the first time.
* That function is called from the protocol init in fb_prot.c if eeprom is empty.
*
*/
void setApplicationDefaults(void)
{
     write_memory_p(0x0104,0x08);	// Herstellercode 0x04 = Jung  ,0x08 = Gira
     write_memory_p(0x0105,0x00);	// Ger�te Typ (2038.10) 2060h Bestellnummer: 2138.10REG  	//20  //00  gira 1032
     write_memory_p(0x0106,0x00);  	// 	"	"	"								//60	 //00  gira 1032
     write_memory_p(0x0107,0x01);	// Versionsnummer
// programm zu gross
  /*   write_memory_p(0x0000,0x00); // default is off
     write_memory_p(0x01C2,0x00); // miniwert K1+2 =0
     write_memory_p(0x01C6,0x00); // anspringen K1+2 =0;
     write_memory_p(0x01C4,0xff); // einschalthellikeit K1+K1 =0xF
     write_memory_p(0x01F2,0x00); //
    */ return;
}

/**
* Function is called when microcontroller gets power or if the application must be restarted.
* It restores data like in the parameters defined.
*
* @return FB_ACK or FB_NACK
*/
uint8_t restartApplication(void)
{
     uint8_t i,temp;
     uint16_t initialPortValue;

	mindimmwert_k1=	((read_memory(0x01C2)&0x0f)*10)+10;
	anspringen_k1=		 (read_memory(0x01C6)>>3)&0x01;
	einschathellikeit_k1=(read_memory(0x01C4)&0x0f);
	hellikeit[0x1]=mindimmwert_k1;
	hellikeit[0xB]=MAXDIMMWERT;
	K1dimmwert=hellikeit[(read_memory(0x01e2)&0x0f)]; //Verhalten bei Busspannungswiederkehr

	mindimmwert_k2=	((read_memory(0x01C2)>>4)*10)+10;
	anspringen_k2=		 (read_memory(0x01C6)>>7)&0x01;
	einschathellikeit_k2=(read_memory(0x01C4)>>4);
	hellikeit[0x1]=mindimmwert_k2;
	hellikeit[0xB]=MAXDIMMWERT;
	K1dimmwert=hellikeit[(read_memory(0x01e2)>>4)]; //Verhalten bei Busspannungswiederkehr

//ri     setupIOsAsOutput();
//     DDRB |= (1<<PB3);

     // reset timer values
	currentTime=0;
	currentTimeOverflow=0;
  //ri   timerRunning=0;

     // read old saved status
     portValue=read_memory(0x0000);
//	DEBUG_PUTHEX(portValue);

     // check if at power loss we have to restore old values (see 0x01F6) and do it here
     initialPortValue= (read_memory(0x01F7) << 8) | (read_memory(0x01F6));
     for(i=0;i<=7;i++)
     {
          temp=(initialPortValue>>(i*2))&0x03;
//          DEBUG_PUTHEX(temp);
          if(temp==0x01)
          {
               // open contact
               portValue &= ~(1<<i);
//               DEBUG_PUTHEX(i);
//               DEBUG_PUTS("P");
          }else if(temp==0x02)
          {
               // close kontakt
               portValue |= (1<<i);
//               DEBUG_PUTHEX(i);
//               DEBUG_PUTS("L");
          }
     }
//     DEBUG_PUTHEX(portValue);
//     switchObjects();

     return 1;
}

/**
* Read status from port and return it.
*
* @param rxmsg
*
* @return
*/
uint8_t readApplication(struct msg *rxmsg)
{
     struct fbus_hdr *hdr=(struct fbus_hdr *) rxmsg->data;
     //DEBUG_PUTS("Read");

     uint8_t i;
     uint8_t einaus;
     uint16_t destAddr=((hdr->dest[0])<<8 | hdr->dest[1]);

     uint8_t assocTabPtr=read_memory(0x0111);                   // points to start of association table (0x0100+assocTabPtr)
     uint8_t countAssociations=read_memory(0x0100+assocTabPtr); // number of associations saved in associations table
     uint8_t numberInGroupAddress;                              // reference from association table to group address table
     uint8_t commObjectNumber;                                  // reference from association table to communication object table


     for(i=0;i<countAssociations;i++)
     {
          numberInGroupAddress=read_memory(0x0100+assocTabPtr+1+(i*2));
          // check if valid group address reference
          if(numberInGroupAddress == 0xFE)
               continue;
          commObjectNumber=read_memory(0x0100+assocTabPtr+1+(i*2)+1);

          // now check if received address is equal with the safed group addresses, substract one
          // because 0 is the physical address, check also if commObjectNumber is between 0 and 7
          // (commObjectNumber is uint8_t so cannot be negative don't need to check if >= 0)
          if(destAddr==grp_addr.ga[numberInGroupAddress-1] && commObjectNumber <= 1)
          {
            if((commObjectNumber==0 && (read_memory(0x018d)&(1<<3)) != 0) || (commObjectNumber==1&&(read_memory(0x0190)&(1<<3))!=0))//lesen erlaubt
              {   // found group address and read bit is on

               /** @todo lesen des 1byte wert */
              struct msg * resp = AllocMsgI();
               if (!resp)
                    return FB_NACK;
               struct fbus_hdr * hdr= (struct fbus_hdr *) resp->data;

               resp->repeat=3;
               resp->len = 9;//9
               hdr->ctrl = 0xBC;
               hdr->src[0] = read_memory(0x0117);
               hdr->src[1] = read_memory(0x0118);
               hdr->dest[0]=grp_addr.ga[numberInGroupAddress-1]>>8;
               hdr->dest[1]=grp_addr.ga[numberInGroupAddress-1];
               hdr->npci = 0xE1;
               hdr->tpci = 0x00;
               // put data into the apci octet
               if(commObjectNumber==0)
                 einaus=K1dimmwert_ausgang;
               else
                 einaus=K2dimmwert_ausgang;
               if(einaus!=0)
                 einaus=1;
//               hdr->apci = 0x40+einaus;
               hdr->apci = 0x40+0x3f;


               fb_hal_txqueue_msg(resp);
              }
          } else if(commObjectNumber > 1 && commObjectNumber < 12)
		{
			// additinal function
			/** @todo write part additional functions */
 //              DEBUG_PUTS("ZF");
//               DEBUG_NEWLINE();
		}

  }
	return FB_ACK;
}

/**
* Function is called if A_GroupValue_Write is received it is the function "EIS1" or "Data Type Boolean" for the relais module.
* Read all parameters in that function and set global variables.
*
* @param rxmsg
*
* @return
*/
uint8_t runApplication(struct msg *rxmsg)
{
	struct fbus_hdr * hdr= (struct fbus_hdr *) rxmsg->data;

     uint8_t i;
     uint16_t destAddr=((hdr->dest[0])<<8 | hdr->dest[1]);

     uint8_t assocTabPtr=read_memory(0x0111);                   // points to start of association table (0x0100+assocTabPtr)
     uint8_t countAssociations=read_memory(0x0100+assocTabPtr); // number of associations saved in associations table
     uint8_t numberInGroupAddress;                              // reference from association table to group address table
     uint8_t commObjectNumber;                                  // reference from association table to communication object table
     uint8_t commStabPtr=read_memory(0x0112);                   // points to communication object table (0x0100+commStabPtr)
     uint8_t commValuePointer;                                  // pointer to value
     uint8_t commConfigByte;                                    // configuration byte
     uint8_t commValueType;                                     // defines type of byte
     uint8_t data;

     // handle here only data with 6-bit length, maybe we have to add here more code to handle longer data
     /** @todo handle data with more then 6 bit */
     // dump all received octets
	for(i=0;i<rxmsg->len;i++)
     {
		DEBUG_PUTHEX(rxmsg->data[i]);
          DEBUG_SPACE();
	}
     DEBUG_PUTC(':');
	DEBUG_PUTHEX(rxmsg->data[8]);
     DEBUG_NEWLINE();


     data=hdr->apci & 0x3F;


   for(i=0;i<countAssociations;i++)
     {
          numberInGroupAddress=read_memory(0x0100+assocTabPtr+1+(i*2));
          // check if valid group address reference
          if(numberInGroupAddress == 0xFE)
               continue;
          commObjectNumber=read_memory(0x0100+assocTabPtr+1+(i*2)+1);

          // now check if received address is equal with the safed group addresses, substract one
          // because 0 is the physical address, check also if commObjectNumber is between 0 and 7
          // (commObjectNumber is uint8_t so cannot be negative don't need to check if >= 0)
          if(destAddr==grp_addr.ga[numberInGroupAddress-1] && commObjectNumber <= 7)
          {
               // found group address
			//DEBUG_PUTS(" ");
			DEBUG_PUTHEX(commObjectNumber);
			DEBUG_PUTHEX(data);
			//DEBUG_PUTS(" ");
               // read communication object (3 Byte)
               commValuePointer=read_memory(0x0100+commStabPtr+2+(commObjectNumber*3));
               commConfigByte=read_memory(0x0100+commStabPtr+2+(commObjectNumber*3+1));
               commValueType=read_memory(0x0100+commStabPtr+2+(commObjectNumber*3+2));



			// commObjectNumber== 0 Schalten K1 data== 0=aus  1=ein
			// commObjectNumber== 1 Schalten K2 data== 0=aus  1=ein
			// commObjectNumber== 2 Dimmen K1  data== 9=heller 0=stop 1=dunkler
			// commObjectNumber== 3 Dimmen K2  data== 9=heller 0=stop 1=dunkler
			// commObjectNumber== 4 WERT K1 kommt noch kein wert
			// commObjectNumber== 5 WERT K2 kommt noch kein wert


			if(data==0&&commObjectNumber==0)		//schaltobjekt K1 aus
               {
//				portValue |= (1<<commObjectNumber);
				portValue |= 1;
				K1dimmwert=0;
			}
 			//KANAL1
			if(data==1&&commObjectNumber==0)		//schaltobjekt K1 EIN
               {
				portValue &= ~1;
				K1dimmwert=hellikeit[einschathellikeit_k1];
			}
			if(commObjectNumber==2)		//schaltobjekt K1 aus
               {
				gK1helldunkel=data;
//				portValue |= (1<<commObjectNumber);
				portValue |= 1;
               }
			if(commObjectNumber==4)		//wertobjekt K1
			{
			//if(rxmsg->data[8]>=mindimmwert_k1&&rxmsg->data[8]<=MAXDIMMWERT)
				K1dimmwert = rxmsg->data[8];
			}
			if(data==0&&commObjectNumber==0)		//schaltobjekt K1 aus
               {
//				portValue |= (1<<commObjectNumber);
				portValue |= 1;
				K1dimmwert=0;
			}

			//KANAL2

			if(data==0&&commObjectNumber==1)	//schaltobjekt K2 AUS
               {
				portValue |= 2;
				K2dimmwert=0;
			}
			if(data==1&&commObjectNumber==1)	//schaltobjekt K2 EIN
               {
				portValue &= ~2;
				K2dimmwert=hellikeit[einschathellikeit_k2];
			}
			if(commObjectNumber==3)			//Dimmobjekt K2
               {
				gK2helldunkel=data;
				portValue |= 2;
               }
			if(commObjectNumber==5)			//Wertobjekt K2
				K2dimmwert = rxmsg->data[8];


                   //sendRespondTelegram(i,(portValue & (1<<i))?1:0, 0x0C);
          } else if(commObjectNumber > 7 && commObjectNumber < 12)
		{
			// additinal function
			/** @todo write part additional functions */
//               DEBUG_PUTS("ZF");
//               DEBUG_NEWLINE();
		}

     }
//     switchObjects();
	return FB_ACK;
}


/**
* Switch the objects to state in portValue and save value to eeprom if necessary.
*
*
void switchObjects(void)
{
//ri     uint16_t initialPortValue;
     uint8_t portOperationMode=read_memory(0x01F2);      ///< defines if IO is closer or opener, see address 0x01F2 in eeprom
//ri      uint8_t i;

     DEBUG_PUTS("Sw");

     // check if timer is active on the commObjectNumber

     // read saved status and check if it was changed
    uint8_t savedValue=read_memory(0x0000);
     if(savedValue != portValue)
     {
		// now check if last status must be saved, we write to eeprom only if necessary
		initialPortValue= (read_memory(0x01F7) << 8) | (read_memory(0x01F6));
		for(i=0;i<=7;i++)
		{
			if(((initialPortValue>>(i*2))&0x03)==0x0)
			{
				write_eeprom_block(0x0000,1,&portValue);
				DEBUG_PUTS("Sv");
				break;
			}
		}
     }
     *
     // check 0x01F2 for opener or closer and modify data to relect that, then switch the port
     switchPorts(portValue^portOperationMode);
}
*/
unsigned char i2c_wait(void)
	{
	unsigned int timeueberlauf=0;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)))
		{
		timeueberlauf++;
		if(timeueberlauf>MAXZEIT)
			return 0xff;
		}
	return 0;
	}

/** 
* @todo add documentation
* 
* @param daten 
* 
* @return 
*/unsigned char i2c_send(unsigned char daten)
	{
	unsigned char err=0;
	TWDR = daten;
	err=i2c_wait();
	//if ((TWSR & 0xF8) != TW_MT_SLA_ACK) 	//error
	//	err=TW_MT_SLA_ACK;
	return err;
	}

/** 
* @todo add documentation
* 
* 
* @return 
*/unsigned char i2c_send_daten(void)
	{
	unsigned char err=0;

	TWBR=0x20;//bitrate =100000 bit
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)|(1<<TWEA);	//send start MASTER
	err=i2c_wait();
	if(err==0) 	err=i2c_send(0xa0);	
	if(err==0) 	err=i2c_send(0x00);
	if(err==0) 	err=i2c_send(K1dimmwert_ausgang);
	if(err==0) 	err=i2c_send(K2dimmwert_ausgang);
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO)|(1<<TWEA);	//stop senden
	return 0;
	}


/**
* The start point of the program, init all libraries, start the bus interface, the application
* and check the status of the program button.
*
* @return
*/
int main(void)
{
     uint8_t i;
	unsigned int ie=0;
	sei();                             // enable interrupts
 	msg_queue_init();

	DEBUG_INIT();
//   DEBUG_PUTS("V0.02\n\r");

	fb_prot_init();
	restartApplication();
	// here we have the main loop, check prog button and run the application
	while(1)
	{
         checkProgTaster();
          // check if 130ms timer is ready
          if(TIMER1_OVERRUN)
          {
               CLEAR_TIMER1_OVERRUN;
               timerOverflowFunction();
          }

          for(i=0;i<200;i++)
               _NOP();
		//sleep_mode();

		if(zldurch2<100) //Geschwindikeit zum heller oder dunler dimmen
			zldurch2++;
		else
			{
			if(ie<4000)
				++ie;
			else
				{
				i2c_send_daten();
				ie=0;
				}

			if(anspringen_k1)
				K1dimmwert_ausgang=K1dimmwert;	//anspringen vom wert
			else
			{
				if(K1dimmwert_ausgang > K1dimmwert)
					K1dimmwert_ausgang-=1;
				if(K1dimmwert_ausgang < K1dimmwert)
					K1dimmwert_ausgang+=1;
			}

			if(anspringen_k2)
				K2dimmwert_ausgang=K2dimmwert;	//anspringen vom wert
			else
			{
				if(K2dimmwert_ausgang > K2dimmwert)
					K2dimmwert_ausgang-=1;
				if(K2dimmwert_ausgang < K2dimmwert)
					K2dimmwert_ausgang+=1;
			}
		zldurch2=0;
		if(K1dimmwert_ausgang!=mk1||K2dimmwert_ausgang!=mk2)
			{
			mk1=K1dimmwert_ausgang;
			mk2=K2dimmwert_ausgang;
			i2c_send_daten();
			}
		}
	}
	return 0;
}
