#include "main.h"



//http://de.wikipedia.org/wiki/Europ%C3%A4ischer_Installationsbus

const char* FrameTypeNames[] = {"L-Data", "L_Poll_Data", "Acknowledge", "Unknown"};
const char* PriorityNames[] = {"System", "Urgent", "Normal", "Low"};

uint16_t CMessage::ui16Count = 0;
CMessage::CMessage ()
{
	_init ();
	this->Len = 0;
}

void CMessage::_init (void)
{
//	this->RepeatCount = 3;
	this->Next = NULL;
	this->State = MSGSTATE_FREE;
//	this->L_Data.Control = 0xB0;
}

uint16_t CMessage::GetOctet (uint8_t pos)
{
	//Ein Octet ist aufgebaut:
    // LSB
	//  0   : Startbit, immer 0
	// 1-8  : Datenbyte
	//  9   : Parity
	//  10  : Stopbit

	uint16_t tmp = 0x0400; 
	tmp |= this->Byte[pos]<<1;
	

	/* calculate parity */
	if (parity_even_bit(this->Byte[pos]))
		tmp |= 0x0200;

	return tmp;
}

void CMessage::SetOctet (uint8_t pos, uint16_t frame)
{
	/////////////////
	//do some checks
	if (pos > 23) 
		return; //More than 24 databytes are not allowed

	if ((frame & 0x0401) != 0x0400) //startbit must "0" and stopbit "1"
		return;


	uint8_t data = frame>>1;
	uint8_t parity = parity_even_bit (data);

	if (((parity == 0x01) && ((frame & 0x0200) == 0x0000)) || //Check parity
	    ((parity == 0x00) && ((frame & 0x0200) == 0x0200)))
		return;
	
	//Data may be valid
	this->Byte[pos] = data;
	return;
}

uint8_t CMessage::CalcChecksum (void)
{
	//Calculate checksum
	uint8_t checksum = 0, i = 0;
	for (; i<this->Len-1; i++)
		checksum ^=	this->Byte[i];
	return  ~checksum;	
}

uint8_t CMessage::GetChecksum (void)
{
	return Byte[Len-1];
}

//returns "1" when the message is a ACK-message, otherwise it returns "0"
uint8_t CMessage::IsACK (void)
{
	//look at 03_02_02 Communication Medium TP 1 v1.0 AS.PDF, 2.1.4 Acknowledge-Frame
	return (this->Byte[0] == EIBACK);
}

//returns "1" when the message is a NACK-message, otherwise it returns "0"
uint8_t CMessage::IsNAK (void)
{
	//look at 03_02_02 Communication Medium TP 1 v1.0 AS.PDF, 2.1.4 Acknowledge-Frame
	return (this->Byte[0] == EIBNAK);
}

uint8_t CMessage::IsBusy (void)
{
	//look at 03_02_02 Communication Medium TP 1 v1.0 AS.PDF, 2.1.4 Acknowledge-Frame
	return ((this->Byte[0] == EIBBSY) || ((this->Byte[0] == EIBNAKBSY)));
}

FRAMETYPE CMessage::GetFrameType (void)
{
	if (Byte[0] == 0b11110000)
		return L_POLL_DATA;
	else if ((Byte[0] & 0b01010011) == 0b00010000)
		return L_DATA;
	else if ((Byte[0] & 0b00110011) == 0b00000000)
		return ACKNOWLEDGE;
	else
		return UNKNOWN;
}

//dumps the content of the message to the serial interface
void CMessage::Dump (uint8_t bExtended)
{
	//print the message count
	printf_P (PSTR ("\033[30m#%05d:\t"), ++CMessage::ui16Count);

	//Check for the type of the message
	if (this->IsACK())
		puts_P (PSTR ("\033[32mACK"));
	else if (this->IsNAK())
		puts_P (PSTR ("\033[31mNACK"));
	else if (this->IsBusy())
		puts_P (PSTR ("\033[33mBusy"));
	else
	{	
		//Dump the content of the message
		for (uint8_t i = 0; i < this->Len; i++)
			printf_P (PSTR ("%02X "), this->Byte[i]);

		if (!bExtended)
		{
			puts_P (PSTR (""));
			return;
		}

		if (L_Data.DAF == 1)
			//Gruppenadresse
			printf_P (PSTR ("\n\tfrom: %d.%d.%d to %d/%d/%d"),
			          L_Data.Source.Range, L_Data.Source.Line, L_Data.Source.Subscriber,
					  L_Data.Destination.Maingroup, L_Data.Destination.Middlegroup,L_Data.Destination.Subgroup);
		else
			//Phys. Adresse
			printf_P (PSTR ("\n\tfrom: %d.%d.%d to %d.%d.%d"),
			          L_Data.Source.Range, L_Data.Source.Line, L_Data.Source.Subscriber,
					  L_Data.Destination.Range, L_Data.Destination.Line, L_Data.Destination.Subscriber);
		
		puts_P (L_Data.RepeatFlag?PSTR(""):PSTR(" (repeated)"));
		printf_P (PSTR ("\tTyp: %s  Priority: %s"), FrameTypeNames[GetFrameType()], PriorityNames [L_Data.Priority]);
		printf_P (PSTR ("  Lengths: %d/%d  Checksum: %02X\n"), Len, L_Data.Length+1, CalcChecksum());
	}
}
