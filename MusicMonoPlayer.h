// Header file , zawiera wszystkie potrzebne deklaracje
#include <avr/io.h>
#include <avr/interrupt.h>
#define PORT PORTD
#define PORT_PIN 3
asd
short kolumna , wiersz = 1;			// Odpowiedzialne za pilnowanie pozycji LCD
short NewKey = 0 ;
short KeyNum = 0 ;

struct TimerParametres
	{
		short MusicNumber : 4;	
		short PlayerLoopCounter : 5;
		short PlayerCounter :16;
		short temp_player_loop_counter : 5;
		short Note_Number : 6;
	}Parametr;

struct Track
	{
	short Note :11;
	short Notelength : 5;
	};

struct MyMelody								// Deklaracja struktury , ktorej obiekt
	{										// to poprostu jeden utwór.
		//double Notes[50];
		//double Notelength[50];
		struct Track Table[100];
		int length : 7;
		char *title;
	}MyMelodyBox[8];

//*********************************************************************************************
				
SIGNAL(SIG_OVERFLOW0)
	{			
		if(!Parametr.PlayerLoopCounter)
			{
				PORT ^= 1<<PORT_PIN	;		// generacja przebiegu
				TCNT0 = 255 - Parametr.PlayerCounter; // prostakatnego
				Parametr.temp_player_loop_counter = Parametr.PlayerLoopCounter;
			}								

		else
			{
				--Parametr.temp_player_loop_counter;
				TCNT0 = 0;
			}
	TIFR = 1<<TOV0;
	}

	
//************************************************************************************
/*
SIGNAL(SIG_OVERFLOW2)
	{
	
		if(!Parametr.temp_time_loop_counter)
			{
				Parametr.PlayerCounter = Count(MyMelodyBox[Parametr.MusicNumber].Notes[Parametr.Note_Number]);
				//temp = Parametr.PlayerCounter;
				TCNT0 = 255 - Parametr.PlayerCounter;
				if(Parametr.PlayerCounter>255)
					{
					Parametr.PlayerLoopCounter=CountLoop(MyMelodyBox[Parametr.MusicNumber].Notes[Parametr.Note_Number]);
					TCNT0 = 0 ;
					}
				Parametr.TimeCounter = Count(1/(MyMelodyBox[Parametr.MusicNumber].Notelength[Parametr.Note_Number]));
			//	temp = Parametr.TimeCounter;
			//	temp2 = MyMelodyBox[0].Notelength[Parametr.Note_Number];
				TCNT2 = 255 - Parametr.TimeCounter;
				Parametr.TimeLoopCounter = 0;
				if(Parametr.TimeCounter >255)
					{
					Parametr.TimeLoopCounter = CountLoop(1/(MyMelodyBox[Parametr.MusicNumber].Notelength[Parametr.Note_Number]));
					TCNT2 = 0;
			//		temp = Parametr.TimeLoopCounter;
					}
			Parametr.temp_time_loop_counter = Parametr.TimeLoopCounter;
			++Parametr.Note_Number;
			}
		else
			{
				--Parametr.temp_time_loop_counter;
				TCNT2 = 0 ;
			}
	TIFR = 1<<TOV2;
	}

(*/
//*********************************************************************************************



