9.11.2010 - 17.01.2011
------------------------
Es ist soweit. Ich habe ein funktionierendes freebus-RF System. Zwar zunächst
minimalistisch mit nur 1 Funk Teilnehmer und 1 Gateway, aber immerhin, es funktioniert.

Es ist sogar KNX-RF kompatibel! Ein Telegramm vom Hager TR302A wird verarbeitet!

Im folgenden Text verwende ich diese Abkürzungen:

TP  twisted pair, der drahtgebundene freebus
RF  radio frequency, also Funk
FB  klar, freebus
PA  physikalische adresse
GA  Gruppenadresse
DA  domain adresse
SN  Seriennummer


Als Quellen zu KNX-RF dienten mir:

das Buch
KNX, EIB für die Gebäudesystemtechnik in Wohn- und Zweckbau
Werner Kriesel ; Frank Sokollik ; Peter Helm.
Hüthig Verlag
Das Buch enthält viele Details, die ich im www nicht gefunden habe.
Die www Artikel haben eher Präsentationscharakter, hier die Links:

http://www.weinzierl.de/download/development/knx_stack_rf/KnxRf_Paper_E.pdf

https://www.auto.tuwien.ac.at/LVA/HGA/stp/abinger.pdf

https://www.auto.tuwien.ac.at/downloads/knxsci06/reinisch-wireless-knxsci06-website.pdf

Joost Demarest , Wireless congress Nov2005
http://www.carontenet.it/approfondimenti/matdomotica/orgknx/knx_rf_joost_demarest.pdf

-----------------------------------------------------------------------------------------

Hardware
--------
Als Funk-Hardware nehme ich den RFM22B (in der 868MHz Version) von hoperf.com.
Es gibt noch den kompatiblen RFM23 (etwas weniger Sendeleistung), hab' ich aber nicht probiert.
Der ältere, bekanntere RFM12 unterstützt nicht die KNX-RF chiprate, und hat auch
unbefriedigende Reichweite.

Als Microcontroller nehme ich den ATMEGA 328p. Der hat reichlich Speicher (32k flash, 1k EEPROM)
und eine SPI Schnittstelle (wichtig für RFM22).

Es gibt Schematics und Platinenlayout für 4TE Gehäuse.
svn: hardware/controller/atmega168p_4te_rf

Diese Platine kann man wahlweise bestücken:

- Ohne RFM22, mit Quarz am ATMEGA, dann ist es ein controllerboard für freebus TP.

- mit RFM22, ohne Quarz am ATMEGA, dann ist es ein gateway TP <-> RF.
  Ja, man kann sogar zusätzlich noch eine Relais- oder Input app damit betreiben.

- mit RFM22, ohne Quarz am ATMEGA, ohne TP Sende und Empfangsschaltung,
  dann hat man ein RF only board, was allerdings auch eine Stromversorgung braucht.
  Wenn man eine "nur senden" app baut, ist Versorgung aus Batterie denkbar.
  Controller und RFM22 können im sleep mode auf Tastendruck warten (<0.1mA Stromverbrauch).
  Eine app für so etwas gibt es noch nicht.
  Eine gute Lösung für die Stromversorgung muss noch gefunden werden.

todo: genaue Beschreibung der Bestückungsvarianten

Ich habe ein Gateway und ein RF only board aufgebaut.
Das Gateway hat zusätzlich noch eine out8 app.
Das RF only board ist für einen Bewegungsmelder am Grundstückseingang vorgesehen,
diese app ist nicht einem Original KNX Gerät nachempfunden, sondern auf den speziellen
Bedarf zugeschnitten. Folglich gibt es auch keine Produktdatenbank.


Antennenanschluss
-----------------
 ist für SMA oder SMB Buchse ausgelegt.
 Gut wäre eine Antenne auf PCB für RF-only Geräte.
 Das Gateway sollte eine gute Antenne haben (z.B. C* Artikel Nr. 190123, sowas kann man auch
 für weit weniger Euros selbst bauen)


Firmware
---------
Per Defines im Makefile der app können verschiedene Versionen erzeugt werden:
CUSTOM_CFLAGS=-DFB_TP  aktiviert TP Schnittstelle
CUSTOM_CFLAGS=-DFB_RF  aktiviert RF Schnittstelle
CUSTOM_CFLAGS=-DFB_TP -DFB_RF  aktiviert beide, für Gateway

