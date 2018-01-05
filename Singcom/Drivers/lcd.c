#include "lcd.h"
#include "oled_c1.h"
#include "define.h"
#include "Global.h"
#include <string.h>

#include "DeviceApi.h"
#include "i2c_c1.h"
#include "Util.h"
#include "string.h"

static unsigned char CurrentMenu = 0;
static unsigned char LastMenu = 0;
static unsigned char LeftMostMenu = 0;
unsigned char EditMode = FALSE, EditPosition = 0;
char LCDPrintBuffer[2][nLCDLEN];
	const char zeros[]={"0000000000"};

typedef struct{
	char MenuTitle[18];
	char MenuTitleShort[12];
	unsigned char DisplayInSubMenu;		//whether menu and value should be displayed in the second row
	char FixedText[18];
	BYTE u8RightMenu;
	BYTE u8LeftMenu;
	BYTE u8UpMenu;
	BYTE u8DownMenu; 
    unsigned char Editable;		//1=editable, 
	unsigned char ValueType;	//0=ISNUMBER, 1=ISTEXT, 2=NIL
	char DigitsEditable[16];		//if the value is a number, indicate which digit can be edit
	char TextValue0[10];
	char TextValue1[10];
	unsigned char DisplaySign;
	char DecimalPoint[8];
}tMenuDisplay;

typedef struct{
	int	ValueH;
	long Value;			//For ValueType ISTEXT, Value=1 display TextValue1, Value=2 display TextValue2;
	int MaxValueH;
	long MaxValue;
	int MinValueH;
	long MinValue;
	unsigned char UpdatedFromMenu;	//To inform other programs that the value has been updated by keypad/menu.
	unsigned char ToBeUpdated;		//To inform LCD program to update the value which has been changed by other program.
}tMenuValue;

BYTE LCDRefreshFlag = FALSE; //update whole Menu
BYTE LCDDigitUpdateFlag = FALSE; //Update one digit only

const rom tMenuDisplay MenuDisplay[MENUCOUNT]=
{
/*0*/	{"[DOWN CONVERTER] ","DC ",{TRUE},"",1,2,0,3,FALSE,NIL,{0},"NIL","NIL",FALSE,{0}},
/*1*/	{"[MAIN CTRL] ","CTRL ",{TRUE},"",2,0,1,10,FALSE,NIL,{0},"NIL","NIL",FALSE,{0}},
/*2*/	{"[DEVICE INFO] ","INFO ",{TRUE},"",0,1,2,11,FALSE,NIL,{0},"NIL","NIL",FALSE,{0}},
/*3*/	{"[RF FREQ]","RF ",{TRUE},"0000.000MHz ",4,9,0,3,TRUE,ISNUMBER,{8,7,6,4,3,2,1,0},"NIL","NIL",FALSE,{4}},
/*4*/	{"[POWER]","PWER ",{TRUE},"+00.0dBm ",5,3,0,4,FALSE,ISNUMBER,{5,3,2,1,0},"NIL","NIL",TRUE,{3}},
/*5*/	{"[SET POWER]","SET POWER ",{FALSE},"+00.0dBm ",6,4,0,5,TRUE,ISNUMBER,{5,3,2,1,0},"NIL","NIL",TRUE,{3}},
/*6*/	{"[LO STATUS]","LO ",{TRUE},"UNLOCKED ",7,5,0,6, FALSE,ISTEXT,{0},"UNLOCKED ","LOCKED   ",FALSE,{0}},
/*7*/	{"[ATTN]","ATTN ",{FALSE},"00.0dB ",8,6,0,7, TRUE,ISNUMBER,{4,2,1,0},"NIL","NIL",FALSE,{2}},
/*8*/	{"[ALC]","ALC ",{FALSE},"ON  ",9,7,0,8, TRUE,ISTEXT,{0},"OFF ","ON  ",FALSE,{0}},
/*9*/	{"[RF ENBL]","RF ENBL ",{FALSE},"ENABLED  ",3,8,0,9, TRUE,ISTEXT,{0},"DISABLED ","ENABLED  ",FALSE,{0}},
/*10*/	{"[REF]","REF ",{TRUE},"INTERNAL ",10,10,1,10, FALSE,ISTEXT,{0},"INTERNAL ","EXTERNAL ",FALSE,{0}},
/*11*/	{"[IP]","IP ",{TRUE},"192.168.001.001 ",12,13,2,11, TRUE,ISNUMBER,{15,14,13,11,10,9,7,6,5,3,2,1,0},"NIL","NIL",FALSE,{3,7,11,15}},
/*12*/	{"[MODEL NUMBER]","MODEL ",{TRUE},"0000000000 ",13,11,2,12, FALSE,ISTEXT,{0},"NIL","NIL",FALSE,{0}},
/*13*/	{"[SERIAL NUMBER]","SERIAL ",{FALSE},"00000000 ",11,12,2,13, FALSE,ISNUMBER,{0},"NIL","NIL",FALSE,{0}},
};

