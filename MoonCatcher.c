//--------------------------------------------------------------------------------------
//
//	 Prosta gerka polegajaca na lapaniu ksiezyca za pomoca klawiatury
//	 Kazde piec zlapan powoduje ,ze wzrasta szybkosc przemieszczania sie uciekiniera	
//	 klawisz 15 resetuje czestotliwosc , 
//
//
//--------------------------------------------------------------------------------------
//
#define SYS_FREQ 8000000
#define PRESKALER 1024
#define BASE_SPEED 50

short NewKey,KeyNum,wiersz = 1;
signed short kolumna = 1;
#include "KEYS_MATRIX_LIB_4x4.h"
#include "HD44780_LIB.h"

/*
void LcdInitial();						// Funkcja inicjalizujaca wyswietlacz
void LcdData(unsigned char);    			// Funkcja przesylajaca wybrany bajt do wyswielacza
void LcdInstr(unsigned char);			// Funkcja przyjmujaca bajt jako instrukcje
void LcdOwnSigns(usigned char *, short);	// Funkcja wlasne znaki do pamieci CGRAM
void LcdMoveTo(short , short); 			// Ustawia sie na ustalonej pozycji ( kolumna , wiersz )
void LcdViewSign(unsigned char );   		// wypisuje pojedynczy znak na wyswietlaczu
void LcdViewString(unsigned char *); 	// wypisuje caly C-STRING
void LcdClean(); 						// Czysci wyswietlacz
void LcdNewLine();						// przechodzi do nastepnej linii
void LcdMoveCurs(unsigned char);			// rysuje okreslona liczbe spacji
void LcdSpace();							// Wstawia spacje 
void LcdBackspace();						// Usuwa porpzedni znak
void LcdDistance(short);					// Wstawia okreslona liczbe wolnych pol

*/
struct GameObjects
	{
	short MoonSpeed : 6;						// Okresla z jaka szybkoscia porusza sie ksiezyc
	short NewMoonPosition :1;					// Nie mieszajac , dodaje 1 bit aby wiedziec kiedy nowa
	short MoonColPosition : 5 ;					// pozycja.
	short MoonRowPosition :2;					// Podwojne obiekty sa potrzebne ,zeby pamietac
	short MoonColPosition_cnt : 5;				// poprzednia pozycje ksiezyca do odswiezania okranu
	short MoonRowPosition_cnt : 2;
	short Score_l : 4;							// jednostki wyniku
	short Score_h : 4;							// dziesiatki wyniku
	int TimeLoopCounter : 10;					// Wartosc przez jaka trzeba przemnozyc 255
	int TempTimeLoopCounter : 10;				// aby otrzymac zamierzony czas
	}Game;

#include "MoonCatcher.h"

int Count(double Value);
int LoopCount(double Value);
void CheckPositions();

