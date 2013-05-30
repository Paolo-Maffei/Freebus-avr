Stand 30.05.2013
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



Todo:

-Ausgabe per I2C

-Grundhelligkeit
-Helligkeitswert beim Ausschalten speichern/wiederherstellen
-Helligkeitswert bei Busspannungsausfall speichern/wiederherstellen
-Lichtszenen
-Lichtszenen ver�nderbar
-Zeitdimmfunktion
