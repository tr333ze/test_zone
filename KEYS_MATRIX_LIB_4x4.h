//-------------------------------------------------------------------------------
//
// Biblioteka do obslugi klawiatury matryciwej 4x4 podpietej do jednego portu
//
//-------------------------------------------------------------------------------

#include <avr/io.h>
#include <avr/interrupt.h>
									
#define INTERRUPT SIG_INTERRUPT0					// W zaleznosci ktore przerwanie
#define Int_reg      GICR
#define Int_get      MCUCR
#define Int_type     INT0
#define Int_flag_reg GIFR
#define KEYB  PINA									// PORT z ktorej odczytujemy stany bitow
#define KEYSD DDRA
#define KEYSP PORTA
		
#define IntPortout PORTD							// Port na ktorym jest wyprowadzenie
#define IntPortconf DDRD							// wypr. od zew.przrwania

#define NewKey NewKey							// Obiekt trzymajacy informacje o nowo
													// wcisnietym klawiszu
#define KeyNum KeyNum							// NUmer wcisnietego

//*******************************************************************************
// Funkcja ktora za argumenty przyjmuje numer przerwania:
//	0 - INT0 , 1 - INT1
// Drugi argument informuje jakim stanem ma byc wywolane przerwanie :
// u - zbocze rosnace , f - malejace , b - jaka kolwiek zmiana stany , l - stan niski
// Trzeci to numer wyprowadzenia na PORCIE odpowiadajacy za przerwanie
//	ISn0    ISCn1
//	  0       0			- Przerwanie na stan niski 	 
//    0       1			- Jaka kolwiek zmiana
//	  1       0			- zbocze malejace
//    1       1			- zbocze rosnace

void InterruptInitial(short Number , char Edge ,short PortPin)
	{
		if(!Number)
			{   
				Int_reg |= 1<<INT0;
				if(Edge == 'u'){Int_get |= ( 1<<ISC01 | 1<<ISC00 );}
				if(Edge == 'f'){Int_get =(Int_get|= 1<<ISC01) & ~(1<<ISC00);} 
				if(Edge == 'b'){Int_get = (Int_get | 1<<ISC00) & ~(1<<ISC01);}
				if(Edge == 'l'){(Int_get &= ~(1<<ISC00 | 1<<ISC01));}
				Int_flag_reg |= 1<<INTF0;
			}
			
		else{		
					
				Int_reg |= 1<<INT1;
				if(Edge == 'u'){Int_get |= ( 1<<ISC11 | 1<<ISC10 );}
				if(Edge == 'f'){Int_get =(Int_get|= 1<<ISC11) & ~(1<<ISC10);} 
				if(Edge == 'b'){Int_get = (Int_get | 1<<ISC10) & ~(1<<ISC11);}
				if(Edge == 'l'){(Int_get &= ~(1<<ISC10 | 1<<ISC11));}
				Int_flag_reg |= 1<<INTF1;
			}

		IntPortconf &= ~(1<<PortPin);	
		IntPortout |= 1<<PortPin;
			
	}

//*******************************************************************************

SIGNAL(INTERRUPT)
	{
	_delay_ms (20);
	if(!NewKey)
		{
		NewKey = 1;
		  if(KEYB == 0b00001110){KeyNum = 0;}
		  if(KEYB == 0b00001101){KeyNum = 4;}
		  if(KEYB == 0b00001011){KeyNum = 8;}
		  if(KEYB == 0b00000111){KeyNum = 12;}
		KEYSD = 0b00001111;
		KEYSP = 0b11110000;
		_delay_ms(60);
		  if(KEYB == 0b11100000){KeyNum += 0;}
		  if(KEYB == 0b11010000){KeyNum += 1;}
		  if(KEYB == 0b10110000){KeyNum += 2;}
		  if(KEYB == 0b01110000){KeyNum += 3;}
		KEYSD = 0b11110000;
		KEYSP = 0b00001111;
		GIFR = 1<<INTF0;
		_delay_ms(20);
		}
	else
		{
		NewKey = 0;
		GIFR = 1<<INTF0;
		}
	}