tMenuValue MenuValue[MENUCOUNT]=
{
/*0*/	{0,0,0,0,0,0,0,0},
/*1*/	{0,0,0,0,0,0,0,0},
/*2*/	{0,0,0,0,0,0,0,0},
/*3*/	{0,950000,0,2150000,0,950000,0,0},
/*4*/	{0,0,0,20,0,-30,0,0},
/*5*/	{0,0,0,20,0,-30,0,0},
/*6*/	{0,0,0,1,0,0,0,0},
/*7*/	{0,0,0,31,0,0,0,0},
/*8*/	{0,0,0,1,0,0,0,0},
/*9*/	{0,0,0,1,0,0,0,0},
/*10*/	{0,0,0,1,0,0,0,0},
/*11*/	{192,168001001,999,999999999,0,0,0,0},
/*12*/	{0,0,0,0,0,0,0,0},
/*13*/	{0,0,0,0,0,0,0,0},
};

void KeyProcessing(void)
{
	char tempString[20];
	switch (PressedKey)
	{
		case UPKEY:
		{
			if (EditMode)
			{
				if (MenuDisplay[CurrentMenu].ValueType==ISNUMBER)
				{
					if ((LCDPrintBuffer[1][EditPosition] >= '0') && (LCDPrintBuffer[1][EditPosition] < '9'))
	    			{
	        			LCDPrintBuffer[1][EditPosition]++;
	    			}
	    			else if (LCDPrintBuffer[1][EditPosition] == '9')
	    			{
	        			LCDPrintBuffer[1][EditPosition] = '0';
	    			}
	    			else if (LCDPrintBuffer[1][EditPosition] == '-')
	    			{
	        			LCDPrintBuffer[1][EditPosition] = '+';
	    			}
	    			else if (LCDPrintBuffer[1][EditPosition] == '+')
	    			{
	        			LCDPrintBuffer[1][EditPosition] = '-';
					}
				}
				else if (MenuDisplay[CurrentMenu].ValueType==ISTEXT)
				{
					if ((MenuDisplay[CurrentMenu].TextValue0[0]==LCDPrintBuffer[1][0])
						&&(MenuDisplay[CurrentMenu].TextValue0[1]==LCDPrintBuffer[1][1]))
					{
						strcpypgm2ram(LCDPrintBuffer[1], MenuDisplay[CurrentMenu].TextValue1);
					}
					else if ((MenuDisplay[CurrentMenu].TextValue1[0]==LCDPrintBuffer[1][0])
						&&(MenuDisplay[CurrentMenu].TextValue1[1]==LCDPrintBuffer[1][1]))
					{
						strcpypgm2ram(LCDPrintBuffer[1], MenuDisplay[CurrentMenu].TextValue0);
					}
				}
				LCDDigitUpdateFlag=1;
			}
			else
			{
				if (MenuDisplay[CurrentMenu].u8UpMenu!=CurrentMenu)
				{
					CurrentMenu=MenuDisplay[CurrentMenu].u8UpMenu;
					LeftMostMenu=CurrentMenu;
					LCDRefreshFlag=1;
				}
			}
		}
		break;
		case DOWNKEY:
		{
			if (EditMode)
			{
				if (MenuDisplay[CurrentMenu].ValueType==ISNUMBER)
				{
					if ((LCDPrintBuffer[1][EditPosition] > '0') && (LCDPrintBuffer[1][EditPosition] <= '9'))
	    			{
	        			LCDPrintBuffer[1][EditPosition]--;
	    			}
	    			else if (LCDPrintBuffer[1][EditPosition] == '0')
	    			{
	        			LCDPrintBuffer[1][EditPosition] = '9';
	    			}
	    			else if (LCDPrintBuffer[1][EditPosition] == '-')
	    			{
	        			LCDPrintBuffer[1][EditPosition] = '+';
	    			}
	    			else if (LCDPrintBuffer[1][EditPosition] == '+')
	    			{
	        			LCDPrintBuffer[1][EditPosition] = '-';
					}
				}
				else if (MenuDisplay[CurrentMenu].ValueType==ISTEXT)
				{
					if ((MenuDisplay[CurrentMenu].TextValue0[0]==LCDPrintBuffer[1][0])
						&&(MenuDisplay[CurrentMenu].TextValue0[1]==LCDPrintBuffer[1][1]))
					{
						strcpypgm2ram(LCDPrintBuffer[1], MenuDisplay[CurrentMenu].TextValue1);
					}
					else if ((MenuDisplay[CurrentMenu].TextValue1[0]==LCDPrintBuffer[1][0])
						&&(MenuDisplay[CurrentMenu].TextValue1[1]==LCDPrintBuffer[1][1]))
					{
						strcpypgm2ram(LCDPrintBuffer[1], MenuDisplay[CurrentMenu].TextValue0);
					}
				}	
				LCDDigitUpdateFlag=1;			
			}
			else
			{
				if (MenuDisplay[CurrentMenu].u8DownMenu!=CurrentMenu)
				{
					CurrentMenu=MenuDisplay[CurrentMenu].u8DownMenu;
					LeftMostMenu=CurrentMenu;
					LCDRefreshFlag=1;
				}
			}
		}
		break;
		case LEFTKEY:
		{
			if (EditMode)
			{
				if (MenuDisplay[CurrentMenu].ValueType==ISNUMBER)
				{
					unsigned char i=0,j,k;
					strcpypgm2ram(tempString,MenuDisplay[CurrentMenu].DigitsEditable);
					j=strlen(tempString);
					while (EditPosition!=MenuDisplay[CurrentMenu].DigitsEditable[i]-1)
					{
						i++;
					}
					k=1;
					if (EditPosition==MenuDisplay[CurrentMenu].DigitsEditable[j-1]-1)
					{
						EditPosition=MenuDisplay[CurrentMenu].DigitsEditable[0]-1;
					}
					else
					{
						EditPosition=MenuDisplay[CurrentMenu].DigitsEditable[i+1]-1;
					}
			    	WriteIns(0xC0 + EditPosition);
    				DRI_vLCDDisplay(TRUE, TRUE, FALSE); 
				}
			}
			else
			{
				LCDRefreshFlag=1;
				if (MenuDisplay[CurrentMenu].u8LeftMenu!=CurrentMenu)
				{
					CurrentMenu=MenuDisplay[CurrentMenu].u8LeftMenu;
				}
				if (LCDMenuInScreen()==0)
				{
					LeftMostMenu=CurrentMenu;
				}
			}
		}
		break;
		case RIGHTKEY:
		{
			if (EditMode)
			{
				if (MenuDisplay[CurrentMenu].ValueType==ISNUMBER)
				{
					unsigned char i=0,j;
					strcpypgm2ram(tempString,MenuDisplay[CurrentMenu].DigitsEditable);
					j=strlen(tempString);
					while (EditPosition!=MenuDisplay[CurrentMenu].DigitsEditable[i]-1)
					{
						i++;
					}
					if (EditPosition==MenuDisplay[CurrentMenu].DigitsEditable[0]-1)
					{
						EditPosition=MenuDisplay[CurrentMenu].DigitsEditable[j-1]-1;
					}
					else
					{
						EditPosition=MenuDisplay[CurrentMenu].DigitsEditable[i-1]-1;
					}
			    	WriteIns(0xC0 + EditPosition);
    				DRI_vLCDDisplay(TRUE, TRUE, FALSE); 
				}
			}
			else
			{
				LCDRefreshFlag=1;
				if (MenuDisplay[CurrentMenu].u8RightMenu!=CurrentMenu)
				{
					CurrentMenu=MenuDisplay[CurrentMenu].u8RightMenu;
				}
				while (LCDMenuInScreen()==0)
				{
					LeftMostMenu=MenuDisplay[LeftMostMenu].u8RightMenu;
				}
			}
		}
		break;
		case ENTERKEY: 
		{
			if (MenuDisplay[CurrentMenu].Editable==TRUE)
			{
				if (EditMode==TRUE)
				{
					EditMode = FALSE;
		    		EditPosition = 0;
				    DRI_vLCDDisplay(TRUE, FALSE, FALSE);
					if (MenuDisplay[CurrentMenu].ValueType==ISNUMBER)
					{
						long temp,tempH;
						temp=Util_u8String2int32_noDotL(LCDPrintBuffer[1]);
						tempH=Util_u8String2int32_noDotH(LCDPrintBuffer[1]);
						if (
							((temp<=MenuValue[CurrentMenu].MaxValue) || (tempH<=MenuValue[CurrentMenu].MaxValueH))
							&&((temp>=MenuValue[CurrentMenu].MinValue)||(tempH>=MenuValue[CurrentMenu].MinValueH))
							&& ((tempH!=MenuValue[CurrentMenu].ValueH)||(temp!=MenuValue[CurrentMenu].ValueH))
							)
						{
							MenuValue[CurrentMenu].Value=temp;
							MenuValue[CurrentMenu].ValueH=tempH;
							MenuValue[CurrentMenu].UpdatedFromMenu=TRUE;
						}
					}
					else if (MenuDisplay[CurrentMenu].ValueType==ISTEXT)
					{
						if ((MenuDisplay[CurrentMenu].TextValue0[0]==LCDPrintBuffer[1][0])
							&&(MenuDisplay[CurrentMenu].TextValue0[1]==LCDPrintBuffer[1][1]))
						{
							if (MenuValue[CurrentMenu].Value!=1)
							{
								MenuValue[CurrentMenu].Value=1;
								MenuValue[CurrentMenu].UpdatedFromMenu=1;
							}
						}
						else if ((MenuDisplay[CurrentMenu].TextValue1[0]==LCDPrintBuffer[1][0])
							&&(MenuDisplay[CurrentMenu].TextValue1[1]==LCDPrintBuffer[1][1]))
						{
							if (MenuValue[CurrentMenu].Value!=2)
							{
								MenuValue[CurrentMenu].Value=2;
								MenuValue[CurrentMenu].UpdatedFromMenu=1;
							}
						}					
					}
				}
				else
				{
					EditMode=TRUE;
	    			EditPosition = 0;
				    WriteIns(0xC0 + EditPosition);
	    			DRI_vLCDDisplay(TRUE, TRUE, FALSE);  
				}
			}
		}
		break;
		case CANCELKEY:
		{
			if (EditMode==TRUE)
			{
		    	DRI_vLCDDisplay(TRUE, FALSE, FALSE);
    			EditMode = FALSE;
    			EditPosition = 0;
			}
		}
		break;
		default :
		break;
	}
}

