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
#ifndef _FB_1WIRE_C
#define _FB_1WIRE_C


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
#include "fb_ow_temp_lib.h"
#include "fb_ow_lib.h"
#include "fb_i2c_lib.h"

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

#define MAXSENSORS 10
#define CONVTIME 12

/** Structure for 1-wire */
struct Sensors1 {
          uint16_t ga;     /**< The group address (2x8 Bit) */
          OW_DEVICE_ID id;
          int32_t temp;
};


/** Structure for 1-wire */
struct Sensors2 {
          uint8_t count;
          uint16_t pollTime;
          uint8_t index;
          uint8_t step;
          uint8_t convTime;
          struct Sensors1 sensor[MAXSENSORS];
};

struct Sensors2 sensors;


/**************************************************************************
 * DECLARATIONS
 **************************************************************************/
extern struct grp_addr_s grp_addr;

static uint32_t currentTime;              /**< defines the current time in 10ms steps (2=20ms) */
static uint32_t currentTime2;
uint8_t zyk_senden_basis;
unsigned char sende_sofort_bus_return;
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



/** Structure for 1-wire */
/*
struct Sensors1 {
          uint16_t ga;     // The group address (2x8 Bit)
          uint8_t id[OW_ROMCODE_SIZE];
          int32_t temp_eminus4;
};
*/
/** Structure for 1-wire */
/*
struct Sensors2 {
          uint8_t count;
          struct Sensors1 sensor[MAXSENSORS];
};

struct Sensors2 sensors;
uint8_t gSensorIDs[MAXSENSORS][OW_ROMCODE_SIZE];
*/

//static uint16_t delayValues[8];           /**< save value for delays */



/*************************************************************************
 * FUNCTION PROTOTYPES
 **************************************************************************/
void timerOverflowFunction(void);
void sendData(uint16_t wGA,uint8_t *data,uint8_t dataLen,uint8_t sendType);
unsigned int find_ga(unsigned char objno);
void send_value(unsigned char type, unsigned char objno,int16_t sval);
unsigned char read_obj_type(unsigned char objno);
int16_t sendewert(unsigned char objno);
void bus_return(uint8_t channel);
void sendTemp(int32_t temp_eminus4, uint16_t ga,uint8_t type);
void uart_put_temp_eis5(int32_t tval);
static void uart_put_temp_maxres2(int32_t tval);
uint8_t DS18X20_format_from_maxres( int32_t temperaturevalue, char str[], uint8_t n);


/**************************************************************************
 * IMPLEMENTATION
 **************************************************************************/
void DS18X20_show_id_uart( OW_DEVICE_ID *id )
{
	uint8_t i;

	DEBUG_PUTS_BLOCKING( "FC:" );
	DEBUG_PUTHEX_BLOCKING(id->familyId);
	DEBUG_PUTS_BLOCKING(" ");
	if ( id->familyId == OW_DS18S20_FAMILY_CODE ) { DEBUG_PUTS_BLOCKING ("(18S)"); }
	else if ( id->familyId == OW_DS18B20_FAMILY_CODE ) { DEBUG_PUTS_BLOCKING ("(18B)"); }
	else if ( id->familyId == OW_DS1822_FAMILY_CODE ) { DEBUG_PUTS_BLOCKING ("(22)"); }
	else { DEBUG_PUTS_BLOCKING ("( ? )"); }

	DEBUG_PUTS_BLOCKING( "SN: " );
	for( i = 0; i < 6; i++ ) {
		DEBUG_PUTHEX_BLOCKING(id->serialNr[i]);
		DEBUG_PUTS_BLOCKING(" ");
	}

	DEBUG_PUTS_BLOCKING( "CRC:" );
	DEBUG_PUTHEX_BLOCKING(id->crc);
	DEBUG_PUTS_BLOCKING(" ");
}


