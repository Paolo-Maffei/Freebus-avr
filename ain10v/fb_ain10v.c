/* $Id$ */
/*
 *      __________  ________________  __  _______
 *     / ____/ __ \/ ____/ ____/ __ )/ / / / ___/
 *    / /_  / /_/ / __/ / __/ / __  / / / /\__ \ 
 *   / __/ / _, _/ /___/ /___/ /_/ / /_/ /___/ / 
 *  /_/   /_/ |_/_____/_____/_____/\____//____/  
 *                                      
 *  Copyright (c) 2010 Matthias Fechner <matthias@fechner.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
/**
 * @file   fb_ain10v.c
 * @author Gerald Eichler
 * @date   Sun Aug 08 08:01:58 2010
 * 
 * @brief  
 * to use the 09609110.vd1 Analog-Sensorschnittstelle 4-fach
 * at the moment only for 0-10V !!
 */
#ifndef _FB_ADC_C
#define _FB_ADC_C


/*************************************************************************
 * INCLUDES
 *************************************************************************/
#include "fb.h"
#include "fb_hardware.h"
#include "freebus-debug.h"
#include "fb_eeprom.h"
#include "msg_queue.h"
#include "fb_hal.h"
#include "fb_prot.h"
#include "fb_app.h"
#include "fb_ain10v.h"
#include <avr/sleep.h>

/**************************************************************************
 * DEFINITIONS
 **************************************************************************/
/** Reset the internal variables used for the application timer and reload the timer itself
 * @todo check if move of currentTime to this function really does not introduce a bug
 */
#define RESET_RELOAD_APPLICATION_TIMER() {      \
        currentTimeOverflowBuffer=0;            \
        currentTime=0;                          \
        RELOAD_APPLICATION_TIMER();             \
    }

/**************************************************************************
 * DECLARATIONS
 **************************************************************************/
extern struct grp_addr_s grp_addr;

static uint32_t currentTime;              /**< defines the current time in 10ms steps (2=20ms) */
volatile uint16_t value = 0;    /**< the value from the ADC */
uint16_t channelValue[4],lastValue[4],lastsentValue[4];
uint8_t limit[4];
volatile uint8_t newValue = 0;    /**< to tell the programm, that new data is here !! */
uint8_t zyk_senden_basis;
uint8_t channelIndex = 0;
unsigned char sende_sofort_bus_return;
unsigned char delrec[36];
uint8_t cycle = 0;


uint8_t nodeParam[EEPROM_SIZE];           /**< parameterstructure (RAM) */

/** list of the default parameter for this application */
const STRUCT_DEFPARAM defaultParam[] PROGMEM =
    {
        { SOFTWARE_VERSION_NUMBER, 0x01 },    /**< version number                               */
        { APPLICATION_RUN_STATUS,  0xFF },    /**< Run-Status (00=stop FF=run)                  */
        { COMMSTAB_ADDRESS,        0x3A },    /**< COMMSTAB Pointer                             */
        { APPLICATION_PROGRAMM,    0x00 },    /**< Port A Direction Bit Setting???              */

        { 0x0000,                  0x00 },    /**< default is off                               */
        { 0x01EA,                  0x00 },    /**< no timer active                              */
        { 0x01F6,                  0x55 },    /**< don't save status at power loss (number 1-4) */
        { 0x01F7,                  0x55 },    /**< don't save status at power loss (number 5-8) */
        { 0x01F2,                  0x00 },    /**< closer mode for all relais                   */

        { MANUFACTORER_ADR,        0x08 },    /**< Herstellercode 0x08 = Giera                  */
        { DEVICE_NUMBER_HIGH,      0xB0 },    /**< Devicetype 0xB003 = GIRA Analogeingang 960 00  */
        { DEVICE_NUMBER_LOW,       0x0A },    /**<                                              */

        { 0xFF,                    0xFF }     /**< END-sign; do not change                      */
    };


