Stand 20.05.2013
Applikation für den GIRA 2 Kanal Dimmaktor 1032 00


mögliche Ausgangsoptionen:
-DPWM8
			pulsbreitenmoduliertes Signal mit 8bit Auflösung an den Ausgängen OC2A und OC2B
			(Pin 17 und Pin 5 beim ATmega328P)
			
-DPWM10		
			pulsbreitenmoduliertes Signal mit 10bit Auflösung an den Ausgängen OC1A und OC1B
			(Pin 15 und Pin 16 beim ATmega328P)

			
Funktionen:
-Empfang & Rückmeldung Ein/Aus Objekt
-Empfang & Rückmeldung Helligkeitsobjekt
-Empfang Dimmen 100% auf- abwärts, Dimmen Stop(todo: Rückmeldung bei Dimmen Stop)


Parameter:
-Helligkeit bei Busspannungswiederkehr
-Helligkeit beim Einschalten
-Soft Ein
-Soft Aus
-Helligkeitswert andimmen/anspringen
-Dimmgeschwindigkeit
-Ausschaltfunktion / Zeitverzögerung




Todo:

-Ausgabe per Uart
-Ausgabe per I2C

-Grundhelligkeit
-Dimmbefehle 1%-50%
-Helligkeitswert beim Ausschalten speichern/wiederherstellen
-Helligkeitswert bei Busspannungsausfall speichern/wiederherstellen
-Sperrobjekt
-Lichtszenen
-Lichtszenen veränderbar
-Zeitdimmfunktion
