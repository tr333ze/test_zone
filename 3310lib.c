//--------------------------------------------------------------------- 
//	Biblioteka do obs³ugi wyœwietlacza ze sterownikiem		     	  |	
//					PCD8544(N3310)			|			         	  |
//											|__					 	  |
//											| |	|					  |
//	By Saya		      						| |	|					  |	
//	Plik Ÿród³owy							|___|					  |
//																	  |
//---------------------------------------------------------------------

//####################################################################

#include "3310lib.h"
//####################################################################
/* Funkcja inicjalizujaca wyswietlacz + SPI .
SDIN --> MISO(uC) // MOSI(wyswietlacz)
SCK  --> Sygnal zegarowy , ustawiony na fsys/16
SCE  --> (Linia SS) inaczej Chi Select .Pracuje jako wyjscie , a modul
uruchamiamy stanem niskim
DC   --> Informacja dla sterownika jaka dana wysylany . DC = 0 oznacza
przesylanie instrukcji , = 1 danej do pamieci
Res  --> Reset sterownika , Resetowanie dokonywane jest stanem niskim
Modu³ Spi jest dostosowany do sterownika , ktory probkuje zboczem rosnacym
faza ustawiona na 0 , a czestotliwosc generowanego zegara : fsys/16 */
void LcdInitial()
	{
		DDR(LCD_PORT) = 1<<SDIN | 1<<SCK | 1<<SCE | 1<<DC | 1<<RES;
		PORT(LCD_PORT) = 1<<SCE | 1<<DC | 1<<SCK | 1<<RES; 
		SPCR = 1<<SPE | 1<<MSTR ;//| 1<<SPR0;
		//SPSR = 1<<SPI2X;
		RES_0;
		NopDelay(5);
		RES_1;
		LcdInstr(0x21);
		LcdInstr(TEMP);				// Ustawienie kompensacji temperatury
		LcdInstr(BIAS);				
		LcdInstr(VOP);				// Ustawienie kontrastu
		LcdInstr(0x20);				// Wlaczenie podtsawowego zestawu + 
									// poziome inkrementowanie
		LcdInstr(DisplayAll);	
		LcdClean();					// Pierwsze wyczyszczenie i powrot do
									// poczatku wyswietlacza
	}

//####################################################################
/* Prosta funkcja przesylajaca Bajt przez modul SPI . 
Po przeslaniu funkcja czeka na potwierdzenie przyjecia 
( w przypaduku bledu program stanie w miejscu !! ---- dolaczyc wdt*/
void SendSpi(u8 Bajt)
	{
		SCE_0;
		SPDR = Bajt;
		while((SPSR & (1<<SPIF)) == 0);
		SCE_1;
		SPSR |= 1<<SPSR;
	}

//####################################################################
/* Przeslanie instrukcji do sterownika.
W zasadzie jest to wywolanie funkcji SendSpi(unsigned char) z roznica
wyzerowanej linii DC*/
void LcdInstr(u8 Bajt)
	{
		DC_0;
		SendSpi(Bajt);
		DC_1;
	}
//####################################################################
/* Wypisanie spacji
Prosta petla ktora piec razy przesle bajt o wartosc 0 */
void LcdSpace()
	{
	for(int i = 0 ; i <5 ;i++){SendSpi(0);}
	}



//####################################################################
/* Wypisanie pojedynczego znaku
*/
void LcdViewSign(u8 Sign)
	{
		for(int i = 0 ; i < 6 ; i++)
			{
			SendSpi(pgm_read_byte(&LcdFont[Sign-31][i]));
			}
	}
//####################################################################
/* Wypisanie ciagu znaków bêd¹cego C-Stringiem
Aby uzywac musi byc dolaczony plik naglowkowy 3310libFont.h 
ktory zawiera prawie cala przepisana tablice znakow kodu ASCII.
Funkcja przyjmuje wskaznik na pierwszy znak , nastepnie w petli 
z warunkiem znaku !=0 wypisywane jest kolejnych 6 bajtow .
&LcdFont[Text-32][i] : -32 uzyte dlatego , ¿e tablica pomija
pierwsze 32 znaki symbolizujace w kodzie ASCII rozne operacje.*/
void LcdViewString(u8 *Text)
	{
	u8 temp_sign ;	
		while((temp_sign = pgm_read_byte(Text++)) != 0)
		{
			for(int i = 0 ; i < 6 ;i++)
				{
					SendSpi(pgm_read_byte(&LcdFont[temp_sign-31][i]));
				}
		}
		
	}

