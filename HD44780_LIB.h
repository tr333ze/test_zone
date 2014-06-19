//-------------------------------------------------------------------------------
//
// Biblioteka do obslugi wyswietlacz 2*16 znakow ze sterownikiem HD44780
//
//-------------------------------------------------------------------------------

/*  // Dla rozszerzenia extern glownego pliku
extern void LcdInitial();						// Funkcja inicjalizujaca wyswietlacz
extern void LcdData(unsigned char);    			// Funkcja przesylajaca wybrany bajt do wyswielacza
extern void LcdInstr(unsigned char);			// Funkcja przyjmujaca bajt jako instrukcje
extern void LcdOwnSigns(usigned char *, short);	// Funkcja wlasne znaki do pamieci CGRAM
extern void LcdMoveTo(short , short); 			// Ustawia sie na ustalonej pozycji ( kolumna , wiersz )
extern void LcdViewSign(unsigned char );   		// wypisuje pojedynczy znak na wyswietlaczu
extern void LcdViewString(unsigned char *); 	// wypisuje caly C-STRING
extern void LcdClean(); 						// Czysci wyswietlacz
extern void LcdNewLine();						// przechodzi do nastepnej linii
extern void LcdMoveCurs(unsigned char);			// rysuje okreslona liczbe spacji
extern void LcdSpace();							// Wstawia spacje 
extern void LcdBackspace();						// Usuwa porpzedni znak
extern void LcdDistance(short);					// Wstawia okreslona liczbe wolnych pol
*/
#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#define F_CPU 4000000UL     // 4 MHz
#define CR 0x0A 			// przejscie do nowej linii

//*******************************************************************************

extern short kolumna , wiersz ;    // Globalne zmienne zewnetrzne potrzebne do okreslania
								  // pozycji na wyswielaczu
									
//*******************************************************************************

#define KLCD_LINE DDRD		// Rejestr kierunku portu	
#define OLCD_LINE PORTD		// Rejestr wyjscia portu

#define RS_LINE	0 			// Numer wyprowadzenia informujaczy czy dana(1) / instrukcja(0)
#define E_LINE 1  			// Numer wyprowadzenia potwierdzajacego dana / instrukcje

// Bity informujaca o znaku , lub instrukcji
#define DB7 7
#define DB6 6
#define DB5 5
#define DB4 4
#define DB3 0    			//   
#define DB2 0    			// U¿ywany tryb 4 bitowej transmisji
#define DB1 0	 			//	
#define DB0 0    			//

//*****************************************************************************

// Wszystkie etykiety potrzebne do uzyskania interesujacych nas konfiguracji

#define LCD_CL 0
#define LCD_RE 1
#define LCD_VAR_INPUT 2
	#define LCD_CURS_INCR 1		// 1 - rosnie , 0 malaje
	#define LCD_WIND_MOVE 0		// przesuwanie oknem po kazdym zapisie

#define LCD_OPTIONS 3		
	#define LCD_POW 2			// 1 - wlacz lcd , 0 - wylacz lcd
	#define LCD_CURS 1			// 1 - wlacz kursor , 0 - wylacz kursor
	#define LCD_CURS_TW 0		// 1 - wlacz migotanie , 0 - wylacz

#define LCD_CURS_OPTIONS 4
	#define LCD_CURS_MOVE  3    // 1- przesuwanie kursora , 0 - przesuwanie wysw
	#define LCD_CHANGE_WA  2	// 0 - w lewo , 1 - w prawo

#define LCD_GLOBAL 5 
	#define LCD_INTER 4			// 1 - 8 bitowy , 0 - 4 bitowy
	#define LCD_LINES 3			// 1 - 2 linie , 0 - 1 linia
	#define LCD_CHARS 2			// 1 - 5x10 , 0 5x7

#define CGRAM_ADRESS 6
#define DDRAM_ADRESS 7


//*****************************************************************************
// 				Funkcje sterujace / konfiguracyjne							//
//*****************************************************************************

