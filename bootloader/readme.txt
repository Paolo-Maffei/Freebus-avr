Der freebus bootloader.
=======================
9. 2. 2011  Dirk Armbrust (tuxbow)
update 3. 3. 2011

nur für AVR ATMEGA328P. Denn der bootloader ist fast 4k byte.

Beim ATMEGA328 müssen die fuses gesetzt werden so dass
 at reset jump to bootloader,
 bootloader size: 4k byte (2k words) bootloader size.

Für das "alte" AVR Board (3.01) :
efuse = 05
hfuse = D1
lfuse = FF

Der Bootloader prüft, wenn ca 8 Sekunden lang kein telegramm
an ihn eingegangen ist, das flash byte auf adresse 0, also das
erste byte im flash. Ist es 0x0C (was bei einem jump der fall ist),
wird die Applikation gestartet. Vorher wird noch die PROG Led
eingeschaltet. Sie wird normalerweise von der App gleich wieder
aus gemacht, aber man sieht sie einmal kurz aufleuchten dabei.

Wann ist das Telegramm an den Bootloader adressiert?
Nach einem power-on reset bestimmt das erste vom bus kommende
Telegramm die PA des Bootloaders (Zieladresse muss eine physikalische
Adresse sein).
Wenn der Bootloader aus einer Applikation heraus gestartet wird,
wird die PA der App auch für den Bootloader verwendet.

Wie startet man den Bootloader von der App aus?
Durch einen A_Memory_Write auf Adresse 0x8000, 2 byte, vorzugsweise 0x00, 0x00.
Das geht aber nur mit der neuesten avr-lib, derzeit nur im git verfügbar.

Es müssen nun A_Memory_Write Telegramme folgen.
Gültig sind nur gerade Adressen und geradzahlige Anzahl von bytes.

Das flash ist in pages zu je 128 byte organisiert.
Es müssen alle 128 byte mittels A_Memory_Write übertragen werden.
Der eigentliche erase/write Zyklus wird dann ausgelöst, wenn das
letzte übertragene byte genau auf dem Ende einer page liegt
(also adressen 0xXX7F oder 0xXXFF, XX ist eine beliebige hex Zahl).
Bei einem erase/write blitzt die PROG Led kurz auf.

ACHTUNG: Beim schreiben, und nur beim schreiben, muss man die Adresse mit einem
Offset von 0x1000 beaufschlagen. Falls versehentlich eine Applikation
adressiert ist, und nicht ein Bootloader, können so nicht die eeprom parameter
einer App zerstört werden.

Mit eibd kann man das z.B. so machen, um die erste page des flash zu schreiben
  (myname ist der Name des Computers, auf dem der eibd läuft):
mwriteplain ip:myname 0.0.7 1000 0C 94 7E 00 0C 94 9B 00 0C 94 3D 11
mwriteplain ip:myname 0.0.7 100C 0C 94 9B 00 0C 94 9B 00 0C 94 9B 00
mwriteplain ip:myname 0.0.7 1018 0C 94 9B 00 0C 94 9B 00 0C 94 9B 00
mwriteplain ip:myname 0.0.7 1024 0C 94 67 11 0C 94 42 0C 0C 94 9B 00
mwriteplain ip:myname 0.0.7 1030 0C 94 9D 00 0C 94 9B 00 0C 94 E9 0D
mwriteplain ip:myname 0.0.7 103C 0C 94 A4 0C 0C 94 9B 00 0C 94 77 16
mwriteplain ip:myname 0.0.7 1048 0C 94 9B 00 0C 94 9B 00 0C 94 9B 00
mwriteplain ip:myname 0.0.7 1054 0C 94 9B 00 0C 94 90 05 0C 94 9B 00
mwriteplain ip:myname 0.0.7 1060 0C 94 9B 00 0C 94 9B 00 07 01 01 0D
mwriteplain ip:myname 0.0.7 106C 01 FF 12 01 9A 0C 01 00 00 00 00 EA
mwriteplain ip:myname 0.0.7 1078 01 00 F6 01 55 F7 01 55

Also 10 x 12 byte und 1 x 8 byte, macht 128 byte, eine page.

Mit A_Memory_Read kann der Inhalt des flash ausgelesen werden.
Hier aber kein Adress Offset anwenden.
Weil das Lesen noch langsamer ist als das Schreiben, kann zur
Verifikation des flash Inhalts der CRC32 einer page angefordert
werden. Das macht man mit einem A_Memory_Read auf die Anfangsadresse
einer page und einer Länge von 4 byte, der eibd Befehl wäre

mread ip:myname 0.0.7 0000 4
89 CE 8E 34
 
und wir sehen den CRC32 der ersten flash page.

A_Memory_Read mit Adresse, die nicht die Anfangsadresse einer page ist,
 oder mit einer byte Anzahl != 4, liest direkt den flash Inhalt aus.