/*************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/
void timerOverflowFunction(void);
void configureAdc(void);
inline void doAdcMeasurement(uint8_t index);
void sendData(uint16_t wGA,uint8_t *data,uint8_t dataLen,uint8_t sendType);
unsigned int find_ga(unsigned char objno);
void write_delay_record(unsigned char objno, unsigned char delay_state, long delay_target);
void clear_delay_record(unsigned char objno);
void send_value(unsigned char type, unsigned char objno,int16_t sval);
unsigned char read_obj_type(unsigned char objno);
int16_t sendewert(unsigned char objno);
void bus_return(uint8_t channel);
void grenzwert (unsigned char eingang);
void messwert (unsigned char eingang);

/**************************************************************************
 * IMPLEMENTATION
 **************************************************************************/

/**
* bringt den Messwert ins Sendeformat 8 Bit oder 16 Bit
*
* \param  objno
*
* @return sendewert
*/
int16_t sendewert(unsigned char objno)
{
	
	uint8_t objno_help=objno>>1;
	uint8_t mul = mem_ReadByte(0x0184 + (objno_help*9));
	int32_t value = (int32_t)channelValue[objno_help];

	int16_t min = (mem_ReadByte(0x017C + 2 + (objno_help*9)) << 8) + mem_ReadByte(0x017C + 3 + (objno_help*9));
	int16_t max = (mem_ReadByte(0x0180 + 2 + (objno_help*9)) << 8) + mem_ReadByte(0x0180 + 3 + (objno_help*9));
	value = (((int32_t)max-(int32_t)min)*value) >> 10; // >> ... /1024
	value += (int32_t)min;
	// Sendeformat 8Bit
	if ((mem_ReadByte(0x01A4)>>4)&(1<<objno_help))
	{
		return value;
	}

	// Sendeformat 16 Bit
	else
	{
		/*
			0 ... * 0.01
			1 ... * 0.1
			2 ... * 1
			3 ... * 10
			4 ... * 100

		*/
		if (mul==0){
			value /= 100;
		}else if (mul==1){
			value /= 10;
		}else if (mul==2){
			value *= 1;
		}else if (mul==3){
			value *= 10;
		}else if (mul==4){
			value *= 100;
		}		
		return (int16_t)value;
	}
}

/**
*
*
* \param objno
*
* @return gibt den Typ eines Objektes zurueck
*/
unsigned char read_obj_type(unsigned char objno)
{
	unsigned char  commstab, objtype;
	
	objtype=0xFF;
	commstab=mem_ReadByte(COMMSTAB_ADDRESS);	
	if (objno <= commstab) {	// wenn objno <= anzahl objekte
		objtype=mem_ReadByte(BASE_ADDRESS_OFFSET+commstab+objno*3+4);
	}
	return(objtype);
}



/**
* sucht Gruppenadresse für das Objekt objno uns sendet ein EIS Telegramm
*
* \param  type
* \param  objno
* \param  sval
*
* @return void
*/
void send_value(unsigned char type, unsigned char objno,int16_t sval)
{
	unsigned int ga;
	unsigned char objtype;
	uint8_t sendOk = 0;
	uint8_t len = 0;
	uint8_t data[4];
	//int16_t sval;
	//check if input configured
	if (((mem_ReadByte(0x016B+((objno>>2)&0x01)))<<(4*((objno>>1)&0x01))&0xF0)!=0x70) 
	{ 
		//DEBUG_PUTS("sendeZyklisch ");
		//DEBUG_PUTHEX(objno);
		//DEBUG_NEWLINE();							
		//ok input configured		
		ga=find_ga(objno);					// wenn keine Gruppenadresse hintrlegt nix tun
		if (ga!=0)
		{
			//sval = sendewert(objno);
			objtype=read_obj_type(objno);			
			//DEBUG_PUTS("objtype= ");
			//DEBUG_PUTHEX(objtype);
			//DEBUG_NEWLINE();							
			if(objtype<=5)			// Objekttyp, 1-6 Bit
			{
				len = 1;				
				data[0]=sval;
				sendOk = 1;
			}
			else if(objtype<=7)		// Objekttyp, 7-8 Bit
			{
				len = 2;				
				data[0]=sval;
				sendOk = 1;
			}
			else if(objtype<=8)		// Objekttyp, 16 Bit
			{
				len = 3;				
				data[0]=sval>>8;
				data[1]=sval;
				sendOk = 1;
			}
			if (sendOk){
				if (type == 0){			
					sendData(ga,&data[0],len,0); //read_value_request			
				}else{
					
					sendData(ga,&data[0],len,2); //write_value_request
				}
			}
		}
	}
}