//####################################################################
/*Wyrysowanie Bitmapy na wyswietlaczu
Argumenty : 
Ymin --> Wartosc poczatkowa kolumny od ktorego zaczniemy rysowac
( lewy gorny rog BMP )
Xmin --> Poczatkowa wartosc wiersza
Ymax --> Kiedy znamy dokladne wymiary obrazka to mozemy za argument
podac jak wysoki 
Xmax --> To samo jak w poprzednim , dodatkowo mozemy przyciac BMP.
( Ustawiajac Xmax , mniejszy niz szerokosc obrazka)
strcu BMP *BMP --> Struktura do ktorej wskaznik zawiera wszystkie 
potrzebne informacje o BMP ( wskaznik do tablicy , 
rozmiary wymiarow BMP'ka . Uzyta ,zeby nie bylo koniecznosci podawania
zbyt duzej liczby argumentow - brak przejrzystosci.

Na poczatku wyliczne sa wartosci maksymalne ktore obrazuja wielkosc
obrazka , bez skalera . Nastepnie wyliczana jest wierszowa korekcja
w przypadku przycinania music zostac pomijana czesc wyrysowanej tablicy.
Dwa warunki , kiedy nie okreslone zostana wartosci maksymalne 
( w C++ wykorzystac mozna domniemanie ) zostana wyrysowana cala BMP.

 */
void LcdDisplayBmp(u8 Ymin , u8 Xmin ,u8 Ymax,u8 Xmax ,struct BMP *Bmp)
	{
	unsigned char Ymax_temp = Bmp->TableSize / Bmp->TableSize0;
	unsigned char Xmax_temp = Bmp->TableSize0 / Bmp->TableSize00;
	unsigned char BmpCorrect = Xmax_temp - Xmax; 
	if( Ymax ==0 )
	{
		Ymax = ( Ymax_temp >6 )? 6:Ymax_temp;
	}
	if( Xmax ==0 )
	{
		Xmax = ( Xmax_temp > 84) ? 84 :Xmax_temp;
		BmpCorrect = 0;
	}
	unsigned char * temp_wsk = Bmp->BmpPointer;
	for(int i = Ymin ; i < Ymin+Ymax ;i++)
		{
			LcdMoveTo(i,Xmin);
			for(int j = Xmin ; j < Xmax+Xmin ; j++ , temp_wsk++)
				{
				SendSpi(pgm_read_byte(temp_wsk));
				}
			temp_wsk += BmpCorrect;
		}
	}

//####################################################################
/* Czysciciel mosierny wyswietlacza.
Czysci , glaszcze a na koncu wraca na start*/
void LcdClean()
	{
	LcdMoveTo(0,0);
	for(int i = 0 ; i < 504 ; i++){SendSpi(0);}
	LcdMoveTo(0,0);
	}



//####################################################################
/* Wypelnienie wyswietlacz rzadanym znakiem
*/
void LcdFillAll(u8 Sign)
	{
	LcdMoveTo(0,0);
	for(int i = 0 ; i < 84 ;i++)
		{
			LcdViewSign(Sign);
		}
	}

//####################################################################
/*Hop siup i ju¿ jestem w Twoim pamietniku .......
Pierwszym argumentem jest wiersz Y ( 0 - 5 )
Drugim to pozycja w kolumnie X ( 0 - 83). */
void LcdMoveTo(u8 Y, u8 X)
	{
		LcdInstr(SetYAdress | Y);
		LcdInstr(SetXAdress | X);
	}

//####################################################################
/* Hop siup , skok za skokiem
W przeciwienstwie do powyzszej tutaj poruszamy sie precyzyjnie dla tekstu
Przykladowe wywolanie : LcdMoveTo(2,2) spowoduje przejsce do linii drugiej
ale do odpowiadajacej kolumnie drugiemu znakowi*/
void LcdMoveToC(u8 Y,u8 X)
	{
		LcdInstr(SetYAdress | Y);
		LcdInstr(SetXAdress | (X*6));
	}
	

