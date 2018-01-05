#include "define.h"
#include "i2c_c1.h"
#include "oled_c1.h"





rom unsigned char CGRAM[2][8] = {{0x04,0x0E,0x15,0x04,0x04,0x04,0x04,0x04}, {0x04,0x15,0x0E,0x04,0x04,0x04,0x04,0x04}};

void OLED_vSetKepad(void)
{
	UPKEY_TRIS = 1;
	DOWNKEY_TRIS = 1;
	LEFTKEY_TRIS = 1;
	RIGHTKEY_TRIS = 1;
	ENTERKEY_TRIS = 1;
	CANCELKEY_TRIS = 1;

	CONVERTERA_STATUS_TRIS = 1;
	CONVERTERB_STATUS_TRIS = 1;
}





void CheckBusy()
{
	unsigned char temp;

	OLED_BUS_W = 0xff;
	OLED_RS = 0;
	OLED_RW = 1;
	OLED_BUS_TRIS = 0xFF;
	do
	{
		OLED_EN = 1;
		Nop(); /* zj */
		temp = OLED_DB7_R;
		OLED_EN = 0;
	}while(temp);
	OLED_BUS_TRIS = 0;
}

void WriteIns(char instruction)
{
	CheckBusy();
	OLED_RS = 0;
	OLED_RW = 0;
	OLED_EN = 1;
	Nop(); /* zj */
	OLED_BUS_W = instruction;
	OLED_EN = 0;
}
void WriteData(char data1)
{
	CheckBusy();
	OLED_RS = 1;
	OLED_RW = 0;
	Nop(); /* zj */
	OLED_EN = 1;
	Nop(); /* zj */
	OLED_BUS_W = data1;
	OLED_EN = 0;
	
	Nop(); /* zj */
}
unsigned char ReadData(void)
{
	unsigned char temp;

	OLED_BUS_TRIS = 0xFF;
	OLED_RS = 1;
	OLED_RW = 1;
	OLED_EN = 1;
	Nop(); /* zj */
	temp = OLED_BUS_R;
	OLED_EN = 0;
	OLED_BUS_TRIS = 0;
	CheckBusy();

	return temp;
}

void WriteString(unsigned char count,unsigned char * MSG)
{
	unsigned char i;
	for(i = 0; i < count; i++)
	{
		if ((MSG[i] >= ' ') && (MSG[i] <= 'z'))
		{
			WriteData(MSG[i]);
		}
	}
}
void LCDWriteString(unsigned char * MSG)
{
	unsigned char temp;
	
	temp = *MSG;
	while (temp)
	{
		if ((temp >= ' ') && (temp <= 'z'))
		{
			WriteData(temp);
		}
		MSG++;
		temp = *MSG;	
	}
}

void DRI_vPrintCGRAM(unsigned char number1)
{
	unsigned char i;
	WriteIns(0x40); //SET CG_RAM ADDRESS 000000
	for(i = 0;i<=24;i++)
	{
		WriteData(CGRAM[number1][i]);
	}
}

void DRI_vLCDClear(void)
{
	WriteIns(0x01); //Clear Display
}

void DRI_vLCDReturnhome(void)
{
	WriteIns(0x02); //Return Home
}
void DRI_vLCDEntryMode(void)
{
	WriteIns(0x06); //Entry Mode Set, address increment & Shift off
}

void DRI_vLCDDisplay(unsigned char displayflag, unsigned char cusorflag, unsigned char blinkflag)
{
	unsigned char temp = 0x08;
	
	if (displayflag == TRUE)
	{
		temp |= 0x04; // display ON
	}
	if (cusorflag == TRUE)
	{
		temp |= 0x02; // cusor ON
	}
	if (blinkflag == TRUE)
	{
		temp |= 0x01; // Blink ON
	}
	WriteIns(temp); //Display ON/OFF control,Display ON,Cursor&Blink OFF
}



void PrintCompanyLCD(void)
{
	unsigned char i, temp, temp1;

	for (i = 0; i < 40; i++)
	{
		temp = 0x80 + i;
		WriteIns(temp);
		temp = LCDPrintBuffer[0][i];
		
		if ((temp >= ' ') && (temp <= 'z'))
		{
			WriteData(temp);
		}
	}
	for (i = 0; i < 40; i++)
	{
		temp = 0xC0 + i;
		WriteIns(temp);
		temp = LCDPrintBuffer[1][i];
		
		if ((temp >= ' ') && (temp <= 'z'))
		{
			WriteData(temp);
		}
	}
	if (EditMode == TRUE)
	{
		// void DRI_vLCDDisplay(unsigned char displayflag, unsigned char cusorflag, unsigned char blinkflag)
		WriteIns(0xC0 + EditPosition);
//		DRI_vLCDDisplay(TRUE, TRUE, FALSE);
	}
}



/* JuZh, Consider only print the blink text part if there is no Menu Index change, provieded the blink location and length */


/* consider only update the value text instead of whole line */


/* Not used any more */