int main()
	{
	
	unsigned char TheMoon[16] = { 0x06,0x0F,0x18,0x10,0x10,0x18,0x08,0x06,   // wizerunek ksiezycka
								  0x04,0x04,0x0E,0x15,0x15,0x0E,0x0C,0x0C};  //	wizerunek gracza		
	DDRA = 0xF0;
	PORTA = 0x0F;
	Game.MoonSpeed = BASE_SPEED;
	Game.NewMoonPosition = 0 ;
	LcdInitial();
	InterruptInitial(0,'b',2);
	LcdOwnSigns(TheMoon , 2);
	LcdViewString("Moon catcher .... :)");
	_delay_ms(1000);
	LcdClean(0);
	LcdMoveTo(1,2);
	LcdViewSign(0);
	LcdClean(0);
	LcdMoveTo(1,2);
	LcdViewSign(1);
	LcdClean();
	LcdViewString("Wyswietlic instrukje?" );
	sei();

	while(KeyNum==0);

	if(KeyNum == 3);
		{
			LcdClean();
			LcdViewString("1 - w gore");
			LcdNewLine();
			_delay_ms(1000);
			LcdViewString("9 - w dol");
			LcdNewLine();
			_delay_ms(1000);
			LcdClean();
			LcdViewString("4 - w lewo");
			LcdNewLine();
			_delay_ms(1000);
			LcdViewString("6 - w prawo");
			_delay_ms(1000);
			LcdClean();
			LcdViewString("Tyle wstepu ... ;)");
			LcdNewLine();
			_delay_ms(3000);
			LcdClean();
 		}
	
	
	while(NewKey);
	Game.TimeLoopCounter = CountLoop(1/(0.1*Game.MoonSpeed));
	Game.TempTimeLoopCounter = Game.TimeLoopCounter;
	TCCR0  = 1<<CS02 | 1<<CS00; 
	TIMSK  = 1<<TOIE0;
	LcdClean();
	LcdViewString("5 To go ");
	LcdNewLine();
	LcdViewString("for a moon ;)");
	while(KeyNum != 5);
	LcdClean();
	LcdMoveTo(2,16);
	LcdViewSign(Game.Score_l+48);
	LcdMoveTo(2,15);
	LcdViewSign(Game.Score_h+48);
	MoonRefresh();
	LcdMoveTo(1,1);
	LcdViewSign(1);
	while(1)
		{
		if(Game.NewMoonPosition)									// Zgloszenie konca czasu
			{												// zablokowanie przerwan na ten czas
			cli();											// co by ktos nie nawciskal za duzo							
			LcdClean();	
			asm("nop");
			LcdViewString("Ojej :< ");
			LcdNewLine();
			LcdViewString("Nie udalo sie :<");			// Jakies tam haselka
				_delay_ms(2000);
			LcdClean();
			asm("nop");
			LcdViewString("Go go Hentaj ~!");
				_delay_ms(2000);
			LcdClean();
				_delay_ms(500);
			MoonRefresh();									// wyrysowanie ksiezyca
			Game.TempTimeLoopCounter = Game.TimeLoopCounter;// przepisanie poprzedniej wartosci											// po "game over" nie gramy od nowa
			Game.NewMoonPosition = 0;
			Game.Score_l = 0;
			Game.Score_h = 0;
			LcdMoveTo(2,16);
			LcdViewSign(Game.Score_l+48);
			LcdMoveTo(2,15);
			LcdViewSign(Game.Score_h+48);							// Wyzerowanie flagi nowego polozenia
			LcdMoveTo(1,1);
			kolumna = 1;
			wiersz = 1;
			Game.MoonSpeed = BASE_SPEED;
			LcdMoveTo(wiersz,kolumna);
			LcdViewSign(1);
			GIFR<<1<<INT0;									// i wracamy do glownej petli
			sei();
			}
		if(NewKey)
			{
			
			if(KeyNum == 1)
				{
				LcdMoveTo(wiersz,kolumna);
				LcdSpace();
				if(wiersz == 2){wiersz = 1;}
				LcdMoveTo(wiersz,kolumna);
				LcdViewSign(1);
				CheckPositions();
				}
			if(KeyNum == 4)
				{
				LcdMoveTo(wiersz,kolumna);
				LcdSpace();
				kolumna = (kolumna == 0)? kolumna: --kolumna ;
				LcdMoveTo(wiersz,kolumna);
				LcdViewSign(1);
				CheckPositions();
				}
			if(KeyNum == 6)
				{
				LcdMoveTo(wiersz,kolumna);
				LcdSpace();
				if(wiersz == 2){kolumna = (kolumna == 14)? kolumna: ++kolumna ;}
				else{kolumna = (kolumna == 16)? kolumna : ++kolumna ; }
				LcdMoveTo(wiersz,kolumna);
				LcdViewSign(1);
				CheckPositions();
				}
			if(KeyNum == 9)
				{
				LcdMoveTo(wiersz,kolumna);
				LcdSpace();
				if(wiersz == 1 && kolumna<= 14)
					{
					wiersz = 2;
					}
				LcdMoveTo(wiersz,kolumna);
				LcdViewSign(1);
				CheckPositions();
				}
			if(KeyNum == 3)
				{
					LcdMoveTo(1,1);
					LcdViewSign(kolumna+48);
					LcdMoveTo(wiersz , kolumna);
				}
			while(NewKey);
				
			}
		}
	
	}