/** 
* Schreibt die Schalt-Verzoegerungswerte
*
*
*
*/
void write_delay_record(unsigned char objno, unsigned char delay_state, long delay_target)
{
	delrec[objno*4]=delay_state;
	delrec[objno*4+1]=delay_target>>16;
	delrec[objno*4+2]=delay_target>>8;
	delrec[objno*4+3]=delay_target;
}


/** 
* loescht die Schalt-Verzoegerungswerte
*
*
*
*/
void clear_delay_record(unsigned char objno)
{
	delrec[objno*4]=0;
	delrec[objno*4+1]=0;
	delrec[objno*4+2]=0;
	delrec[objno*4+3]=0;
}


void sendData(uint16_t wGA,uint8_t *data,uint8_t dataLen,uint8_t sendType)
{
    struct msg * resp = AllocMsgI();
    int i = 0;

    if (!resp)
    {
        return;
    }

    struct fbus_hdr * hdr= (struct fbus_hdr *) resp->data;

    resp->repeat = 3;
    resp->len    = 8 + dataLen;
    hdr->ctrl    = 0xBC; //we are sending always with normal priority
    hdr->src[0]  = mem_ReadByte(PA_ADDRESS_HIGH);
    hdr->src[1]  = mem_ReadByte(PA_ADDRESS_LOW);
    hdr->dest[0] = (uint8_t)(wGA >> 8);
    hdr->dest[1] = (uint8_t)(wGA & 0x00FF);
    hdr->npci    = 0xe0 + dataLen;
    hdr->tpci = 0x00;
    if (sendType == 0)
    {
    	hdr->apci    = 0x00; //read request
    }else if (sendType == 1)
    {
    	hdr->apci    = 0x40; // reponse
    }else if (sendType == 2)
    {
    	hdr->apci    = 0x80;//write request
    }
    if (dataLen == 1){
	hdr->apci += *data;

    } else if (dataLen > 1){
    	for (i = 0;i < dataLen; i++)
    	{
    		resp->data[8 + i] = *data;
    		data++;
    	}
    }

    fb_hal_txqueue_msg(resp);

    return;
}
/**
* Gruppenadresse ueber Assoziationstabelle finden
*
* Die sendende Adresse ist diejenige, bei der die Objektnummer
* und die Assoziationsnummer uebereinstimmt
*
* \param objno
*
* @return
*/

unsigned int find_ga(unsigned char objno)
{
	uint8_t gaindex;
     uint8_t assocTabPtr = mem_ReadByte(ASSOCTABPTR);                             // points to start of association table (0x0100+assocTabPtr)
	gaindex = mem_ReadByte(BASE_ADDRESS_OFFSET + assocTabPtr + 1 + (objno*2));     	
	// check if valid group address reference
    	if(gaindex == 0xFE){
		return 0;
	}else{
		return grp_addr.ga[gaindex-1];
	}
}



/** 
 * Timer1 is used as application timer. It increase the variable currentTime every 130ms and currentTimeOverflow if
 * currentTime runs over 16-bit.
 * 
 * @return 
 * @todo test interrupt lock in this function that it is not disturbing TX and RX of telegrams
 */