unsigned char LCDMenuInScreen(void)
{
	unsigned char TempMenuLength=0,TempMenuLength1;
	unsigned char MenuTemp;
	char tempString[];
	MenuTemp=LeftMostMenu;
	strcpypgm2ram(tempString,MenuDisplay[MenuTemp].MenuTitle);
	TempMenuLength=strlen(tempString);
	while (MenuTemp!=CurrentMenu)
	{
		MenuTemp=MenuDisplay[MenuTemp].u8RightMenu;
		strcpypgm2ram(tempString,MenuDisplay[MenuTemp].MenuTitle);
		TempMenuLength1=strlen(tempString);
		TempMenuLength=TempMenuLength+TempMenuLength1;
	}
	MenuTemp=MenuDisplay[CurrentMenu].u8UpMenu;
	if (MenuTemp!=CurrentMenu)
	{
		strcpypgm2ram(tempString,MenuDisplay[MenuTemp].MenuTitleShort);
		TempMenuLength=TempMenuLength+strlen(tempString);
	}
	if (TempMenuLength>=40)
	{
		return(0);
	}
	else
	{
		return(1);
	}
}

void PrintLCDEditPosition(void)
{
	unsigned char i, temp;    
	WriteIns(0xC0 + EditPosition);
	DRI_vLCDDisplay(TRUE, FALSE, FALSE);  
    WriteIns(0xC0+EditPosition);
    if (MenuDisplay[CurrentMenu].ValueType==ISNUMBER)
	{
		temp = LCDPrintBuffer[1][EditPosition];
        if ((temp >= ' ') && (temp <= 'z'))
        {
            WriteData(temp);
        }
		if (EditMode==TRUE)
		{
		    WriteIns(0xC0 + EditPosition);
			DRI_vLCDDisplay(TRUE, TRUE, FALSE);  
		}
	}
	else
	{			
		for (i = 0; i < 9; i++) /* Only re-print the cursor value and 9 more characters after it */
		{
			temp = LCDPrintBuffer[1][EditPosition + i];
			if ((temp >= ' ') && (temp <= 'z'))
			{
				WriteData(temp);
			}
		} 
	}   
}