//####################################################################
/* Wyczyszczenie wirutalnego wyswietlacza , oraz wyzerowanie pozycji
*/
void VirtualLcdClean(u8 (*VirutalLcd)[84],uint16_t *X,uint16_t *Y)
	{
	for(int i = 0 ; i < 6 ;i++)
		{
		for(int j = 0 ; j < 84 ;j++)
			{
			VirutalLcd[i][j] = 0 ;
			}
		 }
	u8 Column = 0 ;
	uint16_t Row = 503;
	VirtualLcdWrite(VirutalLcd , &Column , &Row);
	}
	
//####################################################################	
/* Funkcja przepisujaca zawartosc wirualnego wyswietlacza na oryginal
Pierwszym argumenetem jest wskaznik do tablicy dwuwymiarowej ktora
jest orbazem naszego wyswietlacza . Kolejne dwa argumenty sa wyznacznikami
jaki obszar chcemy przepisac ( zawsze calkowite przepisywanie byloby strata
czasu .Zamiast poczatkowych obliczen mozna podac dwa dodatkowe argumenty
ale chcac ,zeby ich lista byla przejrzysta wstepnie jest to w takiej postaci.
Przykladowo za argument podajemy numer bajtu od ktorego startujemy , 
a nastepnie numer na ktorym oknczymy ( jeden znak to start == fisnish)
Przykladowo startowy bajt 97 :

97 % 84 = 13 kolumna
97 / 84 = 1,15 -> obcinamy koncowke i mamy pierwsza wiersz

Taka sama zasada dla wyznaczenia koncowki.
Typem argumentow sa unsigned int 16 bit . Potrzebne nam rozszerzenie
bo wartosc moze przyjmowac wartosci od 0 , 503;
*/
void VirtualLcdWrite(u8 (*VirutalLcd)[84] , uint16_t Start,uint16_t Finish)
	{
	uint16_t start_x = Start  % 84;
	uint16_t finish_x = Finish % 84;
	uint16_t start_y = Start  / 84;
	uint16_t finish_y = Finish / 84;
	for(int i = start_y ; i <= finish_y ;i++)
		{
		LcdMoveTo(i,start_x);
		for(int j = start_x ; j <= finish_x ; j++)
			{
				SendSpi(VirutalLcd[i][j]);
			}
		}
	}
//####################################################################
u8 CheckPix(u8 (*VirutalLcd)[84] , uint16_t PxNumber )
	{
		u8 LittleRow = PxNumber / 84;
		u8 Row = (LittleRow)/8;
		u8 Column = PxNumber % 84;
		u8 BitNumber = LittleRow - (Row)*8;
		u8 Sign = VirutalLcd[Row][Column];
		return (Sign & 1<<BitNumber)==1<<BitNumber?1:0;
	}
//####################################################################
/* Operowanie na poszczegolnych pixelach.
W kolejnosci argumenty:
- wskaznik na tablice obrazujaca wirtualny wyswietlacz
- Numer Piksela . Na ktorym ma byc dokonana operacja.
Formulka to : Pixel = Wiersz ( 0 : 48 ) * 84 + wiersz ( 0 : 84 ).
Przykladowo aby znalezc sie w srodku : Pixel = 24 * 84 + 44
- operacja ( 0 : And , 1 : OR , 2:XOR )
SmallRow to wyliczenie "malego wiersza" ( bazowo jeden wiersz to jeden
bajt czyli moje 8 SmallRow. Podzielenie numeru Pixela przez 84 zwroci
wiersz w ktorym , ów pixel sie znajduje . Nastepnie SmallRow/8 zwraca
rzeczywista wartosc wiersza ( wartosc ktora sluzy do ustalania adresu 
na wyswietlaczu bezposrednio ). Pix % 84 zwraca wartosc kolumny.
Okreslenie pozycji Pixela w wybranym bajcie : 
Przykladowo dla Pixela ktory wystepuje w wierszu 10 .
Podzielenie tej wartosci przez 8 , nie na liczbach przecinkowych
powoduje wystwawienie wartosci rzeczywistego wiersza a * 8 zwroci 
ilosci pixeli calosciowo na tych wierszach . Nastepnie wykonana roznica
pomiedzy wartosci poczatkowa a wartosci wyliczona zwraca ilosc "wystajacych"
pixeli . Przykladowo dla liczby 10 :
	PixPosition = 10 - ((10/8)*8) = 2 .
Wykonanie wybranej operacji i odswiezenie wyswietlacza.
*/
void PaintLcdPix(u8 (*VirtualLcd)[84],uint16_t Pix,u8 Op)
	{
		u8 SmallRow = Pix / 84;			// zwraca wiersz we wierszu
		u8 BigRow = SmallRow / 8 ;
		u8 Column = Pix % 84 ;
		uint16_t BytePosition = ( BigRow * 84 ) + Column;
		u8 PixPosition = SmallRow - ((SmallRow/8)*8);

		u8 Value_temp = VirtualLcd[BigRow][Column];
		if(Op==0){ Value_temp &= (~(1<<PixPosition));}
		if(Op==1){Value_temp |= (1<<PixPosition); }
		if(Op==2){Value_temp ^= (1<<PixPosition); } 
		VirtualLcd[BigRow][Column] = Value_temp;
		VirtualLcdWrite(VirtualLcd ,BytePosition ,BytePosition);
	}

