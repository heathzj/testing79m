#ifndef LCD_H
#define LCD_H

#define ISNUMBER 0
#define ISTEXT 1
#define NIL 2
#define MENUCOUNT 14			//the total number of menu
#define nLCDLEN 40


void KeyProcessing(void);
unsigned char LCDMenuInScreen(void);
void PrintLCDEditPosition(void);
void PrintLCDWholePage(void);
void PrintLCDFromBuffer(void);
void PrintLCDEditPosition(void);
void LCDDisplayLoadData(void);
void LCD_vSetKepad(void);
void LCD_vSetIO(void);
void Initial_LCD(void);
void DisplayCompanyMenu(void);
void LCD_vEnableInterrupt(void);
void Value2DisplayBuffer(char *DisplayBuffer, char *Value, char *DigEditable);

#endif