void PrintLCD(void)
{
	unsigned char i, temp;
	static unsigned char BlinkingFlag=0;
	static TICK tiDisplay_tmr=0;
	unsigned char MenuTemp,TempMenuLength,TempBlinkingPos;
	char tempString[20];
	if ((TickGet() - tiDisplay_tmr) >= (TICK_SECOND /3 ))
	{
		tiDisplay_tmr = TickGet();

// check blink position
		MenuTemp=LeftMostMenu;
		strcpypgm2ram(tempString,MenuDisplay[MenuTemp].MenuTitle);
		TempMenuLength=strlen(tempString);
		TempBlinkingPos=0;
		while (MenuTemp!=CurrentMenu)
		{
			TempBlinkingPos=TempBlinkingPos+TempMenuLength;
			MenuTemp=MenuDisplay[MenuTemp].u8RightMenu;
			strcpypgm2ram(tempString,MenuDisplay[MenuTemp].MenuTitle);
			TempMenuLength=strlen(tempString);
		}
		MenuTemp=MenuDisplay[CurrentMenu].u8UpMenu;
		if (MenuTemp!=CurrentMenu)
		{
			strcpypgm2ram(tempString,MenuDisplay[MenuTemp].MenuTitleShort);
			TempBlinkingPos=TempBlinkingPos+strlen(tempString);
		}
			

		if (BlinkingFlag==0)
		{
			BlinkingFlag=1;
			WriteIns(0x80+TempBlinkingPos);
			Nop();
			for (i = TempBlinkingPos; i < TempBlinkingPos+TempMenuLength; i++)
			{
				temp=' ';
				WriteData(temp);
			}
		}
		else
		{
			BlinkingFlag=0;
			WriteIns(0x80+TempBlinkingPos);
			Nop();
			for (i = TempBlinkingPos; i < TempBlinkingPos+TempMenuLength; i++)
			{
				temp = LCDPrintBuffer[0][i];
				WriteData(temp);
			}
		}
		if (EditMode==TRUE)
		{
		    WriteIns(0xC0 + EditPosition);
			DRI_vLCDDisplay(TRUE, TRUE, FALSE);  
		}
		else
		{
		    WriteIns(0xC0 + EditPosition);
			DRI_vLCDDisplay(TRUE, FALSE, FALSE);
		}
			
	}


	if (LCDRefreshFlag==1)
	{
		PrintLCDWholePage();
		LCDRefreshFlag=0;
	}
	if (LCDDigitUpdateFlag==1)
	{
		PrintLCDEditPosition();
		LCDDigitUpdateFlag=0;
	}
}