//####################################################################
/* Pisanie Pixelowe .
Argumenty :
	Tablica wirtualna....
	Pxstart - poczatek pisania
	Struktura zamieszoncz w pliku hardef.h 
		struct Font						
		{
			u8 *Font_table;				// Wskaznik do tablicy z czcionka
			u8 SignHeight;				// wysokosc znaku
			u8 SignWidth;				// szerokosc znaku
		};
Znak ktory chcemy wyswietlic ( jest to inaczej wyswielony znak spod
takiej pozycji w tablicy !)
Na poczatku przypisany jest adres do tablicy z czcionka do tymczasowego
obiektu a nastepnie Wsk_temp += (font->SignWidth * Sign) przesuwa 
o odpowiednia liczbe pozycji .
	Przyk³adowo :
			...
			{0x1F, 0x11, 0x1F, 0x00}, // 0
			{0x04, 0x02, 0x1F, 0x00}, // 1
			{0x1D, 0x15,
			....
Wiêc jesli do adresu dodamy szerokosc znaku * pozycja to przesuniemy sie
na odpowiedni znak. Znak jest rysowana od gory do dolu .
Funkcji PaintLcdPix ostatnim argumentem w ktorym wykorzystuje wyliczenie
jest podanie typu operacji . 0 - and , 1 - or wiêc :
  	
			((Temp_Sign & (1<<x)) == 1<<x)?1:0
Sprawdzano sa w petli kolejne bity , i jesli bit jest ustawiony to wynik
wyliczenia zwraca 1 , i tym samy na konkretnej pozycji jest uzywana funkcja
OR;			
	
*/
void LcdPixWrite(u8 (*VirtualLcd)[84],uint16_t Pxstart,struct Font* font, u8 Sign)
	{
		
		u8 Temp_Sign = 0;
		uint16_t PxStartTemp= 0;
		u8 *Wsk_temp = font->Font_table;
		Wsk_temp += (font->SignWidth * Sign);
		for(int j = 0 ; j < font->SignWidth ; j++)
			{
				Temp_Sign = pgm_read_byte(&(*Wsk_temp));
				PxStartTemp = Pxstart;
				for(int x = 0 ; x < font->SignHeight ; x++,PxStartTemp+=84)
					{	
						PaintLcdPix(VirtualLcd,PxStartTemp,(((Temp_Sign & (1<<x)) == 1<<x)?1:0));
					}
				 ++Pxstart;	
				 ++Wsk_temp;
			}
			
	
	}


//####################################################################

