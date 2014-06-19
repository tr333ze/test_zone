Maciej Potas
MMM
ZZZ
// Projektem jest stworzenie prostego odtwarzacza dzwiekow MONO w postaci odgrywalnej
// muzyczki ;).

#include <avr/interrupt.h>
#include "MusicMonoPlayer.h"
#include "HD44780_LIB.h"
#include "KEYS_MATRIX_LIB_4x4.h"
#define SYS_FREQ 4000000
#define PRESKALER 64

// void LcdInitial();						// Funkcja inicjalizujaca wyswietlacz
// void LcdData(unsigned char);    			// Funkcja przesylajaca wybrany bajt do wyswielacza
// void LcdInstr(unsigned char);			// Funkcja przyjmujaca bajt jako instrukcje
// void LcdOwnSigns(unsigned char *, short);// Funkcja wlasne znaki do pamieci CGRAM
// void LcdMoveTo(short , short); 			// Ustawia sie na ustalonej pozycji ( kolumna , wiersz )
// void LcdMoveCurs(unsigned char )			// rusza kursorem u - gora , d - down , l -lewo , r - prawo
// void LcdViewSign(unsigned char );    	// wypisuje pojedynczy znak na wyswietlaczu
// void LcdViewString(unsigned char *); 	// wypisuje C- STRING
// void LcdClean(); 						// Czysci wyswietlacz
// void LcdNewLine();						// przechodzi do nowej linii
// void LcdSpace();							// Wstawia spacje 
// void LcdBackspace();						// Usuwa porpzedni znak
// void LcdDistance(short);					// Wstawia okreslona liczbe wolnych pol

int main()
	{

	#include "Melodies_definies.h"			// W odrebnym pliku sa definiowane , melodie
											// zeby nie niszczyc maina'a	
	
	char Signs[16] = {0x10,0x10,0x17,0x15,0x15,0x15,0x11,0x1F,
					  0x00,0x01,0x01,0x02,0x02,0x14,0x14,0x08};

	Parametr.MusicNumber = 0;
	Parametr.PlayerLoopCounter = 0 ;
	Parametr.PlayerCounter = 0;
	Parametr.temp_player_loop_counter = 0;
	Parametr.Note_Number = 0 ;
	
	DDRA = 0xF0;
	PORTA = 0x0F;
	PORTD &= 1<<3;
	DDRD |= 1<<3;
	TIMSK = 1<<TOIE0 | 1<<TOIE2;
	InterruptInitial(0,'b',2);
	LcdInitial();
	LcdOwnSigns(Signs,2);
	LcdViewString("Mono Player");
	LcdNewLine();
	LcdViewString("v 0. 1");
	LcdMoveTo(2,15);
	LcdViewSign(0);
	_delay_ms(1000);
	LcdClean();
	LcdViewString(MyMelodyBox[0].title);
	LcdNewLine();
	LcdViewString(MyMelodyBox[1].title);
	LcdMoveTo(1,15);
	LcdViewSign(0);
	sei();
	while(1)
	  {
		if(NewKey)										// Oczekiwanie na wcisniecie klawisza
			{
			cli();
			   if(KeyNum == 0)				
				 {
					LcdClean();							// Wyczyszczenie LCD
					LcdViewString("By Saya");			// wypisanie imienia , oraz		
					LcdSpace();							// loga . Nastepnie zostaja
					LcdViewSign(0);						// odczekane 2 sec a nastepnie
					_delay_ms(1000);					// wyswietlacz zostaje
					LcdRefresh();						// odswiezony
				 }		
			   if(KeyNum == 1)							// Idziemy lista w dol
			   	 {										// wiec nastepuje inkrementacja
				 	++Parametr.MusicNumber;						// numeru utworu
				 	if( (Parametr.MusicNumber+1) % 2 )					// oraz sprawdzenie warunku
						{								// czy lista ma juz zostac odswiezona
						  LcdRefresh();					// Dla 3 , 5 , 9... wartosc modulo
						}								// dzielenia przez 2 daje 1 . Wiec
					else								// wysujemy od nowa liste poczynajac od gory
						{
						LcdMoveTo(1,15);
						LcdSpace();
						LcdMoveTo(2,15);
						LcdViewSign(0);
						}
				  }
			    if(KeyNum == 2)							// Dziala na takiej zasadzie
					{									// Jak przewijanie listy w dol
					--Parametr.MusicNumber;
					if( !((Parametr.MusicNumber+1) % 2))
						{
						  LcdRefresh();
						}
					else
						{
						LcdMoveTo(2,15);
						LcdSpace();
						LcdMoveTo(1,15);
						LcdViewSign(0);
						}
					}
				if(KeyNum == 5)
					{
					LcdMoveTo(wiersz-1,15);
					LcdViewSign(1);
					Play(Parametr.MusicNumber);
					LcdMoveTo(wiersz-1,15);
					LcdViewSign(0);
					}	
			sei();
			while(NewKey ==1);

				}
		
	}
}

void LcdRefresh()
	{
	
	LcdClean();
	if((Parametr.MusicNumber+1) % 2)
		{
			LcdMoveTo(1,0);
			LcdViewString(MyMelodyBox[Parametr.MusicNumber].title);
			LcdNewLine();
			LcdViewString(MyMelodyBox[Parametr.MusicNumber+1].title);
			LcdMoveTo(1,15);
			LcdViewSign(0);
		}
	else
		{
			LcdMoveTo(1,0);
			LcdViewString(MyMelodyBox[Parametr.MusicNumber-1].title);
			LcdNewLine();
			LcdViewString(MyMelodyBox[Parametr.MusicNumber].title);
			LcdMoveTo(2,15);
			LcdViewSign(0);
		}
	}

int Count(double Value)
	{
		return (int) ((SYS_FREQ /(PRESKALER*(Value)))+0.5);
	}
int CountLoop(double Value)
	{
		return (int)((SYS_FREQ /(PRESKALER*(Value)))/255+0.5);
	}

void Play(int Number)
	{
	Parametr.Note_Number = 0 ;
	TCCR0 = 1<<CS01 | 1<<CS00;	
	sei();
	int ax = 0 ;
		for(int i = 0 ; i <= MyMelodyBox[Number].length ;i++)
			{
				ax = i ;
				Parametr.PlayerCounter = Count(MyMelodyBox[Parametr.MusicNumber].Table[i].Note);
				TCNT0 = 255 - Parametr.PlayerCounter;
				if(Parametr.PlayerCounter>255)
					{
						Parametr.PlayerLoopCounter=CountLoop(MyMelodyBox[Parametr.MusicNumber].Table[i].Note);
						TCNT0 = 0 ;
					}
				_delay_ms(1000*(0.1*MyMelodyBox[Parametr.MusicNumber].Table[i].Notelength))	;
				++Parametr.Note_Number;	
			if(KeyNum == 6){i = MyMelodyBox[Number].length;}
			}
	TCCR0 &= ~(1<<CS01 | 1<<CS00);
	//GIFR = 1<<INT0;
	cli();
		
	}