void PrintLCDWholePage(void)
{
	unsigned char i, temp, temp1,BlinkingFlag=0;
	unsigned char BufferTemp[40];
	char tempString[];
	LCDDisplayLoadData();
	strncpy(BufferTemp,LCDPrintBuffer[0],40);

			
	WriteIns(0x80);
	Nop();
	for (i = 0; i < 40; i++)
	{
		temp = LCDPrintBuffer[0][i];
		if ((temp >= ' ') && (temp <= 'z'))
		{
			WriteData(temp);
		}
		else if (temp==0)
		{
			WriteData(' ');
		}
	}

	WriteIns(0xC0);
	Nop();
	for (i = 0; i < 40; i++)
	{
		temp = LCDPrintBuffer[1][i];
		if ((temp >= ' ') && (temp <= 'z'))
		{
			WriteData(temp);
		}
		else if (temp==0)
		{
			WriteData(' ');
		}
	}
	if (EditMode==TRUE)
	{
	    WriteIns(0xC0 + EditPosition);
		DRI_vLCDDisplay(TRUE, TRUE, FALSE);  
	}
	else
	{
	    WriteIns(0xC0 + EditPosition);
		DRI_vLCDDisplay(TRUE, FALSE, FALSE);
	}

}

void PrintLCDFromBuffer(void)
{
	unsigned char i, temp;

	WriteIns(0x80);
	Nop();
	for (i = 0; i < 40; i++)
	{
		temp = LCDPrintBuffer[0][i];
		if ((temp >= ' ') && (temp <= 'z'))
		{
			WriteData(temp);
		}
	}

	WriteIns(0xC0);
	Nop();
	for (i = 0; i < 40; i++)
	{
		temp = LCDPrintBuffer[1][i];
		if ((temp >= ' ') && (temp <= 'z'))
		{
			WriteData(temp);
		}
	}
}