void PaintLcdLine(u8 (*VirtualLcd)[84],uint16_t PxStart, uint16_t PxFinish , u8 Op)
	{
	  u8 LineHeight = (PxFinish/84) - (PxStart/84);
	  u8 LineWidth = (PxFinish % 84) - (PxStart % 84);
	  int8_t Signed = ((PxStart % 84) > (PxFinish %84))?-1:1;
//***
	  u8 ingredient_H = 0 ;
	  u8 ingredient_L = 0 ;
	  float H = 0 ;
	  float L = 0 ; 
	  float dx = 0;
	  if(LineHeight >= LineWidth)						// Linia jest dluzsza na dlugosci niz szerokosci
		{
			H = LineWidth;
			L = LineHeight;
			ingredient_H = 1;
			ingredient_L = 84;
		}
	  else												// Linia jest dluzsza na dlugosci niz szerokosci
		{
			H = LineHeight;
			L = LineWidth;
			ingredient_H = 84;
			ingredient_L = 1;
		} 	
	dx = (L / H + 0.5);  
	uint16_t Position = PxStart ;
//	u8 temp = (u8) dx ;
	u8 temp = 0 ;
	for(uint16_t i = PxStart ; i < PxStart + H ; i+= 1)
		{
		
			//for(uint16_t j = PxFinish ; j < PxFinish + L ; j+= ingredient_L , Po)
				//{
					//PaintLcdPix(VirtualLcd,Position,1);
			//	}
		do
			{
				PaintLcdPix(VirtualLcd,Position,1);
				Position+= ingredient_L;
			}
		while((++temp) < (u8) dx);
		Position += ingredient_H;
		temp = 0;
		//	for(u8 j = 0 ; j  <= dx ; j+= 1 , Position+= ingredient_L)
		//	{
				
		//	}
			
		}			
	}
		
	/*
		float step_l = (Pix_fin / ((Pix_st == 0)?1:Pix_st))/84;		
		float step_r = Pix_fin  - (step_l)*((Pix_st == 0)?1:Pix_st)*84;
		float Step_1 = (step_l > step_r )?step_l:step_r;
		float Step_2 = (step_l > step_r )?step_r:step_l;
		float step = 0 ;
	
		u8 step_c = 0 ;
		u8 step_d = 0 ;
		if(Step_1 > Step_2){step = Step_1/Step_2;step_c = 1;step_d = 84;}
		else{step = Step_2/Step_1;step_c=84;step_d=1;}
		for(float i = 0 ; i < Step_2 ;i++)
			{
			//	for(float j = 0 ; j <= Step_1 ; j+=step )
			//	{
			for(int j = 0 ; j < step ; j++ ,Pix_st+=step_c)
				{
					PaintLcdPix(VirtualLcd,Pix_st,1);
				}
			PaintLcdPix(VirtualLcd,Pix_st+=step_d,1);
			}*/

	



//####################################################################
/* Funkcja rysujaca prostokat
Argumentami sa : wirtualna tablica , oraz poczatek i koniec prostokata.
( Dwa przeciwne koñce )
Sa rozpatrywane dwa przypadki w zaleznosci od wartosci podanych jako poczatek i koniec.
Bo mozna podac jako pierwszy :
		1
			120
			
A mo¿e ktos podaæ
		120					Dla takiego przypadku zostanie wprowadzona korekcja
			1
u8 y = FinishRow - StartRow - oblicza roznice w wierszach czyli jak wysoki
musi jest prostokat.
u8 x = (PxFinish % 84) - (PxStart % 84) ; zwraca roznice w dlugosci bokow
co daje szerokosc.
Nastepnie dwie petle rysuja naraz po dwa boki.
Op - to operacja ktora bedzie przeprowadzana na pixelach podczas rysowania bokach.
	0 - And : wymazanie prostokata o podanych wymiarach
	1 - Or - wyrysowanie
	2 - Xor
*/
void PaintLcdRect(u8 (*VirtualLcd)[84],uint16_t PxStart,uint16_t PxFinish,u8 Op)
	{
	u8 StartRow  = 0;
	u8 FinishRow = 0;
	int8_t ingredient = 1;
	StartRow = PxStart/84;
	FinishRow = PxFinish/84;
	u8 y = FinishRow - StartRow;
	u8 x = (PxFinish % 84) - (PxStart % 84) ;
	for(int i = PxStart ; i <= PxStart + x ; i+= ingredient)
		{
			PaintLcdPix(VirtualLcd,i,1);
			PaintLcdPix(VirtualLcd,i+y*84,1);

		}
	for(int i = PxStart ; i <= PxStart + y*84 ; i+= ingredient*84)
		{
			PaintLcdPix(VirtualLcd,i,1);
			PaintLcdPix(VirtualLcd,x+i,1);
		}
		
	}
