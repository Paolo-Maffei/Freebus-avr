Stand 30.05.2013
Applikation für den GIRA 2 Kanal Dimmaktor 1032 00


mögliche Ausgangsoptionen:
-DPWM8
			pulsbreitenmoduliertes Signal mit 8bit Auflösung an den Ausgängen OC2A und OC2B
			(Pin 17 und Pin 5 beim ATmega328P)
			
-DPWM10		
			pulsbreitenmoduliertes Signal mit 10bit Auflösung an den Ausgängen OC1A und OC1B
			(Pin 15 und Pin 16 beim ATmega328P)

-DUSE_UART
			Ausgabe des Helligkeitswertes über die serielle Schnittstelle
			mit den Parametern:   38400, 8, N, 1
			Sendeformat: highbyte hex; lowbyte hex; CR; LF
			10bit Auflösung; Bit15=0 -> Kanal1; Bit15=1 -> Kanal2
 

			
Funktionen:
-Empfang & Rückmeldung Ein/Aus Objekt
-Empfang & Rückmeldung Helligkeitsobjekt
-Empfang Dimmen 1,5% - 100% auf- abwärts, Dimmen Stopp
-Sperrobjekt

Parameter:
-Helligkeit bei Busspannungswiederkehr
-Helligkeit beim Einschalten
-Soft Ein
-Soft Aus
-Helligkeitswert andimmen/anspringen
-Dimmgeschwindigkeit
-Ausschaltfunktion / Zeitverzögerung
-Sperrobjekt normal/invertiert
-Verhalten am Beginn - Ende einer Sperrung



Todo:

-Ausgabe per I2C

-Grundhelligkeit
-Helligkeitswert beim Ausschalten speichern/wiederherstellen
-Helligkeitswert bei Busspannungsausfall speichern/wiederherstellen
-Lichtszenen
-Lichtszenen veränderbar
-Zeitdimmfunktion