void LCDDisplayLoadData(void)
{
	unsigned char SubMenu;
	unsigned char AppenSize, tempSize=0,k=0,tempMenu=0;
	int i=0,j=0;
	unsigned char TempDisplay[20];
	unsigned char TempMenuTitle[20];
	unsigned char TempDigitsEditable[14];
	char tempValue[20];
	char tempValue1[18];
	char tempValue2[18];
	char tempString[];

// clear buffer
	for (j=0;j<40;j++)
	{
		LCDPrintBuffer[0][j]=' ';
		LCDPrintBuffer[1][j]=' ';
	}


//first menu in the first row
	if (MenuDisplay[LeftMostMenu].u8UpMenu==LeftMostMenu)
	{		
		strcpypgm2ram(LCDPrintBuffer[0], MenuDisplay[LeftMostMenu].MenuTitle);
		tempMenu=LeftMostMenu;
		tempSize=strlen(LCDPrintBuffer[0]);
		tempMenu=MenuDisplay[LeftMostMenu].u8RightMenu;
		strcpypgm2ram(TempMenuTitle,MenuDisplay[tempMenu].MenuTitle);
		tempSize=tempSize+strlen(TempMenuTitle);
	}
	else
	{
		tempMenu=MenuDisplay[LeftMostMenu].u8UpMenu;
		strcpypgm2ram(LCDPrintBuffer[0],MenuDisplay[tempMenu].MenuTitleShort);
		tempSize=strlen(LCDPrintBuffer[0]);
		tempMenu=LeftMostMenu;
		strcpypgm2ram(TempMenuTitle, MenuDisplay[LeftMostMenu].MenuTitle);
		tempSize=tempSize+strlen(TempMenuTitle);
	}
		strcat(LCDPrintBuffer[0], TempMenuTitle);

	while ((tempSize<40)&& ((MenuDisplay[tempMenu].u8RightMenu)!=LeftMostMenu))
	{
		i++;
		tempMenu=MenuDisplay[tempMenu].u8RightMenu;
		strcpypgm2ram(TempMenuTitle,MenuDisplay[tempMenu].MenuTitle);
		AppenSize=strlen(TempMenuTitle);
		if (tempSize+AppenSize>40)
		{
			AppenSize=40-tempSize;
		}
		tempSize=tempSize+AppenSize;
		strncat(LCDPrintBuffer[0],TempMenuTitle,AppenSize);
	}


//if the menu has sub menus, print sub menus and the corresponding values
	if (MenuDisplay[CurrentMenu].u8DownMenu!=CurrentMenu)
	{
		strcpypgm2ram(LCDPrintBuffer[1], MenuDisplay[CurrentMenu].MenuTitleShort);
		SubMenu=MenuDisplay[CurrentMenu].u8DownMenu;
		tempMenu=SubMenu;
		tempSize=0;

		if (MenuDisplay[tempMenu].DisplayInSubMenu==TRUE)
		{
			strcatpgm2ram(LCDPrintBuffer[1],MenuDisplay[tempMenu].MenuTitleShort);
			strcpypgm2ram(tempValue,MenuDisplay[tempMenu].FixedText);
			if (MenuDisplay[tempMenu].DisplaySign)
			{
				sprintf(tempValue1,"%+ld",MenuValue[tempMenu].Value) ;
			}
			else
			{
				if (MenuValue[tempMenu].ValueH==0)
				{
					sprintf(tempValue1,"%ld",MenuValue[tempMenu].Value) ;
				}
				else
				{
					sprintf(tempValue1,"%d",MenuValue[tempMenu].ValueH) ;
					sprintf(tempValue2,"%ld",MenuValue[tempMenu].Value) ;
					if (strlen(tempValue2)<9)
					{
						strncat(tempValue1,zeros,9-strlen(tempValue2));
					}
					strcat(tempValue1,tempValue2);
				}
//				sprintf(tempValue1,"%ld%ld",MenuValue[tempMenu].ValueH, MenuValue[tempMenu].Value) ;
			}
			strcpypgm2ram(TempDigitsEditable,MenuDisplay[tempMenu].DigitsEditable);
			Value2DisplayBuffer(tempValue, tempValue1,TempDigitsEditable);
			strcat(LCDPrintBuffer[1],tempValue);
			tempSize=strlen(LCDPrintBuffer[1]);
		}
		tempMenu=MenuDisplay[tempMenu].u8RightMenu;

		while ((MenuDisplay[tempMenu].u8RightMenu!=SubMenu) && tempSize<40)
		{
			if (MenuDisplay[tempMenu].DisplayInSubMenu==TRUE)
			{
				strcatpgm2ram(LCDPrintBuffer[1],MenuDisplay[tempMenu].MenuTitleShort);
				strcpypgm2ram(tempValue,MenuDisplay[tempMenu].FixedText);
				if (MenuDisplay[tempMenu].DisplaySign)
				{
					sprintf(tempValue1,"%+ld",MenuValue[tempMenu].Value) ;
				}
				else
				{
					if (MenuValue[tempMenu].ValueH==0)
					{
						sprintf(tempValue1,"%ld",MenuValue[tempMenu].Value) ;
					}
					else
					{
						sprintf(tempValue1,"%d",MenuValue[tempMenu].ValueH) ;
						sprintf(tempValue2,"%ld",MenuValue[tempMenu].Value) ;
						if (strlen(tempValue2)<9)
						{
							strncat(tempValue1,zeros,9-strlen(tempValue2));
						}
						strcat(tempValue1,tempValue2);
					}
	//				sprintf(tempValue1,"%ld%ld",MenuValue[tempMenu].ValueH, MenuValue[tempMenu].Value) ;
				}
				strcpypgm2ram(TempDigitsEditable,MenuDisplay[tempMenu].DigitsEditable);
				Value2DisplayBuffer(tempValue, tempValue1,TempDigitsEditable);
				strcat(LCDPrintBuffer[1],tempValue);
				tempSize=strlen(LCDPrintBuffer[1]);
			}
			tempMenu=MenuDisplay[tempMenu].u8RightMenu;
		}

	}

//Print value of the menu
	else
	{
		if (MenuDisplay[CurrentMenu].ValueType==ISNUMBER)
		{
			strcpypgm2ram(tempValue,MenuDisplay[CurrentMenu].FixedText);
			if (MenuDisplay[CurrentMenu].DisplaySign)
			{
				sprintf(tempValue1,"%+ld", MenuValue[CurrentMenu].Value) ;
			}
			else
			{
				sprintf(tempValue1,"%d",MenuValue[CurrentMenu].ValueH) ;
				sprintf(tempValue2,"%ld",MenuValue[CurrentMenu].Value) ;
				if (strlen(tempValue2)<9)
				{
					strncat(tempValue1,zeros,9-strlen(tempValue2));
				}
				strcat(tempValue1,tempValue2);
			}
			strcpypgm2ram(TempDigitsEditable,MenuDisplay[CurrentMenu].DigitsEditable);
			Value2DisplayBuffer(tempValue, tempValue1,TempDigitsEditable);
			strcpy(LCDPrintBuffer[1], tempValue);
		}
		else if (MenuDisplay[CurrentMenu].ValueType==ISTEXT)
		{
			if (MenuValue[CurrentMenu].Value==0)
			{
				strcpypgm2ram(LCDPrintBuffer[1],MenuDisplay[CurrentMenu].TextValue0);
			}
			else
			{
				strcpypgm2ram(LCDPrintBuffer[1],MenuDisplay[CurrentMenu].TextValue1);
			}
		}
	}
	
	
}

