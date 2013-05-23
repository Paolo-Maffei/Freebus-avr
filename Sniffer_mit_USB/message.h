
#ifndef _MESSAGE_H_
#define _MESSAGE_H_

//Conditions  of a message it can reach while its life time
enum MESSAGESTATE
{
	MSGSTATE_FREE,           //message is not used
	MSGSTATE_WAITING,		 //message is ready to send
	MSGSTATE_SENDING,        //message is marked for sending
	MSGSTATE_WAIT_ACK,       //message waits for an acknowledge
	MSGSTATE_COMPLETE,       //transmition completed
	MSGSTATE_RECEIVECOMPLETE //message completly received
};

//The possible acknowledge frames
enum EIBACKS
{
	EIBNUL = 0b00000000,
	EIBACK = 0b11001100,
	EIBNAK = 0b00001100,
	EIBBSY = 0b11000000,
	EIBNAKBSY = EIBNUL
};

//The type of the frame
enum FRAMETYPE
{
	L_DATA,
	L_POLL_DATA,
	ACKNOWLEDGE,
	UNKNOWN
};

//available priorities
enum EIBPRIORITY
{
	SYSTEM = 0b00,
	URGENT = 0b10,
	NORMAL = 0b01,
	LOW    = 0b11
};


/** 
* @todo add documentation
* 
*/
class CMessage
{
	public:
		CMessage ();
		/** @todo add documentation */
		void SetOctet (uint8_t pos, uint16_t frame);
		/** @todo add documentation */
		uint16_t GetOctet (uint8_t pos);

		CMessage* Next; ///< @todo add documentation
		uint8_t Len; ///< Gesamtlänge des Telegramms 
		MESSAGESTATE State; ///< @todo add documentation
		void Dump (uint8_t bExtended); ///< @todo add documentation
		FRAMETYPE GetFrameType (); ///< @todo add documentation
		uint8_t CalcChecksum (void); ///< @todo add documentation
		uint8_t GetChecksum (void); ///< @todo add documentation
		uint8_t IsACK (void); ///< @todo add documentation
		uint8_t IsNAK (void); ///< @todo add documentation
		uint8_t IsBusy (void); ///< @todo add documentation

		union
		{
			uint8_t Byte[23];
			//uint8_t Control;
			struct
			{
				union
				{
					uint8_t Ctrl;
					struct
					{
						unsigned MustZero_1:2;
						unsigned Priority:2;
						unsigned MustOne_1:1;
						unsigned RepeatFlag:1;
						unsigned MustZero_2:1;
						unsigned FrameType: 1;
					} __attribute__ ((packed));
				};

				union
				{
					uint16_t Value;
					struct
					{
						unsigned Line:4;
						unsigned Range:4;
						uint8_t Subscriber;
					} __attribute__ ((packed));
				} Source;
			
				union
				{
					uint16_t Value;
					struct
					{	//Only valid when the DAF is not set
						unsigned Line:4;
						unsigned Range:4;
						unsigned Subscriber:8;
					} __attribute__ ((packed));
					struct
					{	//Only valid when the DAF is set	
						unsigned Middlegroup:3;
						unsigned Maingroup:4;
						uint8_t Subgroup;	
					} __attribute__ ((packed));

				} Destination;

				union
				{
					uint8_t DRL;
					struct
					{
						unsigned Length:4;  //length (0 to 15; begins with octet 7)
						unsigned NCF:3;     //Network Control Field (Routing-counter)
						unsigned DAF:1;     //Destination-address-flag; 0: physical address; 1: group address
					} __attribute__ ((packed));
				};
				uint8_t Data[17];
			} L_Data;
		} __attribute__ ((packed)); ///< @todo add documentation


	private:
		void _init (void);
		static uint16_t ui16Count;
};

#endif //_MESSAGE_H_
