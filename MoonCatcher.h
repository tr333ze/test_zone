#include <avr/interrupt.h>
#define SYS_FREQ 8000000
#define PRESKALER 1024
extern short wiersz ;
extern signed short kolumna;

//********************************************************************************************
// Uzylem jednego timera do odliczania " przypadkowej " pozycji kolejnego pojawienia
// sie ksiezyca oraz do odmierzania czasu jego wyswietlenia ( czas do ktorego gracz powinien
// potrafic dojsc do ksiezyca

SIGNAL(SIG_OVERFLOW0)
	{

		if(!Game.TempTimeLoopCounter)
			{
				Game.NewMoonPosition = 1;
				Game.TempTimeLoopCounter = Game.TimeLoopCounter;
			}
		else
			{
			--Game.TempTimeLoopCounter;
			Game.MoonColPosition_cnt = ( Game.MoonColPosition_cnt != 30 )? ++Game.MoonColPosition_cnt :  0;
			Game.MoonRowPosition_cnt = ( Game.MoonRowPosition_cnt != 2 ) ? ++Game.MoonRowPosition_cnt :  1;
			}
	TIFR = 1<<TOV0;
	}

//********************************************************************************************
// Funkcje ktore wywolane w rezultacie zwracaja wartosci do ktorym musza zliczac timery
// aby otrzymac rzadany czas

int Count(double Value)
	{
		return SYS_FREQ/(PRESKALER*Value)+0.5;
	}
int CountLoop(double Value)
	{
		return (SYS_FREQ/(PRESKALER*Value))/255;
	}

//********************************************************************************************
// Funkcja porownuje nam dwie pozycje , za rowno ksiezyca jak i gracza.
// Jesli sa one rowne wynik zostaje zwiekszony , czas wyswietlania ksiezyca zmniejszony
// o 0.5 [s].

void CheckPositions()
	{
	short temp = wiersz;
	short temp2 = kolumna;
		
	if( kolumna == Game.MoonColPosition )
		{
		if(wiersz == Game.MoonRowPosition)
			{
				if(++Game.Score_l == 10)
					{
					Game.Score_l = 0;
					Game.Score_h = (Game.Score_h != 9)? ++Game.Score_h : 0 ;
					}
			Game.MoonSpeed -= 1;
			Game.TimeLoopCounter = CountLoop(1/(0.1*Game.MoonSpeed));
			Game.TempTimeLoopCounter = Game.TimeLoopCounter;
			LcdMoveTo(2,15);
			LcdViewSign(Game.Score_h+48);
			LcdMoveTo(2,16);
			LcdViewSign(Game.Score_l+48);
			asm("nop");
			MoonRefresh();
			}
		
		}
		
	LcdMoveTo(temp,temp2);
	}
void MoonRefresh()
	{
	if(Game.MoonColPosition_cnt >= 16){Game.MoonColPosition_cnt = 30 - Game.MoonColPosition_cnt;}
	if(Game.MoonColPosition_cnt == kolumna){++Game.MoonColPosition_cnt;}			
				Game.MoonColPosition = Game.MoonColPosition_cnt;
				Game.MoonRowPosition = Game.MoonRowPosition_cnt;										
				LcdMoveTo(Game.MoonRowPosition,Game.MoonColPosition);						
				LcdViewSign(0);	
	}
//********************************************************************************************