void LCDMenuValue2Char(char *ValueInChar, unsigned char TempMenu)
{
	unsigned char SizeOfText;
	unsigned char i=0,temp=0;
	char tempString[];
	strcpypgm2ram(ValueInChar,MenuDisplay[TempMenu].FixedText);
	strcpypgm2ram(tempString,MenuDisplay[TempMenu].FixedText);
	SizeOfText=strlen(tempString);
	if (MenuDisplay[TempMenu].ValueType==ISNUMBER)
	{
		if (MenuDisplay[TempMenu].DisplaySign)
		{
		}
	}
}





void LCD_vSetKepad(void)
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

void LCD_vSetIO(void)
{
	OLED_RS_TRIS = 0;
	OLED_RW_TRIS = 0;
	OLED_EN_TRIS = 0;
	OLED_DB7_TRIS = 0;
	OLED_DB6_TRIS = 0;
	OLED_DB5_TRIS = 0;
	OLED_DB4_TRIS = 0;
	OLED_DB3_TRIS = 0;
	OLED_DB2_TRIS = 0;
	OLED_DB1_TRIS = 0;
	OLED_DB0_TRIS = 0;

}

void Initial_LCD(void)
{
	WriteIns(0x38); //function set,8-bit transfer,2-lines display & 5*8 dot characteristic, font 00
	WriteIns(0x38); //function set,8-bit transfer,2-lines display & 5*8 dot characteristic, font 00
	WriteIns(0x38); //function set,8-bit transfer,2-lines display & 5*8 dot characteristic, font 00
	WriteIns(0x38); //function set,8-bit transfer,2-lines display & 5*8 dot characteristic, font 00
	WriteIns(0x0C); //Display ON/OFF control,Display ON,Cursor&Blink OFF
	WriteIns(0x06); //Entry Mode Set, address increment & Shift off
	WriteIns(0x02); //Return Home
	WriteIns(0x01); //Clear Display
}