CUSTOM_CFLAGS=-DBOARD301  -DFB_TP erzeugt firmware für das "alte" AVR Controllerboard Rev. 3.01.

Es gibt 4 verschiedene libs:
libfbrf.a    nur RF
libfbrftp.a  RF und TP (gateway)
libfbtp.a    nur TP
libavreib.a  für das "alte" AVR Controllerboard Rev. 3.01.

CUSTOM_CFLAGS steuert das automatische einbinden der jeweils richtigen lib.

Telegramme werden unter bestimmten Bedingungen von TP nach RF "geroutet":
- alle Broadcast Telegramme
- Multicast (an Gruppenadressen gerichtet) wenn die GA in der Adresstabelle steht.
  Sie braucht keinem Objekt (im gateway) zugeordnet sein.
- Direkt adressierte (Zieladresse ist eine PA), wenn das high byte der PA zu einer
im EEPROM abgelegten Maske passt.

Umgekehrt werden Telegramme von RF nach TP geroutet, wenn:
- die DA des RF Teilnehmers mit der DA des gateway übereinstimmt.
- oder der RF Teilnehmer eine Seriennummer sendet.
(Die Unterscheidung DA / SN erfolgt durch npci bit0.)
Eine Selektion mittels SN ist noch nicht implementiert.

Selbstverständlich werden die Telegramme im Gateway entsprechend umgebaut.
RF hat z.B. einen Datenblock (Block1) mit SN bzw DA, welcher zuerst gesendet wird,
erst danach kommen (Block2) die Daten, sie sind fast so wie bei TP. Fast. Daher wird umgebaut.

Da es bei RF kein ACK gibt, werden die GroupValue-write Telegramme vom Sender 2x wiederholt.
Der Empfänger merkt sich die PA des Absenders und die zugehörige LFN (linklayer frame number)
für einige Sekunden, und verwirft Telegramme mit derselben PA / LFN Kombination, die
kurzzeitig nacheinander empfangen werden. LFN ist ein spezielles Merkmal des RF Protokolls,
enthalten in NPCI bits 3..1 .

Um ein ACK im Applikationslayer zu implementieren, wäre das "RespondTelegram" eine Idee.
Das ist ja im code der AVR fb-relais-app schon vorgesehen (wenn ein Relais geschaltet wird,
kann optional ein Telegramm losgeschickt werden, mit einer frei wählbaren GA). Besser
wär' allerdings, das RespondTelegramm an die PA des Absenders zu schicken.

Andere Telegramme als GroupValue-write erwarten in der Regel eine Antwort. Daher merkt
der Sender an der ausbleibenden Antwort dass etwas nicht stimmt.
Eine Telegrammwiederholung auf der Sendeseite würde evtl. mit der Antwort kollidieren.
Daher werden nur GroupValue-write Telegramme wiederholt gesendet.

Konfiguration des Gateway
-------------------------
Zusätzlich zur Konfiguration der app müssen noch folgende Parameter im EEPROM abgelegt werden
(ich mache das mit eibd. Ob das mit ETS geht, weiss ich nicht. Alternativ kann man das EEPROM
direkt mit dem Programmer beschreiben. Dann muss man aber von den untigen Adressen 0x0100 abziehen!):

- Die Adresstabelle muss sämtliche GAs enthalten, für die das Gateway die Telegramme
  nach RF weiterleiten soll (Normalerweise enthält sie nur die GAs, die von der app verarbeitet
  werden).

- KNX-RF Telegramme enthalten eine 6-byte Seriennummer (SNR), oder eine 6 byte Domain Adresse (DA).
  Ich habe bisher nur die DA implementiert. Von RF nach TP werden nur diejenigen Telegramme
  geroutet, bei denen die DA mit der DA des Gateway übereinstimmt.
  Die DA muss im EEPROM auf der Adresse 0x020a-0x020f abgelegt sein.
  Ich verwende z.B. den ASCII code meines freebus nicks als DA.
  Eine Filterung von SNRs ist noch nicht implementiert. Wenn ein empfangenes Telegramm eine SNR
  (und damit keine DA) enthält, wird es z.Zt. IMMER nach TP geroutet.
  
