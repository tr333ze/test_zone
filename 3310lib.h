//--------------------------------------------------------------------- 
//	Biblioteka do obs³ugi wyœwietlacza ze sterownikiem		     	  |	
//					PCD8544(N3310)			|			         	  |
//											|__					 	  |
//											| |	|					  |
//	By Saya		      						| |	|					  |	
//	Header									|___|					  |
//																	  |
//---------------------------------------------------------------------

#ifndef N3310LIB
#define N3310LIB
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/pgmspace.h>
#include "3310libFont.h"
#include "hardef.h"
#include "macro.h"

	// Stale zwiazane z parametrami wyswietlacza
	// Zestaw podstawowy H = 0 
		#define DisplayOff      (0x08 | 0x00)
		#define DisplayNormal   (0x08 | 0x01)
		#define DisplayAll      (0x08 | 0x04)
		#define DisplayInvers   (0x08 | 0x05)
		#define SetYAdress		0x40
		#define SetXAdress		0x80

	//Zestaw rozszerzony H = 1
		#define TEMP            (0x04 | 0x02)
		#define BIAS            (0x10 | 0x03)
		#define VOP             (0x04 | 0xc3) 
	
	// Makra zwizane ze sterowaniem wyswietlacza :
	#define DC_1  PORT(LCD_PORT) |= 1<<DC			// Dwa Makra ktore w prosty
	#define DC_0  PORT(LCD_PORT) &= (~(1<<DC))		// i latwy sposob beda nam
													// resetowac sterownik oraz
	#define RES_1 PORT(LCD_PORT) |= 1<<RES			// przeskakiwac pomiedzy typem,
	#define RES_0 PORT(LCD_PORT) &= (~(1<<RES))		// wysylanego bajtu

	#define SCE_1 PORT(LCD_PORT) |=1<<SCE
	#define SCE_0 PORT(LCD_PORT) &= (~(1<<SCE))
	
	// Funkcje Dzialajace bezposrednio na wyswietlaczu
	
	void LcdInitial();																											
	void SendSpi(u8 );					
	void LcdInstr(u8 );					
	void LcdClean();								
	void LcdMoveTo(u8 , u8);  								
	void LcdMoveToC(u8, u8);
	void LcdViewSign(u8);
	void LcdViewDisigt(u8 , u8);
	void LcdViewString(u8 *);
	void LcdDisplayBmp(u8,u8,u8 ,u8,struct BMP *);
	void LcdSpace();
	void LcdFillAll(u8 Sign);
	
	// Funkcje dzialajace na tablicy wirtualnie przedstawiajacej wyswietlacz
	void VirtualLcdClean(u8 (*)[84],uint16_t* , uint16_t*);
	void VirtualLcdWrite(u8 (*)[84],uint16_t  , uint16_t );
	
	// Funkcje rysowania ( wszystkie operacje sa przeprowadzane na "tablicy wirtualnej" )
	u8 CheckPix(u8 (*VirutalLcd)[84] , uint16_t);
	void LcdPixWrite(u8 (*)[84],uint16_t,struct Font*, u8);
	void PaintLcdPix(u8 (*)[84],uint16_t,u8);
	void PaintLcdLine(u8 (*)[84],uint16_t , uint16_t , u8 Op);
	void PaintLcdRect(u8 (*)[84],uint16_t , uint16_t , u8);
	void PaintLcdCircle(u8 (*)[84],uint16_t,u8,u8);

	// Funkcje potrzebne do zegarka ( a ,ze moga sie kiedys przydac to zostawiam tutaj)
//	void LcdTimeView(u8 (*)[84],uint16_t,struct Font*, struct Time* , u8 *[]);
	
	
#endif