void LcdData(unsigned char Data)
	{
		OLCD_LINE |= 1<<E_LINE;						// Stan E wysoki	
		OLCD_LINE &= ~( 1<<DB7 | 1<<DB6 | 1<<DB5 | 1<<DB4);
		OLCD_LINE |= (Data & 0xF0);					// przygotowanie starszej czesci
		OLCD_LINE &= ~(1<<E_LINE);					// Stam E niski , przeslanie danej
		_delay_ms(5);
		OLCD_LINE |= 1<<E_LINE;
		OLCD_LINE &= ~( 1<<DB7 | 1<<DB6 | 1<<DB5 | 1<<DB4);
		OLCD_LINE |= ((Data & 0x0F)<<4);
		OLCD_LINE &= ~(1<<E_LINE);
		_delay_ms(5);
		OLCD_LINE |= 1<<E_LINE;	
 	} 

//*****************************************************************************

void LcdInstr(unsigned char Data)
	{
		OLCD_LINE &= ~(1<<RS_LINE);					// Linia RS w stanie 0 czyli instr.
		LcdData(Data);							
		OLCD_LINE |= 1<<RS_LINE;					// Standardowo przyjmuje ,ze idzie
		_delay_us(40);								// dana nie intsrukcja wiec znow stan wysoki
	}

//*****************************************************************************

void LcdInitial()
	{
		KLCD_LINE |=   0b11110011;
		OLCD_LINE &= ~(0b11110011); 
		OLCD_LINE |= 1<<E_LINE;	
		OLCD_LINE &= ~(1<<RS_LINE);	
		OLCD_LINE &= ~( 1<<DB7 | 1<<DB6 | 1<<DB5 | 1<<DB4);
		OLCD_LINE |= 1<<DB4 | 1<<DB5;
		_delay_ms(40);									// petla przesylajaca 3 razy
		for(int i = 0 ; i <3 ;i++)						// wartosc 0x30
			{
			asm("nop");
			asm("nop");
			OLCD_LINE &= ~(1<<E_LINE);	
			_delay_ms(5);
			OLCD_LINE |= 1<<E_LINE;	
			}
		OLCD_LINE &= ~( 1<<DB7 | 1<<DB6 | 1<<DB5 | 1<<DB4);
		OLCD_LINE |= 1<<LCD_GLOBAL;
		asm("nop");									    // 0x20 , wic interfejs 4 bitowy
		OLCD_LINE &= ~(1<<E_LINE);						// bi szosty odpowiedzialny za to 
		_delay_ms(10);									// przyjmuje 0 ustawia 4 bitowy
		OLCD_LINE |= 1<<E_LINE;	
		LcdInstr(1<<LCD_GLOBAL | 1<<LCD_LINES);
		LcdInstr(1<<LCD_CURS_OPTIONS | 1<<LCD_CURS_MOVE | 1<<LCD_CHANGE_WA);
		LcdInstr(1<<LCD_VAR_INPUT | 1<<LCD_CURS_INCR);
		LcdInstr(1<<LCD_OPTIONS | 1<<LCD_POW );//| 1<<LCD_CURS | 1<<LCD_CURS_TW);
		_delay_ms(10);
		LcdClean();
	} 

//*****************************************************************************

void LcdOwnSigns(unsigned char * Signs,short Number)

	{
//	short CGDRAM_ADR = 0;
	LcdInstr(1<<CGRAM_ADRESS);						// Przejscie do trybu adresowania
													// pamieci CGDRAM i wymuszenie
													// adresu 0
		for(int i = 0 ; i < Number * 8 ; i++)
			{
			//for(int i = CGDRAM_ADR ; i <CGDRAM_ADR + 8 ; i++)
			//	{
			//	LcdInstr(1<<CGRAM_ADRESS | i);
				LcdData(*Signs++);
			//	}
			
		//	CGDRAM_ADR += 8;
			}
	LcdInstr(1<<DDRAM_ADRESS);
	}