- Telegramme mit einer PA als Zieladresse werden ebenfalls gefiltert nach RF geroutet,
  die Filterung erfolgt an Hand des oberen Bytes der PA.
  Es muss mit dem Inhalt der EEPROM zelle 0x208 übereinstimmen, maskiert von zelle 0x209.
  Beispiel: 0x0208 = 0xAA, 0x0209 = 0xA0. Weitergeleitet werden dann PAs 0xA000 bis 0xAFFF,
  oder in der üblichen Schreibweise 10.x.x (10.0.0 bis 10.15.255).



Konfiguration der Applikation
-----------------------------
Nachdem die PA des RF-Geräts gesetzt wurde, und diese mit der PA Maske des Gateway zusammenpasst,
müssten die RF Geräte genauso wie die TP Geräte mit eibd (bzw. ETS) programmierbar sein.
Freebus-Applikationen können mit kleinen Anpassungen nach RF übernommen werden. 


Speicherbelegung EEPROM
------------------------
Der "NodeParam" Bereich wurde für RF um 16 + 2 byte aufgebohrt.
(Folgende Adressen sind mit dem BASE_ADDRESS_OFFSET von 0x100 beaufschlagt,
damit man sie direkt mit mread/mwrite verwenden kann)
0x0100-0x01ff  : alter NodeParam Bereich

0x0200       :  RSSI (Received Signal Strength Indication)

0x0208       :  forward PA. Messages with physical dest address are forwarded if upper byte of
                dest address matches. Only the bits that are 1 in the MASK must match (see below).
0x0209       :  MASK for the forward PA.

0x020a-0x0120f   serial number / domain address (6 byte)

0x0210-0x0211 :  Abgleichwert für den Oszillator des RFM22.
                


Programmieren der domain address
--------------------------------
Da Funk ein offenes Medium ist, muss man sich gegen evtl vorhandene KNX-RF Systeme
in der Nachbarschaft abgrenzen. Dies geschieht mit einer "domain address" (DA).
Sie kann individuell festgelegt werden. Empfangene RF Telegramme werden nur verarbeitet,
wenn die DA von Sender und Empfänger übereinstimmen.


Zuerst programmiert man die DA des gateway über den Drahtbus, z.B. mit eibd und mwrite.
Die DA vom gateway wird automatisch in ein RF Modul übernommen, wenn dessen PA
programmiert wird. Dieser Vorgang läuft ja schon über Funk ab, und die RF Telegramme
des Gateway enthalten die DA, so dass diese in's RF Gerät übernommen werden kann.
Write PA ist ausserdem ein broadcast telegramm, es wird also unbedingt vom Gateway durchgereicht.

Fake: Damit PA read auch bei noch nicht gesetzter DA funktioniert,
wird bei PA response die DA als SN gesendet (npci bit 0 = 0).
Dadurch reicht das gateway das Telegramm weiter in den Drahtbus, unabhängig
von der DA.

zur Info:
Alternativ zur DA kann auch eine Seriennummer (SN) übertragen werden.
Dabei hat jedes KNX-RF Gerät eine eindeutige (verschiedene) SN.
Das handling der SN wird aber von freebus-RF noch nicht unterstützt.
Geräte die nur senden können (z.B. Taster, Handsender) benutzen allerdings die SN.
Da diese Geräte nicht empfangen, können sie auch nicht konfiguriert werden,
nicht einmal GAs können hier gesetzt werden. Sie senden mit festen GAs,
die erst zusammen mit der SN eindeutig werden. Hierfür muss im gateway eine
Übersetzungstabelle vorhanden sein. Dies ist wie gesagt noch nicht in freebus-RF implementiert.


Feinabgleich des RFM22 Oszillators.
------------------------------------
Zur Optimierung der Funkübertragung kann durch beschreiben der Adresse 0x0210
der Wert im Register 9 des RFM22 gesetzt werden. Wenn Adresse 0x0211 denselben
Wert invertiert enthält, wird dieser Wert auch beim reset in den RFM22 geladen.
Ein einmaliges Schreiben auf 0x0210 lädt den Wert erstmal nur in den RFM22.
Falls der dann total falsch abgestimmt ist, und nicht mehr empfangen kann,
hilft ein reset um den default wert wieder zu laden.
Die Güte der Abstimmung kann man z.B. durch lesen der Adresse 0x0200 ermitteln,
hier bekommt man den RSSI Wert zurück (Received Signal Strength Indication).
Wenn man so das Optimum ermittelt hat, schreibt man denselben Wert ein zweites
mal auf Adresse 0x0210. Dadurch wird er ins EEPROM übernommen
(incl.  Komplement -> 0x211) und auch beim reset wieder geladen.
Durch dieses Verfahren wird verhindert, dass man einen Abstimmwert in's EEPROM
schreibt, bei dem der RFM22 keinen Empfang mehr hat.