void timerOverflowFunction(void)
{
	uint8_t objno,zyk_faktor,delay_state,objno_help,n;
	uint32_t delval,zyk_val;
	/* check if programm is running */
	if(mem_ReadByte(APPLICATION_RUN_STATUS) != 0xFF)
		return;
	currentTime++;
	currentTime&=0x00FFFFFF;
		for(objno=0;objno<=8;objno++) {
			delay_state=delrec[objno*4];
			if(delay_state!=0x00) {			// 0x00 = delay Eintrag ist leer
				delval=delrec[objno*4+1];
				delval=(delval<<8)+delrec[objno*4+2];
				delval=(delval<<8)+delrec[objno*4+3];
				if(delval==currentTime) {

					if (objno<=3)	// Zyklisch Senden Eingänge Messwert und Grenzwert
					{
						zyk_faktor=mem_ReadByte(0x0161+objno)&0x7F;

						zyk_val=(zyk_faktor<<zyk_senden_basis);

						zyk_val=zyk_val+currentTime;

						write_delay_record(objno,delay_state,zyk_val);

						/*
						DEBUG_PUTS_BLOCKING("T ");
						DEBUG_PUTHEX_BLOCKING(currentTime >> 16);		
						DEBUG_PUTHEX_BLOCKING(currentTime >> 8);		
						DEBUG_PUTHEX_BLOCKING(currentTime);		
						DEBUG_PUTS_BLOCKING(" D ");
						DEBUG_PUTHEX_BLOCKING(delrec[objno*4+1]);		
						DEBUG_PUTHEX_BLOCKING(delrec[objno*4+2]);		
						DEBUG_PUTHEX_BLOCKING(delrec[objno*4+3]);		
						DEBUG_NEWLINE_BLOCKING();
						*/
						if ((delay_state&0x80) && (sende_sofort_bus_return==0))	// Messwert zyk senden
						{
							send_value(1,(objno<<1),sendewert(objno<<1));
							if (delay_state&0x01)	// Grenzwert zyk senden
							{
								send_value(1,((objno<<1)+1),limit[objno]);
							}
						}
					}
					else if (objno<=7)	// Sendeverzögerung Eingänge Messwerte
					{
						objno_help=objno-4;

						send_value(1,(objno_help<<1),sendewert(objno_help<<1));
						lastsentValue[objno_help]=channelValue[objno_help];

						clear_delay_record(objno);
					}

					else	// Sendeverzögerung Eingänge Messwerte Busspannungswiederkehr
					{
						for (n=0;n<=6;n=n+2)
						{
							if (delay_state&(0x40>>n))
							{
								send_value(1,n,sendewert(n));								
							}
						}
						delrec[8*4]=0;
					}
				}
			}
		}
	/*
	cycle++;
	if (cycle == 10){
		DEBUG_PUTS_BLOCKING("Z ");
		DEBUG_PUTHEX_BLOCKING(currentTime >> 16);		
		DEBUG_PUTHEX_BLOCKING(currentTime >> 8);		
		DEBUG_PUTHEX_BLOCKING(currentTime);		
		DEBUG_NEWLINE_BLOCKING();
		cycle = 0;
	}
	*/
    return;
}

/**
* Verhalten bei Busspannungswiederkehr
*	sofortiges Senden der Messwerte und Grenzwerte bei Busspannungswiederkehr
*
* \param  void
*
* @return void
*/
void bus_return(uint8_t channel)
{
	uint8_t kanal_help;

	kanal_help=channel<<1;
	//send channels
	if (sende_sofort_bus_return&(0x80>>kanal_help))
	{
		send_value(1,kanal_help,sendewert(kanal_help));
		sende_sofort_bus_return&=0xFF-(0x80>>kanal_help);
	}
	//send limits
	if (sende_sofort_bus_return&(0x40>>kanal_help))
	{
		DEBUG_PUTS("sg");
		DEBUG_PUTHEX(channel);
		DEBUG_NEWLINE();		
		send_value(1,((channel<<1)+1),limit[channel]);
		sende_sofort_bus_return&=0xFF-(0x40>>kanal_help);
	}


}

