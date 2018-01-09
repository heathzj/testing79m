/*********************************************************************

 *********************************************************************
 * FileName:        MainDemo.c
 * Dependencies:    TCPIP.h
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.30 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
 * Company:         Microchip Technology, Inc.
 *
 ********************************************************************/

#define THIS_IS_STACK_APPLICATION


#if defined(__18F97J60) || defined(_18F97J60)
// Include all headers for any enabled TCPIP Stack functions
#include "TCPIP Stack/TCPIP.h"
#endif

// Include functions specific to this stack application
#include "singcom.h"   //*******************************************************************************
#include "Main.h"
#include "define.h"
#include "delays.h"
#include "Global.h"
#include "timers.h"
#include "list.h"
#include "DualUART.h"

#include "Uart.h"

#include <i2c.h>
#include "i2c_c1.h"
#include "oled_c1.h"
#include "Util.h"
#include "lcd.h"

#define nMINUTE 60


//*******************************************************************************************************
extern char Uart0PCCmdBuf[32];
extern BYTE Uart1ReadLCDCmd;
extern BYTE Uart0ReadPCCmd;
extern BYTE Uart0PCCmdBufCount;

//**********************************************************************************************************




#if 0
const char CompanyString1[40] = {"                  SINGCOM               "};
const char CompanyString2[40] = {"               DOWN CONVERTER           "};
const char menutextstring1[] = {"[CVT. 1] "};
const char menutextstring2[] = {"[CVT. 2] "};
const char menutextstring3[] = {"[DEVICE INFO] "};
const char menutextstring4[] = {"LO "};
const char menutextstring5[] = {"RF "};
const char menutextstring6[] = {"LO "};
const char menutextstring7[] = {"Atten. "};
const char menutextstring8[] = {"IF "};
#endif
char FirmwareVersion[4] = {"100"};

tUpDownConverter stConverter;

unsigned char SecondCounter = 0, MinuteCounter = 0;


// Declare AppConfig structure and some other supporting stack variables
unsigned char HardwareWDT = 1;

//#define UART_TEST // uart testing

IP_ADDR		MyStaticIPAddr;
unsigned char ostart,oend,ctemp;

#pragma udata grp5
APP_CONFIG AppConfig;

#pragma udata
unsigned char Read2[2][4];

#pragma udata gpr8
systeminfo_t systeminfo1;

#pragma udata grp10
systeminfo_t2 systeminfo2;


static TICK webPass_tmr = 0, lcd_timer = 0, keypad_tmr = 0;

static void DRI__vEnableGlobalInt();
static void DRI__vHitWatchDog(void) ; 
static void DRI__vEnableGlobalInt();
static void DRI__vConfigSPI();

static void SYS__vStatusTask();
//static void SYS__vLcdDisplayTask(void);
static void SYS__vLcdKeyPadTask(void);
static BYTE SYS__u8InitData(void);

static void HTTP_vClearWebPassword();

void main(void);

/* Called in Main function */
static void DRI__vHitWatchDog(void)  
{ 
	// clear MCU watchdog 
	//ClearWatchDog();
_asm 
	CLRWDT 
_endasm

	// clear hardware watch dog, toggle RA3
//	if (HardwareWDT == 1)
	{
//		WDI_FLAG = 0;
//		HardwareWDT = 0;
	}
//	else
	{
//		WDI_FLAG = 1;
//		HardwareWDT = 1;
	}
}

//********************* FUNCTIONS ************************



/* only used in pic18f8722 */
#if 0 
static void DRI__vINT_OFF(void)
{
//	INTCONbits.GIEL = 0;
//	INTCONbits.GIEH = 0;		
//	INTCONbits.GIE = 0;
}

static void DRI__vINT_ON(void)
{
//	INTCONbits.GIEL = 1;
//	INTCONbits.GIEH = 1;		
//	INTCONbits.GIE = 1;
}
#endif


/* Enable Global interrupts */
static void DRI__vEnableGlobalInt()
{
	INTCONbits.GIEL=1;
	INTCONbits.GIEH = 1;		
	INTCONbits.GIE = 1;		//Enable global interrupts
	INTCONbits.PEIE = 1;	//Enable peripheral interrupts

}