Für das AVR programming team (Matthias, Dirk O., et al)
--------------------------------------------------------
- Grundlage "master" branch aus GIT, merge mit "wip" branch erforderlich
- msg struct aufgebohrt, für zusätzlicen block1 des RF telegramms, und CRCs
- timer2 wird jetzt für RF gebraucht, damit nicht mehr für PWM zur Verfügung
  PWM jetzt mit timer 1

Fuses für ATMEGA328P
------
efuse = 0x05
hfuse = 0xD1
lfuse = 0xC0 (ext clock from RFM22), 0xFF (crystal oscillator)

Fuses für ATMEGA168(P)
------
efuse = 0x01
hfuse = 0xD5
lfuse = 0xC0 (ext clock from RFM22), 0xFF (crystal oscillator)

----------------------------------------------------------------------------------

Vergleich der KNX-RF Spec mit den Daten des RFM22
-------------------------------------------------

                  KNX/RF spec           RFM22
                  -----------           ------
Center Frequency  868.3 MHz             868.3 MHz
Frequ tolerance   +/-35ppm              ? feinabgleich ist möglich
FSK deviation     +/-40kHz to +/-80kHz  ist so einstellbar, ich nehme +/-60kHz
Rx Bandwidth      300kHz                nicht vergleichbar
Rx Sensitivity    -95dB typ.            -118dB (also wesentlich besser)
Chiprate          32.768kHz             32.77 kHz

Tx minimum ERP    0dBm                  +17dBm (auch wesentlich besser)


ERP = Effective radiated power
    = effektive Sendeleistung,
      0dBm entspricht 1mW. 
     14dBm = 25mW, das ist maximal auf der Frequenz erlaubt.
     Wenn man davon ausgeht, dass die Antenne nicht optimal gestaltet ist,
     und mindestens 3dB verliert, darf man wohl auch 17dB einstellen

Manchester Codierung:
    Prinzipiell unterstützt der RFM22 sie.
    Aber: KNX-RF protokoll hat hinter der Preambel eine "Manchester Violation".
    Die wird nicht unterstützt. Also mache ich die Manchester Codierung /
    Decodierung per software mit dem controller. Die Manchester Violation
    betrachte ich als teil des "sync" wortes.

CRC:
    auch hier Radio Eriwan: im prinzip ja.
    RFM22 kennt einige standard CRC Polynome, nicht jedoch das von KNX-RF.
    Daher mache ich auch das CRC mit dem controller.

Spezifikation war unklar in folgenden Punkten:
----------------------------------------------
inzwischen geklärt mit einem original KNX-RF Teil (Hager tebis TR 302A)

1.) Bitreihenfolge. LSB-MSB oder MSB-LSB?

    MSB-LSB ist richtig

2.) AN067 (ti.com) seite 9: final CRC complemented. Das gilt für MBUS.
    gilt das auch für KNX-RF? ja!

CRC bytes komplementiert ins Telegramm einfügen!
    und vor der CRC Prüfung nochmal (damit sie wieder "normal" sind).
    Auch das 1. byte von Block 1 in die CRC Berechnung einbeziehen.
    Für die Länge des Telegramms wird es nicht mitgezählt, für CRC schon.

Besonderheiten RF verglichen mit TP
-----------------------------------
Keine Kollisionserkennung. Das ist bei FSK einfach nicht möglich.

Kein Acknowledge auf Link Level. Optional kann ACK im Applikation Layer implementiert werden.
Siehe https://www.auto.tuwien.ac.at/downloads/knxsci06/reinisch-wireless-knxsci06-website.pdf
Seite 8. Zitat:
KNX RF does not use link layer acknowledgments for a couple of reasons.
First of all, transmit-only devices would not be able to receive acknowledgments.
Also, acknowledgments would have to include a unique identification of their sender
to be meaningful. This applies to multicasts in particular, but also in general
since on an open medium data frames and acknowledgments of multiple individual
transmissions may be mixed up. Instead of adding this overhead, KNX RF suggests
implementing end-to-end acknowledgments at the application level where required.

Ich wiederhole GroupValue-write Telegramme 2x . Siehe Abschnitt Firmware.