/**
* Senden bei Grenzwertüber- bzw. unterschreitung
*	überprüft die Grenzwerte
*	schreibt die Objektwerte und sendet Telegramm
*
* \param  eingang
*
* @return void
*/
void grenzwert (unsigned char eingang)
{
	uint16_t schwelle1, schwelle2;
	unsigned char reaktion, wert, objno;

	objno=(eingang<<1)+1;

	reaktion=mem_ReadByte(0x16D+eingang);

	schwelle1=(uint16_t)(1024*(uint32_t)(mem_ReadByte(0x171+eingang)&0x7F)/100);
	schwelle2=(uint16_t)(1024*(uint32_t)(mem_ReadByte(0x175+eingang)&0x7F)/100);

	//steigend
	if ((lastValue[eingang]<schwelle2 || sende_sofort_bus_return) && channelValue[eingang]>schwelle2)	// GW 2 überschritten
	{
		if (reaktion&0x0C)
		{
			wert=(reaktion>>2)&0x01;
			limit[eingang]=wert;
			//write_obj_value(objno,wert);
			if(!sende_sofort_bus_return)
			{
				send_value(1,objno,wert);
			}
		}
	}

	if ((lastValue[eingang]<schwelle1 || sende_sofort_bus_return) && channelValue[eingang]>schwelle1)	// GW 1 überschritten
	{
		if (reaktion&0xC0)
		{
			wert=(reaktion>>6)&0x01;
			limit[eingang]=wert;
			//write_obj_value(objno,wert);
			if(!sende_sofort_bus_return)
			{
				send_value(1,objno,wert);
			}
		}
	}


	//fallend
	if ((lastValue[eingang]>schwelle1 || sende_sofort_bus_return) && channelValue[eingang]<schwelle1)	// GW 1 unterschritten
	{
		if (reaktion&0x30)
		{
			wert=(reaktion>>4)&0x01;
			limit[eingang]=wert;
			//write_obj_value(objno,wert);
			if(!sende_sofort_bus_return)
			{
				send_value(1,objno,wert);
			}
		}
	}

	if ((lastValue[eingang]>schwelle2 || sende_sofort_bus_return) && channelValue[eingang]<schwelle2)	// GW 2 unterschritten
	{
		if (reaktion&0x03)
		{
			wert=reaktion&0x01;
			limit[eingang]=wert;
			//write_obj_value(objno,wert);
			if(!sende_sofort_bus_return)
			{
				send_value(1,objno,wert);
			}
		}
	}


	lastValue[eingang]=channelValue[eingang];
}

/**
* Senden bei Messwertdifferenz
*	überprüft die Messwertdifferenz
*	schreibt die Verzögerungszeit ins delrec
*
* \param  eingang
*
* @return void
*/
void messwert (unsigned char eingang)
{
	unsigned int mess_diff;
	int mess_change;
	unsigned long zyk_val;

	unsigned char zykval_help;

	if (mem_ReadByte(0x165+eingang)&0x80)
	{
		mess_diff=(uint16_t)(1024*(uint32_t)(mem_ReadByte(0x165+eingang)&0x7F)/100);		
		//mess_diff=180*(mem_ReadByte(0x165+eingang)&0x7F);

		if (channelValue[eingang]<=lastsentValue[eingang])
		{
			mess_change=lastsentValue[eingang]-channelValue[eingang];
		}
		else
		{
			mess_change=channelValue[eingang]-lastsentValue[eingang];
		}

//		if (mess_change<0) mess_change=0-mess_change;

		if(mess_change>mess_diff)
		{
			if (delrec[(eingang+4)*4]==0)
			{
				zykval_help=(mem_ReadByte(0x169+(eingang>>1)))>>(4*(!(eingang&0x01)))&0x0F;

				if (zykval_help<=5)
				{
					zyk_val=zykval_help*8;
				}
				else if (zykval_help<=10)
				{
					zyk_val=(zykval_help-5)*77;
				}
				else
				{
					zyk_val=(zykval_help-10)*462;
				}

				zyk_val=zyk_val+currentTime+1;

				write_delay_record((eingang+4),1,zyk_val);
			}
		}
		else
		{
			clear_delay_record(eingang+4);
		}
	}
}



/** 
 * Function is called when microcontroller gets power or if the application must be restarted.
 * It restores data like in the parameters defined.
 * 
 * @return FB_ACK or FB_NACK
 */
