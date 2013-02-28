Beschreibung:
-------------
Die Applikation 'app-in8' beinhaltet die Firmware für einen Binäreingang 8-fach.
Die Funktionalität ist dabei weitgehend kompatibel zum Rollladenaktor '2118 REG' der Fa. Jung.


Funktionsuebersicht:
--------------------
+ Zeitverzögerung bei Busspannungswiederkehr einstellbar
+ Verhalten bei Busspannungswiederkehr für jeden Eingang parametrierbar
+ Telegrammratenbegrenzung einstellbar
+ Entprellzeit einstellbar
+ Sperrobjekt für jeden Eingänge (Polarität einstellbar)
+ Auslesen aller Objektzustände 
+ Programmier- und parametrierbar über ETS
+ Freie Zuordnung der Funktionen Schalten, Dimmen, Jalousie, Wertgeber zu den Eingängen
+ Funktion Schalten:
  2 unabhängige Schaltobjekte pro Eingang
  zyklisches Senden Der Schaltobjekte in Abhängigkeit des Objektwertes parametrierbar
  Reaktion für steigende und fallende Flanke für jedes Schaltobjekt parametriebar (EIN/AUS/UM)
+ Funktion Jalousie:
  Reaktion für steigende Flanke einstellbar (keine Funktion, AUF, AB, UM)
  Bedienkonzept Kurz–Lang–Kurz und Lang-Kurz
  Zeit zwischen Kurz- und Langzeitbetrieb einstellbar (nur bei Kurz–Lang–Kurz)
  Lamellenverstellzeit einstellbar 

  
 Nicht umsetzbare bzw. (noch) nicht implementierte Funktionen:
- Funktion Dimmen
- Funktion Wertgeber
- Funktion Impulszähler
- Funktion Schaltzähler
- Verhalten bei Spannungsausfall


Kennzeichnung Releases für die verschiedenen Varianten:
 TP301   AVR-Controller Rev. 3.01
 m168p   Prozessor ATmega 168P bzw. ATmega 168PA
 m328p   Prozessor ATmega 328

 
Release-Uebersicht:
------------------
app-8-in_20130216_TP301_m168p.hex
 Erstimplementierung fuer neue Lib, aufgelistete Funktionen wurde verifiziert (siehe Protokoll)
 
 
