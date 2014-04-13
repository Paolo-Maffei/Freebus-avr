Stand 13.04.2014
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
			
-DOUT10V
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
			
-DSERVO
			Option zur Ansteuerung zweier Modellbau Servos an den Ausgängen OC1A und OC1B
			(Pin 15 und Pin 16 beim ATmega328P)
			Die Sperrfunktion wird zur Umkehrung der Bewegungsrichtung des Servos benutzt
			Über den Parameter Grundhelligkeit kann die min. und max. Pulslänge
			(linker und rechter max. Ausschlag des Servos) eingestellt werden:
			1	0,6ms - 2,4ms
			2	0,7ms - 2,3ms
			3	0,8ms - 2,2ms
			4	0,9ms - 2,1ms
			5	1,0ms - 2,0ms
			6	1,1ms - 1,9ms
			7	1,2ms - 1,8ms
			8	1,3ms - 1,7ms
			

			
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
-Grundhelligkeit



Todo:

-Ausgabe per I2C

-Helligkeitswert beim Ausschalten speichern/wiederherstellen
-Helligkeitswert bei Busspannungsausfall speichern/wiederherstellen
-Lichtszenen
-Lichtszenen veränderbar
-Zeitdimmfunktion