/* Configure SPI bits and registers */
static void DRI__vConfigSPI()
{
	SCL_TRIS = 0;
	SDA_TRIS = 0;

	SSP2CON1bits.SSPEN = 0;
	SSP2STATbits.SMP = 1;
	SSP2CON1 &= 0xF0;
	SSP2CON1 |= 0x08;


}

/* Update System status from sub modules */
/* Consider to change this task to be receive interrupt triggered with update values to reduce process time*/
static void SYS__vStatusTask()
{
    
    /* Status update from Module MCUs and blick LCD */
   
    /* SecondCounter actually update every 1/3 second = 330ms*/
    if ((TickGet() - lcd_timer) >= (TICK_SECOND / 2))
    {
        /* Update status */
        UpdateStatus(); /* A time consuming task */
        lcd_timer = TickGet();
       
    }    
       
}


/* For blinking display Only*/

/* For key processing and Menu Update*/
static void SYS__vLcdKeyPadTask(void)
{
    
    /* 1. Key press handling */
        /* KeyPressedFlag = true & PressedKey is set in ISR.c -> LowISR */
    if ((KeyPressedFlag == TRUE) && (PressedKey != NULL))
    {
		KeyProcessing();
		keypad_tmr = TickGet();
//       FirstPressKey = FALSE; /* This is not the 1st key press */
		KeyPressedFlag = FALSE; /* Clear KeyPressedFlag from LowISR */
    }
	PrintLCD();      
}

/* Flag to be used in HTTP Password Page */
static void HTTP_vClearWebPassword()
{
    
    //schedule task to clear set password result
    if (TickGet() - webPass_tmr >= TICK_SECOND*3)
    {
        webPass_tmr = TickGet();
        webpassinfo.count++;  
        if (webpassinfo.count == 2) 
        { 	   
            webpassinfo.setpass = others; 
            webpassinfo.count = 0;
        }
    }

    
}

static BYTE SYS__u8InitData(void)
{
	stConverter.UpCVT.u8AlarmStatus = 0;
	stConverter.DownCVT.u8AlarmStatus = 0;

	stConverter.UpCVT.u8Lock = 0;
	stConverter.DownCVT.u8Lock = 0;

	stConverter.UpCVT.u16Atten= 0;
	stConverter.DownCVT.u16Atten= 0;	

	stConverter.UpCVT.u8OutputPower= 30;
	stConverter.DownCVT.u8OutputPower= 30;

	stConverter.UpCVT.i16OutputPower= -30;
	stConverter.DownCVT.i16OutputPower= -30;	

	stConverter.UpCVT.u32OutputFreq= 950000;
	stConverter.DownCVT.u32InputFreq= 950000;

	stConverter.UpCVT.u8ALC = FALSE;
	stConverter.DownCVT.u8ALC = FALSE;

	stConverter.UpCVT.u8Mute= FALSE;
	stConverter.DownCVT.u8Mute= FALSE;


	stConverter.stDC.u8LNB_DC_ONOFF= FALSE;
	stConverter.stDC.u8BUC_DC_ONOFF= FALSE;

	stConverter.stDC.u8LNB_REF_ONOFF= FALSE;
	stConverter.stDC.u8BUC_REF_ONOFF= FALSE;

	stI2CUCMsg.u8CtrlStatus= 0;
	stI2CUCMsg.unRfFreq.u32Freq = 950000;
	stI2CUCMsg.unAtten.u16Atten = 0;
	stI2CUCMsg.u16RdPower = 30;
	stI2CUCMsg.u8CtrlStatus = 0x00 | nLOCK; /* Lo Lock, ALC and Mute off */

	stI2CDCMsg.u8CtrlStatus= 0;
	stI2CDCMsg.unRfFreq.u32Freq = 950000;
	stI2CDCMsg.unAtten.u16Atten = 0;
	stI2CDCMsg.u16RdPower = 30;	
	stI2CDCMsg.u8CtrlStatus = 0x00 | nLOCK; /* Lo Lock, ALC and Mute off */


	stI2CREFMsg.u8Status= 0;
	stI2CREFMsg.u16BUCCurrent = 0;
	stI2CREFMsg.u16LNBCurrent = 0;
	stI2CREFMsg.u16BUCCurrentLimit = 0;
	stI2CREFMsg.u16LNBCurrentLimit = 0;


	
	stConverter.stDC.u8EXTREF = 0;
	stConverter.stDC.u8BUC_DC_ONOFF = 0;
	stConverter.stDC.u8LNB_DC_ONOFF = 0;
	stConverter.stDC.u8BUC_REF_ONOFF= 0;
	stConverter.stDC.u8LNB_REF_ONOFF= 0;

	stConverter.stDC.u8BUC_DC_OVERCurrent = 0;

	stConverter.stDC.u8LNB_DC_OVERCurrent = 0;

	stConverter.stDC.u16BUCCurrent = 0;

	stConverter.stDC.u16LNBCurrent = 0;	

	stConverter.stDC.u16BUCCurrentLimit= 0;

	stConverter.stDC.u16LNBCurrentLimit= 0;		

		
}


