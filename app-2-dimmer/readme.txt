Stand 20.05.2013
Applikation f�r den GIRA 2 Kanal Dimmaktor 1032 00


m�gliche Ausgangsoptionen:
-DPWM8
			pulsbreitenmoduliertes Signal mit 8bit Aufl�sung an den Ausg�ngen OC2A und OC2B
			(Pin 17 und Pin 5 beim ATmega328P)
			
-DPWM10		
			pulsbreitenmoduliertes Signal mit 10bit Aufl�sung an den Ausg�ngen OC1A und OC1B
			(Pin 15 und Pin 16 beim ATmega328P)

			
Funktionen:
-Empfang & R�ckmeldung Ein/Aus Objekt
-Empfang & R�ckmeldung Helligkeitsobjekt
-Empfang Dimmen 100% auf- abw�rts, Dimmen Stop(todo: R�ckmeldung bei Dimmen Stop)


Parameter:
-Helligkeit bei Busspannungswiederkehr
-Helligkeit beim Einschalten
-Soft Ein
-Soft Aus
-Helligkeitswert andimmen/anspringen
-Dimmgeschwindigkeit
-Ausschaltfunktion / Zeitverz�gerung




Todo:

-Ausgabe per Uart
-Ausgabe per I2C

-Grundhelligkeit
-Dimmbefehle 1%-50%
-Helligkeitswert beim Ausschalten speichern/wiederherstellen
-Helligkeitswert bei Busspannungsausfall speichern/wiederherstellen
-Sperrobjekt
-Lichtszenen
-Lichtszenen ver�nderbar
-Zeitdimmfunktion