uint8_t restartApplication(void)
{
	unsigned char zyk_funk, n;
	unsigned int sendeverzoegerung;
    currentTime=0;
    //currentTimeOverflow=0;

	// Zeit für Sendeverzögerung bei Busspannungswiederkehr ins delrec schreiben
	sendeverzoegerung=mem_ReadByte(0x01A0)<<3;
	delrec[8*4+2]=sendeverzoegerung>>8;
	delrec[8*4+3]=sendeverzoegerung;
	/*
     DEBUG_PUTS("sendeverz= ");
     DEBUG_PUTHEX(sendeverzoegerung>>8);
     DEBUG_PUTHEX(sendeverzoegerung);
     DEBUG_NEWLINE();
	*/
	// Verhalten bei Busspannungswiederkehr Messwewrte
	sende_sofort_bus_return=mem_ReadByte(0x0179)&0xAA;
	delrec[8*4]=mem_ReadByte(0x0179)&0x55;

    zyk_senden_basis = mem_ReadByte(0x0160)&0x0F;     	
    /*
    DEBUG_PUTS("basis= ");
    DEBUG_PUTHEX(zyk_senden_basis);
    DEBUG_NEWLINE();
    */
	// Schleife für alle Eingänge
	for(n=0;n<=3;n++)
	{
		// Verhalten bei Busspannungswiederkehr Grenzwerte
		sende_sofort_bus_return|=(mem_ReadByte(0x0171+n)&0x80)>>(2*n+1);

		// Bedingungen für zyklisch senden ins delrec schreiben
		zyk_funk=(mem_ReadByte(0x0161+n)&0x80);
		zyk_funk=zyk_funk+((mem_ReadByte(0x0175+n)>>7)&0x01);
		write_delay_record(n,zyk_funk,0x01);


		// Werte zurücksetzen
		channelValue[n]=0;
	     lastValue[n] = 0;
		limit[n] = 0;
		lastsentValue[n] = 0;
	}
    configureAdc();
    channelIndex = 0;
    newValue = 0;
    doAdcMeasurement(channelIndex); // start new measurement
    /* enable timer to increase user timer used for timer functions etc. */
    RELOAD_APPLICATION_TIMER();
    return 1;
} /* restartApplication() */

/** 
 * Read status from port and return it.
 * 
 * @param rxmsg 
 * 
 * @return 
 */
uint8_t readApplication(struct msg *rxmsg)
{
    struct fbus_hdr *hdr =( struct fbus_hdr *) rxmsg->data;
	unsigned char objtype;
	uint8_t sendOk = 0;
	uint8_t len = 0;
	uint8_t data[4];
	int16_t sval;
    DEBUG_PUTS("Read");

    uint8_t i;
    uint16_t destAddr = ((uint16_t)(hdr->dest[0])<<8) | (hdr->dest[1]);

    uint8_t assocTabPtr;            // points to start of association table (0x0100+assocTabPtr)
    uint8_t countAssociations;      // number of associations saved in associations table
    uint8_t numberInGroupAddress;   // reference from association table to group address table
    uint8_t commObjectNumber;       // reference from association table to communication object table

    assocTabPtr = mem_ReadByte(ASSOCTABPTR);
    countAssociations = mem_ReadByte(BASE_ADDRESS_OFFSET + assocTabPtr);
 
    for(i=0; i<countAssociations; i++) {
        numberInGroupAddress = mem_ReadByte(BASE_ADDRESS_OFFSET + assocTabPtr + 1 + (i*2));

        // check if valid group address reference
        if(numberInGroupAddress == 0xFE)
            continue;

        commObjectNumber = mem_ReadByte(BASE_ADDRESS_OFFSET + assocTabPtr + 1 + (i*2) + 1);

        // now check if received address is equal with the safed group addresses, substract one
        // because 0 is the physical address, check also if commObjectNumber is between 0 and 7
        // (commObjectNumber is uint8_t so cannot be negative don't need to check if >= 0)
        if(destAddr == grp_addr.ga[numberInGroupAddress-1]){
		// Messwerte Objekte 0,2,4,6
		if((commObjectNumber&0x01)==0)
		{
			sval = sendewert(commObjectNumber);
			objtype=read_obj_type(commObjectNumber);	
			sendOk = 0;		
			if(objtype<=5)			// Objekttyp, 1-6 Bit
			{
				len = 1;				
				data[0]=sval;
				sendOk = 1;
			}
			else if(objtype<=7)		// Objekttyp, 7-8 Bit
			{
				len = 2;				
				data[0]=sval;
				sendOk = 1;
			}
			else if(objtype<=8)		// Objekttyp, 16 Bit
			{
				len = 3;				
				data[0]=sval>>8;
				data[1]=sval;
				sendOk = 1;
			}
			if (sendOk){
				sendData(destAddr,&data[0],len,1); //response			
			}

		}
		// Grenzwerte Objekte 1,3,5,7
		else
		{
			sval = limit[commObjectNumber>>1];
			objtype=read_obj_type(commObjectNumber);	
			sendOk = 0;		
			if(objtype<=5)			// Objekttyp, 1-6 Bit
			{
				len = 1;				
				data[0]=sval;
				sendOk = 1;
			}
			else if(objtype<=7)		// Objekttyp, 7-8 Bit
			{
				len = 2;				
				data[0]=sval;
				sendOk = 1;
			}
			else if(objtype<=8)		// Objekttyp, 16 Bit
			{
				len = 3;				
				data[0]=sval>>8;
				data[1]=sval;
				sendOk = 1;
			}
			if (sendOk){
				sendData(destAddr,&data[0],len,1); //response			
			}
			
		}
		break;
	   }
    }
    return FB_ACK;
}   /* readApplication() */