void main(void)
{
	// Initialize application specific hardware
	InitializeBoard();
	LCD_vSetKepad();  /* Set OLED Keypad  io directions */
	DRI__vConfigSPI(); /* Configure SPI bits and registers */
	LCD_vSetIO(); /* Set LCD control  io directions */
	Main_vLEDInit(); /* Set LED bits on main panel */
 	SYS__u8InitData();
	Initial_LCD();
    
	DisplayCompanyMenu();
	DelayMs(1000);
	UpdateData();
	DelayMs(300);
	PrintLCDWholePage();
    
	RCONbits.IPEN=1;	// yt: should enable IPEN before GIE
    TickInit();
	DRI__vEnableGlobalInt();

#if defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2)
	MPFSInit();
#endif

	// Initialize Stack and application related NV variables into AppConfig.
	InitAppConfig();
	InitData();
    StackInit();

	// init I2C, 
	// set Converter A LO Frequency, Attenuation; 
	// set Converter B LO Frequency, Attenuation;


	LCD_vEnableInterrupt();/* Set LCD key interrupt bits */

	while(1)
    {
        /**
            Task priority:          Time:       Tick Period:                    Trigger:
            
            0 SYS__vLcdKeyPadTask     Long        2nd Check: TICK_SECOND/10       Interrupt(KeyPressedFlag)
            1 StackTask               Long        NULL                            Receiver INT
            2 SYS__vStatusTask        Long        TICK_SECOND/3                   Timer   
            2 StackApplications       Long        NULL                            Receiver INT
            3 SYS__vLcdBlinkTask      Middle      TICK_SECOND                     Timer
            4 HTTP_vClearWebPassword  Short       TICK_SECOND*3                   Timer
        **/

        /* If KeyPressedFlag == TRUE */
        SYS__vLcdKeyPadTask();
 
		 /* Run ethernet stack task only if PHY is connected */
		 if(MACIsLinked())
		 {
			 /* TCP/IP stack task */
			 StackTask();	
			 
			 /* This tasks invokes each of the core stack application tasks */
			 StackApplications(); 
		 }
        
        /* Run lower priority task only if Keypad is not in use */
       // if( TRUE != KeyPressedFlag )
        {

//            SYS__vLcdBlinkTask();
            
            SYS__vStatusTask();

			UpdateData();


            HTTP_vClearWebPassword();

        }

#if defined(STACK_USE_SNMP_SERVER) && !defined(SNMP_TRAP_DISABLED)
	/*	snmp_trap_task(); */  /* SNMP is not used in this version */
#endif 

		DRI__vHitWatchDog();
    }
}




void UpdateData(void)
{
	UpdateDataModuleSys();	//update data between i2c modules and system
	UpdateDataMenuSys();	//update data between LCD menu/keypad and system
	UpdateDataUartSys();	//update data from Uart port
	UpdateDataLEDSys();		//update data from system to front panel LED display status
}




void UpdateDataUartSys()
{
}

void UpdateDataLEDSys()
{
}


