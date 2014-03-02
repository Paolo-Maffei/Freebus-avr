Stand 02.03.2014
Applikation f�r den GIRA 2 Kanal Dimmaktor 1032 00


m�gliche Ausgangsoptionen:
-DPWM8
			pulsbreitenmoduliertes Signal mit 8bit Aufl�sung an den Ausg�ngen OC2A und OC2B
			(Pin 17 und Pin 5 beim ATmega328P)
			
-DPWM10		
			pulsbreitenmoduliertes Signal mit 10bit Aufl�sung an den Ausg�ngen OC1A und OC1B
			(Pin 15 und Pin 16 beim ATmega328P)

-DUSE_UART
			Ausgabe des Helligkeitswertes �ber die serielle Schnittstelle
			mit den Parametern:   38400, 8, N, 1
			Sendeformat: highbyte hex; lowbyte hex; CR; LF
			10bit Aufl�sung; Bit15=0 -> Kanal1; Bit15=1 -> Kanal2
			
-DOUT0-10V
			Option zur Verwendung mit der Applikationsplatine 2out_0-10V_2te
			als 2-fach 0-10V oder 1-10V Ausgang
			Parameter Grundhelligkeit entspricht folgenden Ausgangsspannungen bei Helligkeitswert 1%:
			1	0,1V
			2	0,5V
			3	0,9V
			4	1,0V
			5	1,1V
			6	1,2V
			7	1,3V
			8	1,4V
 

			
Funktionen:
-Empfang & R�ckmeldung Ein/Aus Objekt
-Empfang & R�ckmeldung Helligkeitsobjekt
-Empfang Dimmen 1,5% - 100% auf- abw�rts, Dimmen Stopp
-Sperrobjekt

Parameter:
-Helligkeit bei Busspannungswiederkehr
-Helligkeit beim Einschalten
-Soft Ein
-Soft Aus
-Helligkeitswert andimmen/anspringen
-Dimmgeschwindigkeit
-Ausschaltfunktion / Zeitverz�gerung
-Sperrobjekt normal/invertiert
-Verhalten am Beginn - Ende einer Sperrung
-Grundhelligkeit



Todo:

-Ausgabe per I2C

-Helligkeitswert beim Ausschalten speichern/wiederherstellen
-Helligkeitswert bei Busspannungsausfall speichern/wiederherstellen
-Lichtszenen
-Lichtszenen ver�nderbar
-Zeitdimmfunktion