/** 
 * Function is called if A_GroupValue_Write is received. The type it is the function "EIS1" or "Data Type Boolean" for the relais module.
 * Read all parameters in that function and set global variables.
 *
 * @param rxmsg 
 * 
 * @return The return value defies if a ACK or a NACK should be sent (FB_ACK, FB_NACK)
 */
uint8_t runApplication(struct msg *rxmsg)
{
    return FB_ACK;
}   /* runApplication() */

/**                                                                       
 * The start point of the program, init all libraries, start the bus interface,
 * the application and check the status of the program button.
 *
 * @return 
 *   
 */
int main(void)
{
    //uint8_t data[4];
    /* disable wd after restart_app via watchdog */
    DISABLE_WATCHDOG();

    /* ROM-Check */
    /** @todo Funktion fuer CRC-Check bei PowerOn fehlt noch */
    
    /* init internal Message System */
    msg_queue_init();
    
    DEBUG_INIT();
    DEBUG_NEWLINE_BLOCKING();
    DEBUG_PUTS_BLOCKING("V0.1");
    DEBUG_NEWLINE_BLOCKING();
       
    /* init procerssor register */
    fbhal_Init();

    /* enable interrupts */
    ENABLE_ALL_INTERRUPTS();

    /* init eeprom modul and RAM structure */ 
    eeprom_Init(&nodeParam[0], EEPROM_SIZE);

    /* init protocol layer */
    /* load default values */
    fbprot_Init(defaultParam);

    /* config application hardware */
    (void)restartApplication();


    /***************************/
    /* the main loop / polling */
    /***************************/
    while(1) {
          /* Auswerten des Programmiertasters */
		if(fbhal_checkProgTaster()) {

		}
		if(TIMER1_OVERRUN) {
			CLEAR_TIMER1_OVERRUN;
		     timerOverflowFunction();
			//we convert only all 130ms 1 time --> 520ms for one cycle !! should be enough !!			
			if (newValue){
			   channelValue[channelIndex] = value;
			   // Grenzwerte
			   grenzwert(channelIndex);

			   // Messwertdifferenz
			   messwert(channelIndex);

			   // Buswiederkehr bearbeiten
			   if (sende_sofort_bus_return){
   				bus_return(channelIndex);
			   }
				/*
			   DEBUG_PUTS("ADC ");
			   DEBUG_PUTHEX(channelIndex);
			   DEBUG_PUTS(" ");
			   DEBUG_PUTHEX(channelValue[channelIndex]>>8);
			   DEBUG_PUTHEX(channelValue[channelIndex]);
			   DEBUG_NEWLINE();
			   */
			   newValue = 0;
			   channelIndex++; //increase channel
			   channelIndex = channelIndex & 0x03; //only from 0 to 3 !!
			   doAdcMeasurement(channelIndex); // start new measurement	
			}
		}
    }   /* while(1) */
}   /* main() */

void configureAdc(void)
{
    // set reference to Avcc, right adjusted result, channel 1
    ADMUX = (1<<MUX0) | (1<<REFS0);

    // disable DIO of pin
    DIDR0 |= (1<<ADC0D) | (1<<ADC1D) | (1<<ADC2D) | (1<<ADC3D);

    // set ADC prescaler to 64, enable ADC interrupt, enable ADC
    ADCSRA = (1<<ADPS2) | (1<<ADPS1) | (1<<ADIE) | (1<<ADEN);
}

inline void doAdcMeasurement(uint8_t index)
{
    ADMUX = (1<<REFS0) + index; //set channelindex !!
    ADCSRA |= (1<<ADSC); //start measurement
}

ISR(ADC_vect)
{	
	value = ADCW;
	newValue = 1;
}
#endif /* _FB_ADC_C */
/*********************************** EOF *********************************/