void DisplayCompanyMenu(void)
{
	strcpypgm2ram (LCDPrintBuffer[0], "                  SINGCOM               ");
	strcpypgm2ram (LCDPrintBuffer[1], "             FREQUENCY CONVERTER        ");
	PrintLCDFromBuffer();
	strcpypgm2ram (LCDPrintBuffer[0], "                                        ");
	strcpypgm2ram (LCDPrintBuffer[1], "                                        ");
}

void LCD_vEnableInterrupt(void)
{
	PIE1bits.TMR2IE = 0;  // disable Timer2 interrupt
	PIR1bits.TMR2IF = 0;
	IPR1bits.TMR2IP = 0; // set Timer2 interrupt to low interrupt




	INTCON3bits.INT1IE = 1; // enable key interrupt
	INTCON3bits.INT1IP = 0; // set key interrupt to low interrupt
	INTCON2bits.INTEDG1 = 1; // 1 = Interrupt on rising edge
	//	INTCON2bits.INTEDG1 = 0; // 0 = Interrupt on falling edge
	TRISBbits.TRISB1 = 1; // RB1 input, INT1
	ADCON0bits.ADON = 0;
	ADCON1 |= 0x0F; // set all IO pin to digital

}

void Value2DisplayBuffer(char *DisplayBuffer, char *Value, char *DigEditable) 
{
	int ValPos=0;
	int i=0,j=0;
	unsigned char k=0,withSign=0;
	ValPos=strlen(Value);
	j=strlen(DisplayBuffer)-1;
	for (i=j;i>=0;i--)
		{
			if (ValPos>0)
			{
				if (DigEditable[k]-1==i)
				{
					if ((Value[ValPos-1]!='+')&&(Value[ValPos-1]!='-'))
					{
						DisplayBuffer[i]=Value[ValPos-1];
					}
					else
					{
						DisplayBuffer[0]=Value[ValPos-1];
						withSign=1;
					}
					ValPos=ValPos-1;
					k=k+1;
				}
			}
			else
			{
				if (DigEditable[k]-1==i)
				{
					if ((withSign==0)||(i>0))
					{
					DisplayBuffer[i]='0';
					k=k+1;
					}
				}
			}
		}
}