Wegen der eingangs erwähnten Prüfung der flash Adresse 0 für den Sprung in die
Applikation, sollte man zuerst die page 0 mit 0xff füllen. Dann bleibt der
Bootloader beliebig lange aktiv. Jetzt können ganz entspannt alle anderen
pages beschrieben und verifiziert werden. Als letztes schreibt man die page 0.
Danach wartet der bootloader noch ca. 8 Sekunden, und wenn er nichts mehr vom
Bus bekommt, startet er die App.

Natürlich muss man das nicht alles zu Fuß machen.
Ich habe dazu ein eibd tool geschrieben, das heisst fb-fwupload.

fb-fwupload ip:myname 0.0.7 <appcode.bin>

<appcode.bin> ist der Name der Datei mit der App, es muss ein .bin file sein.
Man kann die avr-gcc toolchain dazu bewegen, ein .bin file auszugeben.
Ich habe in die Makefiles von app-8-in und app-8-out ein target bin eingebaut,
"make bin" erzeugt also z.B. ein app-8-out.bin .
Oder man wandelt ein hex file in ein bin file um. hex2bin wäre das tool.
Bei einem .hex file können die Adressen drcuhienandregweürflet vorliegen
und evtl Lücken enthalten.
Das kann der bootloader nicht vertragen. Daher braucht man ein .bin file.

compilieren des bootloaders
---------------------------
es gibt ein Makefile für GNU make
Die .c files aus ../lib sind direkt eingebunden.
Das bauen weiterer libs für den bootloader halte ich nicht für sinnvoll. Es gäbe
dann zu viele Versionen.
Mit dem compile switch -DBOOTLOADER wird eine Minimalversion des EIB stacks erzeugt.

Den bootloader gibt's erstmal nur für upload über TP. Upload über RF will ich aber
noch machen. Es gibt erstmal ein Problem mit dem Umfang des codes, z.Zt. sind es
6k byte, und die bootsektion des mega328p hat nur 4k.

Nachtrag 16. 2. 2011
--------------------
Ich habe jetzt auch einen RF-Bootloader.
Er wird "normal" über das TP-RF gateway angesprochen.
Er ist allerdings 6kByte groß, belegt flash ab (byte)addr. 0x6800.
Einsprungadresse ist aber nach wie vor 0x7000, damit der bootloader auch bei
reset angesprungen wird. Erreicht habe ich das durch eine extra section
.xbootloader, und einem eigenen linker script fb-avr5.x .
0x6800 liegt im NRWW Bereich des flash. Dort sind Teile des codes,
die während des flash-write nicht ausgeführt werden.
Der Bootloader schützt sich selbst, ab Adresse 0x6800 kann nicht mehr programmiert werden.

Es gibt unter software/avr/bootloader 2 Makefiles:
Makefile_tp und Makefile_rf.
Vorerst muss man noch ein make clean machen, wenn man zwischen den Beiden wechselt.

---------- als Arbeitshilfe diente mir das Protokoll eines mread ----------
dirk@dirk:~$ mread ip:slux 0.0.12 104 8
04 70 54 02 FF 00 00 00 


ft1.2 sendet
                n  t  a
 00 11 ff 00 0c 76 80
                   ^^--connect_pdu
 00 11 ff 00 0c 79 43 00
                   ^^--read_mask_version_req
                   |---ndp (numbered data packet) 0
controller:
 00 00 0c 11 ff 5a c2
                   ^^--ndp 0, ack_pdu
 ist dies die mask version, 2x ??
 00 00 0c 11 ff 5d 43 40 00 12
 00 00 0c 11 ff 5f 43 40 00 12
                   ^^ ^^ ^^-^^-- mask ver 00 12
                   ||-||- ndp 0, read_mask_version_res

ft1.2
 00 11 ff 00 0c 70 c2
                   ^^-- ndp 0, ack_pdu
 00 11 ff 00 0c 73 46 08 01 04
                   ^^ ^^-^^-^^-- 8 byte, addr 0104
                   ||-|-- ndp 1, read_memory_req

controller
 00 00 0c 11 ff 54 c6
                   ^^-- ndp 1, ack_pdu
 hier kommen die daten, 2x
 00 00 0c 11 ff 57 46 48 01 04 04 70 74 02 ff 00 00 00
 00 00 0c 11 ff 59 46 48 01 04 04 70 75 02 ff 00 00 00
                   ^^ ^^-^^-^^-- 8 byte, addr 0104
                   ||-|-- ndp 1, read_memory_res
 


ft1.2
 00 11 ff 00 0c 7a c6
                   ^^-- ndp 1, ack_pdu
 00 11 ff 00 0c 7c 81
                   ^^--disconnect_pdu

