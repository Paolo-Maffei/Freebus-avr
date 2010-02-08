
#define UCSR1A UCSRA
#define UDRE1 UDRE
#define UDR1 UDR


unsigned int uart_getchar(void)
{
	if ( !(UCSR1A & (1<<RXC)) )
		return 0xffff;
	return UDR1;
}


#define XON  0x11
#define XOFF 0x13


int uart_putchar( char c)
{
	unsigned char x=0;
	if (c == '\n')
		uart_putchar('\r');
	if (c == 'ü')
		c=0x81;
	if (c == 'Ö')
		c=0x99;
	while ( !( UCSR1A & (1<<UDRE1)) )		//Wait for empty transmit buffer
		;
	if ( !(UCSR1A & (1<<RXC)) );else	 //if(XON XOFF)
		{
		x=UDR1;
		if(x==XOFF)
			{
			while(1)
				{
				if ( !(UCSR1A & (1<<RXC)) );else
					if(UDR1==XON)
					break;
				}
			}
			
		}
	
	
	UDR1=c;			//Put data into buffer, sends the data
	return 0;
}



int uart_putcharo( char c)
{
	while ( !( UCSR1A & (1<<UDRE1)) )		//Wait for empty transmit buffer
		;
	UDR1=c;			//Put data into buffer, sends the data
	return 0;
}




#define UART_SCRATCH 40


void uart_printf(char *Buffer,...)
{
	// Hilfsvariable zum Zählen der ausgegebenen Zeichen
	char nFieldWidth = 0;
	struct {
		char     fLeftJust:1;   // Feldausrichtung links oder rechts
		char     fNegative:1;   // Auszugebende Zahl ist negativ.
	} flags;
	unsigned char scratch[UART_SCRATCH];
	unsigned char format_flag;
	unsigned int u_val=0, base=0;
	unsigned char *ptr;
	char     hexA = 'a';
	unsigned char *p;
	int   n;
	int nLen;
	//for(n=0;n<UART_SCRATCH;n++)scratch[n]=0;
	va_list ap;
		
	va_start (ap, Buffer);	
	while (*Buffer != 0)
		{
		if (*Buffer == '%')
			{
			Buffer++;//*Buffer++;
			switch (format_flag = *Buffer++)
			{
 				case 'c':
				   format_flag = va_arg(ap,int);
				   uart_putchar(format_flag++);
				   
				case 'i':
				case 'd':
				case 'u':
					base = 10;
				break;//goto CONVERSION_LOOP;
				case 'o':
					base = 8;
				break;//goto CONVERSION_LOOP;
				
				case 'X':
					hexA = 'A';
					// Weiter wie 'x'
				case 'x':
					base = 16;
			}

			//CONVERSION_LOOP:
            u_val = va_arg (ap, int);
			n = u_val;
            flags.fNegative = 0;
            if (format_flag == 'd' || format_flag == 'i') 
			{
            // Negative Werte auswerten
                if (((int) u_val) < 0) 
				{
                    flags.fNegative = 1;
                    u_val = -u_val;
                }
            }
            // Der Scratchpuffer wird von rechts nach links aufgefüllt
            // beginnend mit dem niederwertigsten Digit.
            ptr = scratch + UART_SCRATCH;
            *--ptr = 0;     // Abschliessendes NULL-Byte eintragen
           do 
			{
			char ch = u_val % base + '0';
               if (ch > '9') 
				ch += hexA - '9' - 1;
			*--ptr = ch;
			u_val /= base;
               } 
		while (u_val);
				
				if (n < base) *--ptr = '0';
                if (flags.fNegative) *--ptr = '-';
			    // Länge bestimmen
			    p = ptr;
			    nLen = 0;
			    while (*p++) nLen++;
				// Feld bei Bedarf links auffüllen
				if (!flags.fLeftJust) {
				for (n=nLen; n < nFieldWidth; n++) uart_putchar(' ');
				}
				// Pufferinhalt schreiben
				for (n=0; n < nLen; n++) uart_putchar(*ptr++);
				// Feld bei Bedarf rechts auffüllen
				if (flags.fLeftJust) {
				for (n=nLen; n < nFieldWidth; n++) uart_putchar(' ');
				}				
			}				
		uart_putchar(*Buffer++);
		}
	va_end(ap);
}