uint8_t DS18X20_format_from_maxres( int32_t temperaturevalue, char str[], uint8_t n)
{
	uint8_t sign = 0;
	char temp[10];
	int8_t temp_loc = 0;
	uint8_t str_loc = 0;
	ldiv_t ldt;
	uint8_t ret;

	// range from -550000:-55.0000�C to 1250000:+125.0000�C -> min. 9+1 chars
	if ( n >= (9+1) && temperaturevalue > -1000000L && temperaturevalue < 10000000L ) {

		if ( temperaturevalue < 0) {
			sign = 1;
			temperaturevalue = -temperaturevalue;
		}

		do {
			ldt = ldiv( temperaturevalue, 10 );
			temp[temp_loc++] = ldt.rem + '0';
			temperaturevalue = ldt.quot;
		} while ( temperaturevalue > 0 );

		// mk 20110209
		if ((temp_loc < 4)&&(temp_loc > 1)) {
			temp[temp_loc++] = '0';
		} // mk end

		if ( sign ) {
			temp[temp_loc] = '-';
		} else {
			temp[temp_loc] = '+';
		}

		while ( temp_loc >= 0 ) {
			str[str_loc++] = temp[(uint8_t)temp_loc--];
			if ( temp_loc == 3 ) {
				str[str_loc++] = '.';
			}
		}
		str[str_loc] = '\0';

		ret = 0x00;
	} else {
		ret = 0x01;
	}

	return ret;
}


