#ifndef HSV_TO_RGB
#define HSV_TO_RGB


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

/*
von mikrocontroller.net Thread "HSV RGB Led Dimmer, C Code & Video & Doku"
 Autor: Fly (Gast)
 Datum: 06.10.2006 18:42
 &
 Autor: Benedikt K. (benedikt) (Moderator)
 Datum: 01.11.2006 12:52
 
 

H:          der Farbton als Farbwinkel H auf dem Farbkreis (z. B. 0° = Rot, 120° = Grün, 240° = Blau)
S:          die Sättigung S in Prozent (z. B. 0% = keine Farbe, 50% = ungesättigte Farbe, 100% = gesättigte, reine Farbe)
V:          der Grauwert V als Prozentwert angegeben (z. B. 0% = keine Helligkeit, 100% = volle Helligkeit)

Skalierung der HSV Werte:
H:      0-255, 0=rot, 42=gelb, 85=grün, 128=türkis, 171=blau, 214=violett
S:      0-255, 0=weißtöne, 255=volle Farben
V:      0-255, 0=aus, 255=maximale Helligkeit

&r:		0-255, rot
&g:     0-255, grün
&b:     0-255, blau
*/


//#define     Ledport             PORTB    // RGB Led Port
//#define     DDR_Ledport         DDRB
//#define     DDR_Inputport       DDRC
//#define     R_PIN               0      // R Ausgang
//#define     G_PIN               1      // G
//#define     B_PIN               2      // B
//#define    INVERT        0      // Ausgang Low aktiv ?
//
//#define    Time        1      // Statusbit für Farbwechsel
//


// Prototypen
void hsv_to_rgb (unsigned char h, unsigned char s, unsigned char v, unsigned char *r, unsigned char *g, unsigned char *b);

// Globale Variabeln (Timer ISR)
// volatile unsigned char Red, Green, Blue;    // PWM Register
// volatile unsigned char Flags;

#endif /* HSV_TO_RGB  */