//*****************************************************************************
// 			         	Funkcje u¿ytkowe									//
//*****************************************************************************
// Funckcja wyswietlajaca znak na wyswietlaczu .  W wiekszej czesci odpowiadajaca
// znakowi w kodzie ASCII

void LcdViewSign(unsigned char sign)				// Wypisanie pojedynczego znaku na wyswietlaczu
	{
		LcdData(sign);								// Jesli przekraczamy zakres to schodzimy wierszem
		if(++kolumna == 16)							// w dol . Jesli przekraczamy dwa wiersze to piszemy
			{										// od poczatku
			LcdNewLine();
			}
		
	}

//*****************************************************************************
// Funkcja za arhument przyjmuje wskaznik do C-STRING'a. Wypisuje do poki
// nie napotka znaku konczcego C-STRING czyli NULL

void LcdViewString(unsigned char *string)			// do poki nie dojedziemy do ostatniego
													// znaku C-Stringu to piszemy wszystko 
	{		
												// jak leci.
		while(*string != 0)						
			{	
				if(*string == CR){ LcdNewLine();}
				else { LcdViewSign(*string++); }

			}
	}


//*****************************************************************************

void LcdMoveTo(short row, short column)				// Przenosimy sie w wybrane miejsce na wyswietlaczu
	{						
													// zrobione dla przypadku tylko dwu liniowego
		wiersz = row;
		kolumna = column;
		LcdInstr((1<<DDRAM_ADRESS) | ((0x40)*(row-1) + (column-1)));
		_delay_us(40);

	}

//*****************************************************************************
	
void LcdClean()										// Instrukcja 0x01 jest odpowiedzialna
	{
		LcdInstr(1<<LCD_CL);						// za wypelnienie znakami ' ' calego wyswietlacza
		_delay_ms(2);								// kursor powraca do poczatku
			
	}

//*****************************************************************************

void LcdNewLine()
	{
		short temp_wiersz = wiersz;
		wiersz = (++temp_wiersz == 3)?  1: temp_wiersz ;
		kolumna = 0;
		LcdMoveTo(wiersz,kolumna);
	}

//*****************************************************************************
// Argumentem jest obiekt typu char;
// u - przesuniecie kursora w gore
// d - przesuniecie kursora w dol
// r - przesuniecie kursora w prawo
// l - przesuniecie kursora w lewo

void LcdMoveCurs(signed char Way)
	{
		if(Way == 'u')
			{
				short temp_wiersz = wiersz;
				wiersz = (--temp_wiersz == 0)?  2: temp_wiersz ;

			}
		if(Way == 'd')
			{
				short temp_wiersz = wiersz;
				wiersz = (++temp_wiersz == 3)?  1: temp_wiersz ;

			}
		if(Way == 'r')
			{
				short temp_kolumna = kolumna;
				if(++temp_kolumna == 17)
					{
					kolumna = 0;
					wiersz = (wiersz == 2)? 1 : 2;
					}
				else{++kolumna;}
			}
		if(Way == 'l')
			{
				short temp_kolumna = kolumna;
				if(--temp_kolumna == -1)
					{
					kolumna = 16;
					wiersz = (wiersz == 1)? 2 : 1;
					}
				else{--kolumna;}
			}

	LcdMoveTo(wiersz,kolumna);

	}

//*****************************************************************************

void LcdSpace()								// Na wyswietlczu zostanie wstawiona
	{										// spacja . ( kolumny sa zliczane bezposrednio
											// w funkcji wypisujacej.
		LcdViewSign(' ');

	}


//*****************************************************************************

void LcdBackspace()	
	{
		if(kolumna > 0)
			{
			LcdMoveCurs('l');
			LcdSpace();
			LcdMoveCurs('l');
			}

	}

//*****************************************************************************

void LcdDistance(short Number_of_spaces)
	{
		for(int i = 0 ; i <Number_of_spaces ;i++)
			{
				LcdSpace();
			}
	}

//*****************************************************************************