static void uart_put_temp_maxres2(int32_t tval)
{
	char s[10];
	uint8_t i = 0;
	DS18X20_format_from_maxres( tval, s, 10 );
	while (s[i] != 0){
		DEBUG_PUTC(s[i]);
		i++;
	}
	//DEBUG_PUTS(";0\r\n" );
	DEBUG_PUTS("\r\n" );
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

#ifdef test
void send_value2(unsigned int ga, uint8_t val)
{
	uint8_t len = 0;
	uint8_t data[4];

	len = 1;
	data[0]=val;
	sendData(ga,&data[0],len,2); //write_value_request
}
#endif


void sendData(uint16_t wGA,uint8_t *data,uint8_t dataLen,uint8_t sendType)
{
    struct msg * resp = AllocMsgI();
    int i = 0;

    if (!resp)
    {
    	DEBUG_PUTS("Q_ERR");
    	DEBUG_NEWLINE();
    	return;
    }

    struct fbus_hdr * hdr= (struct fbus_hdr *) resp->data;

    resp->repeat = 3;
    resp->len    = 8 + dataLen;
    hdr->ctrl    = 0xBC; //we are sending always with normal priority
    hdr->src[0]  = mem_ReadByte(PA_ADDRESS_HIGH);
    hdr->src[1]  = mem_ReadByte(PA_ADDRESS_LOW);
    //hdr->src[0]  = 0x00;
    //hdr->src[1]  = 0xFF;
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
    //DEBUG_PUTS("insert Data to queue\r\n");
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
	/* check if programm is running */
	if(mem_ReadByte(APPLICATION_RUN_STATUS) != 0xFF)
		return;
	currentTime++;
	currentTime&=0x00FFFFFF;
/*
	if (currentTime == 1){
		DEBUG_PUTS("start Temp-Conversion");
		DEBUG_NEWLINE();
		uint8_t ret = ow_tempStartConversion(&sensors.sensor[0].id);
	    DEBUG_PUTS("start_conv=");
	    DEBUG_PUTHEX(ret);
	    DEBUG_NEWLINE();
	}else if (currentTime == CONVTIME){
		uint8_t ret = ow_tempReadTemperature2(&sensors.sensor[0].id,&sensors.sensor[0].temp);
	    DEBUG_PUTS("read Temp=");
	    DEBUG_PUTHEX(ret);
	    DEBUG_NEWLINE();
	    uart_put_temp_maxres2(sensors.sensor[0].temp);
	}
*/
	if (sensors.count > 0){
		if (currentTime2 == sensors.pollTime){
			currentTime2 = 0;
			sensors.index = 0;
			sensors.step = 1;
		}else{
			currentTime2++;
		}
		if (sensors.step == 1){
			uint8_t ret = ow_tempStartConversion(&sensors.sensor[sensors.index].id);
			if (ret == 0){
				//ret = ow_StrongPullup(1);
				if (ret){
					DEBUG_PUTS("F #4\r\n");
				}else{
					sensors.convTime = 0;
					sensors.step = 2;
				}
			}else{
				DEBUG_PUTS("F #3 ");
				DEBUG_PUTHEX(ret);
				DEBUG_PUTS("\r\n");
			}
		}else if (sensors.step == 2){
			sensors.convTime ++;
			if (sensors.convTime == CONVTIME){
				uint8_t ret;
				ret = ow_StrongPullup(0);
				if (ret == 0){
					//ret = ow_tempReadTemperature2(&sensors.sensor[sensors.index].id,&sensors.sensor[sensors.index].temp);
					int16_t t1 = 0;
					ret = ow_tempReadTemperature(&sensors.sensor[sensors.index].id,&t1);
					sensors.sensor[sensors.index].temp = t1;
					sensors.sensor[sensors.index].temp /= 16;
					sensors.sensor[sensors.index].temp *= 10000;
					sensors.sensor[sensors.index].temp += (t1 & 0x000F) * 625;
					if (ret == 0){
						DEBUG_PUTS("#");
						DEBUG_PUTHEX(sensors.index+1);
						//DEBUG_PUTS(" ");
						//DEBUG_PUTHEX(t1 >> 8);
						//DEBUG_PUTHEX(t1 & 0xFF);
						DEBUG_PUTS(" ");
						uart_put_temp_maxres2(sensors.sensor[sensors.index].temp);
						if (sensors.sensor[sensors.index].temp != 850000){
							sendTemp(sensors.sensor[sensors.index].temp,sensors.sensor[sensors.index].ga,2);
							sensors.index++;
							if (sensors.index < sensors.count){
								//start next sensor
								sensors.step = 1;
							}else{
								//ready !!
								sensors.step = 10;
							}
						}else{
							//once again
							sensors.step = 1;
							DEBUG_PUTS("F #2\r\n");
						}
					}else{
						//once again
						sensors.step = 1;
						DEBUG_PUTS("F #1 ");
						DEBUG_PUTHEX(ret);
						DEBUG_PUTS("\r\n");
					}
				}else{
					//once again
					sensors.step = 1;
					DEBUG_PUTS("F #4\r\n");
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

}


/**
 * Function is called when microcontroller gets power or if the application must be restarted.
 * It restores data like in the parameters defined.
 *
 * @return FB_ACK or FB_NACK
 */
uint8_t restartApplication(void)
{
	uint8_t i;
	uint8_t ret;

	currentTime=0;

    DEBUG_PUTS_BLOCKING("anzGA=");
    DEBUG_PUTHEX_BLOCKING(grp_addr.count );
    DEBUG_NEWLINE_BLOCKING();
    for (i = 0; i < grp_addr.count; i++){
        DEBUG_PUTHEX_BLOCKING(grp_addr.ga[i] >> 8 );
        DEBUG_PUTHEX_BLOCKING(grp_addr.ga[i] & 0xFF );
        DEBUG_NEWLINE_BLOCKING();
    }
    i2c_Init();
    ret = ow_Init();
    DEBUG_PUTS_BLOCKING("ow_init=");
    DEBUG_PUTHEX_BLOCKING(ret);
    DEBUG_NEWLINE_BLOCKING();

    OW_SEARCH search;
    ow_SearchInit( &search);
    ret = 0;
    i = 0;
    while ((!search.lastDevice) && (ret == 0)){
    	ret = ow_Search( &search);
        DEBUG_PUTS_BLOCKING("ow_Search=");
        DEBUG_PUTHEX_BLOCKING(ret);
        DEBUG_NEWLINE_BLOCKING();
        if (ret == 0){
			DEBUG_PUTS_BLOCKING( "Sensor# " );
			DEBUG_PUTHEX_BLOCKING( i );
			DEBUG_PUTS_BLOCKING( " is a " );
			if ( search.idDevice.familyId == OW_DS18S20_FAMILY_CODE ) {
				DEBUG_PUTS_BLOCKING( "DS18S20/DS1820 " );
			} else if ( search.idDevice.familyId == OW_DS1822_FAMILY_CODE ) {
				DEBUG_PUTS_BLOCKING( "DS1822 " );
			}
			else {
				DEBUG_PUTS_BLOCKING( "DS18B20 " );
			}
			DS18X20_show_id_uart(&search.idDevice);
			DEBUG_NEWLINE_BLOCKING();
			i++;
        }
    }


    /*
    ret = ow_StrongPullup(1);
    DEBUG_PUTS_BLOCKING("ow_StrongPullup=");
    DEBUG_PUTHEX_BLOCKING(ret);
    DEBUG_NEWLINE_BLOCKING();
    */

    sensors.count = 9;
    //sensors.count = 0;
    sensors.pollTime = 461; //every 1min.
    sensors.step = 0;
    currentTime2 = sensors.pollTime;
    sensors.index = 0; //start with index 1
    //ist Temp Bad

    sensors.sensor[0].ga = 0x010A;
    sensors.sensor[0].id.familyId = 0x28;
    sensors.sensor[0].id.serialNr[0] = 0xAE;
    sensors.sensor[0].id.serialNr[1] = 0xE0;
    sensors.sensor[0].id.serialNr[2] = 0x84;
    sensors.sensor[0].id.serialNr[3] = 0x02;
    sensors.sensor[0].id.serialNr[4] = 0x00;
    sensors.sensor[0].id.serialNr[5] = 0x00;
    sensors.sensor[0].id.crc = 0x13;
    //ist Temp Vorhaus
    sensors.sensor[1].ga = 0x010B;
    sensors.sensor[1].id.familyId = 0x28;
    sensors.sensor[1].id.serialNr[0] = 0xF4;
    sensors.sensor[1].id.serialNr[1] = 0x4A;
    sensors.sensor[1].id.serialNr[2] = 0xCA;
    sensors.sensor[1].id.serialNr[3] = 0x03;
    sensors.sensor[1].id.serialNr[4] = 0x00;
    sensors.sensor[1].id.serialNr[5] = 0x00;
    sensors.sensor[1].id.crc = 0x22;
    //ist Temp Büro
    sensors.sensor[2].ga = 0x010C;
    sensors.sensor[2].id.familyId = 0x28;
    sensors.sensor[2].id.serialNr[0] = 0xF9;
    sensors.sensor[2].id.serialNr[1] = 0x3B;
    sensors.sensor[2].id.serialNr[2] = 0xCA;
    sensors.sensor[2].id.serialNr[3] = 0x03;
    sensors.sensor[2].id.serialNr[4] = 0x00;
    sensors.sensor[2].id.serialNr[5] = 0x00;
    sensors.sensor[2].id.crc = 0xC8;
    //ist Temp OG Vorhaus
    sensors.sensor[3].ga = 0x0200;
    sensors.sensor[3].id.familyId = 0x28;
    sensors.sensor[3].id.serialNr[0] = 0x1E;
    sensors.sensor[3].id.serialNr[1] = 0x31;
    sensors.sensor[3].id.serialNr[2] = 0xAF;
    sensors.sensor[3].id.serialNr[3] = 0x03;
    sensors.sensor[3].id.serialNr[4] = 0x00;
    sensors.sensor[3].id.serialNr[5] = 0x00;
    sensors.sensor[3].id.crc = 0xC8;
    //ist Temp OG Schlafzimmer
    sensors.sensor[4].ga = 0x0201;
    sensors.sensor[4].id.familyId = 0x28;
    sensors.sensor[4].id.serialNr[0] = 0x52;
    sensors.sensor[4].id.serialNr[1] = 0x5F;
    sensors.sensor[4].id.serialNr[2] = 0xCA;
    sensors.sensor[4].id.serialNr[3] = 0x03;
    sensors.sensor[4].id.serialNr[4] = 0x00;
    sensors.sensor[4].id.serialNr[5] = 0x00;
    sensors.sensor[4].id.crc = 0x62;
    //ist Temp OG Stefanie
    sensors.sensor[5].ga = 0x0202;
    sensors.sensor[5].id.familyId = 0x28;
    sensors.sensor[5].id.serialNr[0] = 0x36;
    sensors.sensor[5].id.serialNr[1] = 0x45;
    sensors.sensor[5].id.serialNr[2] = 0xAF;
    sensors.sensor[5].id.serialNr[3] = 0x03;
    sensors.sensor[5].id.serialNr[4] = 0x00;
    sensors.sensor[5].id.serialNr[5] = 0x00;
    sensors.sensor[5].id.crc = 0xAD;
    //ist Temp OG Simon
    sensors.sensor[6].ga = 0x0203;
    sensors.sensor[6].id.familyId = 0x28;
    sensors.sensor[6].id.serialNr[0] = 0x17;
    sensors.sensor[6].id.serialNr[1] = 0x4B;
    sensors.sensor[6].id.serialNr[2] = 0xAF;
    sensors.sensor[6].id.serialNr[3] = 0x03;
    sensors.sensor[6].id.serialNr[4] = 0x00;
    sensors.sensor[6].id.serialNr[5] = 0x00;
    sensors.sensor[6].id.crc = 0x8E;
    //ist Temp OG Bad
    sensors.sensor[7].ga = 0x0204;
    sensors.sensor[7].id.familyId = 0x28;
    sensors.sensor[7].id.serialNr[0] = 0xF8;
    sensors.sensor[7].id.serialNr[1] = 0x2B;
    sensors.sensor[7].id.serialNr[2] = 0xAF;
    sensors.sensor[7].id.serialNr[3] = 0x03;
    sensors.sensor[7].id.serialNr[4] = 0x00;
    sensors.sensor[7].id.serialNr[5] = 0x00;
    sensors.sensor[7].id.crc = 0x92;
    //ist Temp EDV
    sensors.sensor[8].ga = 0x0300;
    sensors.sensor[8].id.familyId = 0x28;
    sensors.sensor[8].id.serialNr[0] = 0xFA;
    sensors.sensor[8].id.serialNr[1] = 0x3B;
    sensors.sensor[8].id.serialNr[2] = 0xAF;
    sensors.sensor[8].id.serialNr[3] = 0x03;
    sensors.sensor[8].id.serialNr[4] = 0x00;
    sensors.sensor[8].id.serialNr[5] = 0x00;
    sensors.sensor[8].id.crc = 0x80;



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
/**
 * Read status from port and return it.
 *
 * @param rxmsg
 *
 * @return
 */
uint8_t readApplication(struct msg *rxmsg)
{
	//DEBUG_PUTS("RD");
	//DEBUG_NEWLINE();
	uint8_t i = 0;
    struct fbus_hdr *hdr =( struct fbus_hdr *) rxmsg->data;
    uint16_t destAddr = ((uint16_t)(hdr->dest[0])<<8) | (hdr->dest[1]);
    if (sensors.count > 0){
		for (i= 0;i < sensors.count;i++){
			if (sensors.sensor[i].ga == destAddr){
				sendTemp(sensors.sensor[i].temp,sensors.sensor[i].ga,1);
				break;
			}

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
	//DEBUG_PUTS("MSG");
	//DEBUG_NEWLINE();
	return FB_ACK;
}   /* runApplication() */

void sendTemp(int32_t temp_eminus4, uint16_t ga,uint8_t type){
	int16_t eis5temp = 0;
	uint8_t exp = 0;
	//v EEE MMM MMMM MMMM
	//<- Vorzeichen
	//  <- Exponent
	//      <- Mantisse
	int32_t tval = temp_eminus4 / 100;
	while(tval > 2047){
		tval = tval >> 1;
		exp++;
	}
	eis5temp=tval & 0x07FF;
	eis5temp=eis5temp+(exp << 11);
	if (tval < 0) eis5temp+=0x8000; //vorzeichen
	uint8_t data[2];
	data[0] = eis5temp >> 8;
	data[1] = eis5temp & 0xFF;

	//DEBUG_PUTS("EIS5=");
	//DEBUG_PUTHEX( data[0] );
	//DEBUG_PUTHEX( data[1] );
	//DEBUG_NEWLINE();
	sendData(ga,&data[0],3,type);

}


void uart_put_temp_eis5(int32_t tval){
	int16_t eis5temp = 0;
	uint8_t exp = 0;
	//v EEE MMM MMMM MMMM
	//<- Vorzeichen
	//  <- Exponent
	//      <- Mantisse

	while(tval > 2047){
		tval = tval >> 1;
		exp++;
	}
	eis5temp=tval & 0x07FF;
	eis5temp=eis5temp+(exp << 11);
	if (tval < 0) eis5temp+=0x8000; //vorzeichen
	uint8_t data[2];
	data[0] = eis5temp >> 8;
	data[1] = eis5temp & 0xFF;

	DEBUG_PUTS_BLOCKING("EIS5=");
	DEBUG_PUTHEX_BLOCKING( data[0] );
	DEBUG_PUTHEX_BLOCKING( data[1] );
	DEBUG_NEWLINE_BLOCKING();
	sendData(0x010A,&data[0],3,2);

}


int main(void)
{

	/* disable wd after restart_app via watchdog */
    DISABLE_WATCHDOG()

        /* ROM-Check */
        /** @todo Funktion fuer CRC-Check bei PowerOn fehlt noch */

        /* init internal Message System */
        msg_queue_init();

	DEBUG_INIT();
    DEBUG_NEWLINE_BLOCKING();
    DEBUG_PUTS_BLOCKING("V0.1 ");
    DEBUG_NEWLINE_BLOCKING();

    /* init eeprom modul and RAM structure already here,
       because we need eeprom values for fbrfhal_init() */
    eeprom_Init(&nodeParam[0], EEPROM_SIZE);
    //DEBUG_PUTS_BLOCKING("eeprom=");
    //DEBUG_PUTHEX_BLOCKING(mem_ReadByte(0x0200));
    //DEBUG_NEWLINE_BLOCKING();


    /* init procerssor register */
    fbhal_Init();

	/* enable interrupts */
    ENABLE_ALL_INTERRUPTS();

    /* init protocol layer */
    /* load default values */
    fbprot_Init(defaultParam);

    /* config application hardware */
    (void)restartApplication();

    /* activate watchdog */
    ENABLE_WATCHDOG ( WDTO_250MS );




    /***************************/
    /* the main loop / polling */
    /***************************/


	while(1) {
        /* calm the watchdog */
        wdt_reset();
        /* Auswerten des Programmiertasters */
        if(fbhal_checkProgTaster()) {
		}
        fbprot_msg_handler();
        /* check if 130ms timer is ready
           we use timer 1 for PWM, overflow each 100\B5sec, divide by 1300 -> 130msec. */
        if(TIMER1_OVERRUN) {
            CLEAR_TIMER1_OVERRUN;
            timerOverflowFunction();

			#ifdef test
				send_value2(2,1);
			#endif
        }
    }   /* while(1) */

}   /* main() */


#endif /* _FB_1WIRE_C */
/*********************************** EOF *********************************/