//####################################################################	
/* Rysowanie okregu o zadanym poczatku , oraz promieniu.
Rysowanie jest rozpoczynane od gory na schemat
		   x*x
		 x	   x	
		*	*   *
          x   x			
		   x*x
* sa to punkty odlegle o Ray od srodka. Schemat rysowania luku wyglada nastepujaco:
Rysujemy punkt * nastepnie jeden do niego przylegly i po skosie do nastepnego.
W zaleznosci od ktorego do ktorego punktu nastepuje rysowanie to trzeba odejmowac 
lub dodawac 84 ( przesuwanie w pionie ) , lub dodawac / odejmowac 1 ( przesuwanie
w pozciomie )
*/
PaintLcdCircle(u8 (*VirtualLcd)[84],uint16_t Center,u8 Ray,u8 Op)
	{
	uint16_t TempPosition = Center - 84 ; 				// Rozpoczecie rysowanie od gory
	int8_t Sign_x = 1;
	int8_t Sign_y = 1; 
	// Wyrysowanie kolejno czterech lukow
	for(register u8 i = 0 ; i < 4 ;i++)
		{
			for(register u8 j = 0 ; j < Ray + 1 ; j++ )
				{
					PaintLcdPix(VirtualLcd,TempPosition,1);
					TempPosition = (Center - Ray*84*Sign_y) + (j+1)*Sign_x +j*84*Sign_y;
				}
		    Sign_x *= -1;	
		    Sign_y = (i < 2)? 1:-1;//\\
			TempPosition = ( i < 2)? Center - 84 : Center+84;	
		}
}
//	``//Sign_x = (i % 2) == 0 ? 1 : -1;
//	``
//	``}
//####################################################################
/*Dodatkowa funkcja aktualizuja czas na wyswietlaczu.
Argumenty :
Tablica wirtualnie odzwierciedlajaca wyswietlacz poczatek wyswietlania
Struktura zamieszoncz w pliku hardef.h 
		struct Font						
		{
			u8 *Font_table;				// Wskaznik do tablicy z czcionka
			u8 SignHeight;				// wysokosc znaku
			u8 SignWidth;				// szerokosc znaku
		};
Struktura do wartosci zliczonego czasu :
	struct Time
		{
			u8 seconds  : 6;
			u8 minutes  : 6;
			u8 hours	   : 5;
			u8 Day_week : 3;
			u8 Day_month: 5;
			u8 Month    : 4;
			u8 year	   : 5;
			u8 Day_part : 2;
		};
Na poczatku dzielona sa wartosc na jednosci i dziesiatki.
Nastepnie tworzona jest tablica wskaznikow do nazw dni tygodnia.
Makro PSTR zwraca wsakznik do pamieci programu ( funcjah LcdViewString korzysta
z makra pgm_read_byte).
Nastepnie wyliczane sa ilossci znakow zarowno dla godziny jak i dla daty ,
natomiast warunek (!((i+1)%2) wyrysowuje co dwa znaki ':' dla godziny 
i '.' dla daty.

	*/
/*void LcdTimeView(u8 (*VirtualLcd)[84],uint16_t Pxstart,struct Font* font, struct Time* time , u8 *days_n[])
	{
		u8 hours[] = { time->hours/10 , time->hours%10 , time->minutes/10 , time->minutes%10,
								time->seconds/10,time->seconds%10};
								
		u8 days[] = {    time->Day_month/10,	time->Day_month%10,time->Month/10,
								time->Month%10,time->year/10,time->year%10 };				
		
		u8 temp_hours = sizeof(hours)/sizeof(u8);
		uint16_t Pxstart_temp = Pxstart;
		u8 Cnt = 2;
		
		
		for(int i = 0 ; i <temp_hours ; i++ , Pxstart_temp+= font->SignWidth)
			{
				LcdPixWrite(VirtualLcd,Pxstart_temp,font, hours[i]);
				if(!((i+1)%2) && Cnt!=0){Pxstart_temp+= font->SignWidth;LcdPixWrite(VirtualLcd,Pxstart_temp,font, 10);--Cnt;}
			
			}
			
		u8 temp_days = sizeof(days)/sizeof(u8);
		Pxstart_temp = Pxstart+45;
		Cnt = 2;
		
		
		for(int i = 0 ; i <temp_days ; i++ , Pxstart_temp+= font->SignWidth)
			{
				LcdPixWrite(VirtualLcd,Pxstart_temp,font, days[i]);
				if(!((i+1)%2) && Cnt!=0){Pxstart_temp+= font->SignWidth;LcdPixWrite(VirtualLcd,Pxstart_temp,font, 11);--Cnt;}
			
			}
		LcdMoveTo(5,0);
		LcdViewString(days_n[time->Day_week-1]);					
								
								
	}
*/












