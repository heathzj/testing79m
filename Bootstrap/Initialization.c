#include "Global.h"
#include "define.h"
#include "p18f97j60.h"
#include "TCPIP Stack/TCPIP.h"

IP_ADDR Manager_IP;

void InitData(void)
{
	unsigned char i,count, count2;

	Manager_IP.v[0]=192;
	Manager_IP.v[1]=168;
	Manager_IP.v[2]=1;
	Manager_IP.v[3]=10;

	Read2[0][0]=192;
	Read2[0][1]=168;
	Read2[0][2]=1;
	Read2[0][3]=10;

	// write default MAC address
	AppConfig.MyMACAddr.v[0] = 0x00;
	AppConfig.MyMACAddr.v[1] = 0x11;
	AppConfig.MyMACAddr.v[2] = 0x01;
	AppConfig.MyMACAddr.v[3] = 0x01;

	SaveAppConfig();

#if defined(STACK_USE_SNMP_SERVER)
	Load_config();
#endif

	// yt: move to RCUHLInit()
	//SSPALL_XEEReadArray((EEADR_RCU_ID), (BYTE*)&RCU_ID, sizeof(RCU_ID));


	// read default value  from EEPROM
	//XEEReadArray((0x00f2), (BYTE*)&systeminfo1, sizeof(systeminfo1));
	//XEEReadArray((0x0133), (BYTE*)&systeminfo2, sizeof(systeminfo2));

//zj set model and serial
	XEEReadArray((0x00f2), (BYTE*)&MNString, sizeof(MNString));
	XEEReadArray((0x0133), (BYTE*)&SNString, sizeof(SNString));


	XEEReadArray((0x0174), (BYTE*)&Read2, sizeof(Read2));
	XEEReadArray((0x0191), (BYTE*)&webpassinfo.webpassword, sizeof(webpassinfo.webpassword));

    count=0;
	while(webpassinfo.webpassword[count]!=0)
	{
		count++;	
	}
    if(count >8)
		strcpypgm2ram(webpassinfo.webpassword, "admin");


	for(count2=0;count2<count;count2++)
	{

		if((webpassinfo.webpassword[count2]<0x30)||(webpassinfo.webpassword[count2]>0x7A))
		{
			strcpypgm2ram(webpassinfo.webpassword, "admin");
			break;
		}
	}



	Manager_IP.v[0] = Read2[0][0];
	Manager_IP.v[1] = Read2[0][1];
	Manager_IP.v[2] = Read2[0][2];
	Manager_IP.v[3] = Read2[0][3];

#if !defined SNMP_TRAP_DISABLED && defined STACK_USE_SNMP_SERVER

	trap_en = 0x01;
	for(i=0; i<2;i++)
	{
		snmp_trap[i].alarm_temp = 0x01;
	}
#endif
}



/****************************************************************************
  Function:
    static void InitializeBoard(void)

  Description:
    This routine initializes the hardware.  It is a generic initialization
    routine for many of the Microchip development boards, using definitions
    in HardwareProfile.h to determine specific initialization.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/
#if defined(__18F97J60) || defined(_18F97J60)
 void InitializeBoard(void)
{	
	// initialize EEPROM
#if defined(EEPROM_CS_TRIS)
	XEEInit();
#endif

	// Initiates board I/O setup process 
	// port config
	// Port A 0	LEDA		library use
	// Port A 1	LEDB		library use
	// Port A 2	Reserve2
	// Port A 3	Reserve1
	// Port A 4	RXEN0
	// Port A 5	Reserve3
	TRISA = 0xFF;
	TRISAbits.TRISA4 = 0; // RXEN0, output

	// Port B 0	UART2_IRQ
	// Port B 1	UART3_IRQ
	// Port B 2	Reserve
	// Port B 3	Reserve
	// Port B 4	MODE1
	// Port B 5	DXEN1
	// Port B 6	PGC			debugger use
	// Port B 7	PGD			debugger use
	TRISB = 0xFF;
	TRISBbits.TRISB0 = 0; // MODE1, output
	TRISBbits.TRISB4 = 0; // MODE1, output
	TRISBbits.TRISB5 = 0; // DXEN1, output

	// Port C 0	DXEN0
	// Port C 1	MODE0
	// Port C 2	DXEN3
	// Port C 3	CLK
	// Port C 4	SDI1
	// Port C 5	DATA
	// Port C 6	UART_TX0
	// Port C 7	UART_RX0
	TRISC = 0xFF;
	TRISCbits.TRISC0 = 0; // DXEN0, output
	TRISCbits.TRISC1 = 0; // MODE0, output
	TRISCbits.TRISC2 = 0; // DXEN3, output
	TRISCbits.TRISC3 = 0; // CLK, output
	TRISCbits.TRISC5 = 0; // DATA, output
	TRISCbits.TRISC6 = 0; // UART_TX0, output

	// Port D 0	CS_CPLD
	// Port D 1	Reserve4
	// Port D 2	CS_ADC1
	// Port D 3	CS_ADC0
	// Port D 4	MOSI
	// Port D 5	MISO
	// Port D 6	SCK
	// Port D 7	FAN_DET
	TRISD = 0xFF;
	TRISDbits.TRISD0 = 0; // CS_CPLD, output
	TRISDbits.TRISD2 = 0; // CS_ADC1, output
	TRISDbits.TRISD3 = 0; // CS_ADC0, output
	TRISDbits.TRISD4 = 0; // MOSI, output
	TRISDbits.TRISD6 = 0; // SCK, output
	
	// Port E 0	DATA0
	// Port E 1	DATA1
	// Port E 2	DATA2
	// Port E 3	DATA3
	// Port E 4	DATA4
	// Port E 5	DATA5
	// Port E 6	DATA6
	// Port E 7	DATA7
	TRISE = 0; // Dual UART data port
	
	// Port F 0	Reserve
	// Port F 1	CPLDUCBUS13
	// Port F 2	CPLDUCBUS8			PATH_ALARM_4
	// Port F 3	CPLDUCBUS9
	// Port F 4	CPLDUCBUS10
	// Port F 5	CPLDUCBUS11
	// Port F 6	CPLDUCBUS12
	// Port F 7	CPLDUCBUS13
	TRISF = 0xFF;
	
	// Port G 0	RXEN1
	// Port G 1	UART_TX1
	// Port G 2	UART_RX1
	// Port G 3	MODE3
	// Port G 4	Reserve
	// Port G 5	CLEAR
	// Port G 6	CS2
	// Port G 7	RXEN3
	TRISG = 0xFF;
	TRISGbits.TRISG0 = 0; // RXEN1, output
	TRISGbits.TRISG1 = 0; // UART_TX1, output
	TRISGbits.TRISG3 = 0; // MODE3, output
	TRISGbits.TRISG5 = 0; // CLEAR, output
	TRISGbits.TRISG6 = 0; // CS2, output
	TRISGbits.TRISG7 = 0; // RXEN3, output

	// Port H 0	CPLDUCBUS0			SWITCH_MODE
	// Port H 1	CPLDUCBUS1			SET_SWITCH1_POS
	// Port H 2	CPLDUCBUS2			SET_SWITCH2_POS
	// Port H 3	CPLDUCBUS3			READ_SWITCH1_POS
	// Port H 4	CPLDUCBUS4			READ_SWITCH2_POS
	// Port H 5	CPLDUCBUS5			PATH_ALARM_1
	// Port H 6	CPLDUCBUS6			PATH_ALARM_2
	// Port H 7	CPLDUCBUS7			PATH_ALARM_3
	TRISH = 0xFF;
	TRISHbits.TRISH0=0;
	TRISHbits.TRISH1=0;
	TRISHbits.TRISH2=0;

	// Port J 0	ADDR2
	// Port J 1	ADDR1
	// Port J 2	ADDR0
	// Port J 3	UART_RST
	// Port J 4	ENB
	// Port J 5	ENA
	// Port J 6	WR
	// Port J 7	RD
	TRISJ = 0; // DUAL UART address/control port

	// initialize SPI driver
//	SPIInit();

	// initialize dual uart driver
//    InitialiseDualUart();

}
#endif

/*********************************************************************
 * Function:        void InitAppConfig(void)
 *
 * PreCondition:    MPFSInit() is already called.
 *
 * Input:           None
 *
 * Output:          Write/Read non-volatile config variables.
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
// MAC Address Serialization using a MPLAB PM3 Programmer and 
// Serialized Quick Turn Programming (SQTP). 
// The advantage of using SQTP for programming the MAC Address is it
// allows you to auto-increment the MAC address without recompiling 
// the code for each unit.  To use SQTP, the MAC address must be fixed
// at a specific location in program memory.  Uncomment these two pragmas
// that locate the MAC address at 0x1FFF0.  Syntax below is for MPLAB C 
// Compiler for PIC18 MCUs. Syntax will vary for other compilers.
//#pragma romdata MACROM=0x1FFF0
#if defined(__18F97J60) || defined(_18F97J60)
static ROM BYTE SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6};
//#pragma romdata

 void InitAppConfig(void)
{
	AppConfig.Flags.bIsDHCPEnabled = TRUE;
	AppConfig.Flags.bInConfigMode = TRUE;
	memcpypgm2ram((void*)&AppConfig.MyMACAddr, (ROM void*)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));
	AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2<<8ul | MY_DEFAULT_IP_ADDR_BYTE3<<16ul | MY_DEFAULT_IP_ADDR_BYTE4<<24ul;
	AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
	AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2<<8ul | MY_DEFAULT_MASK_BYTE3<<16ul | MY_DEFAULT_MASK_BYTE4<<24ul;
	AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
	AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2<<8ul | MY_DEFAULT_GATE_BYTE3<<16ul | MY_DEFAULT_GATE_BYTE4<<24ul;
	AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2<<8ul  | MY_DEFAULT_PRIMARY_DNS_BYTE3<<16ul  | MY_DEFAULT_PRIMARY_DNS_BYTE4<<24ul;
	AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2<<8ul  | MY_DEFAULT_SECONDARY_DNS_BYTE3<<16ul  | MY_DEFAULT_SECONDARY_DNS_BYTE4<<24ul;


	// SNMP Community String configuration
	#if defined(STACK_USE_SNMP_SERVER)
	{
		BYTE i;
		static ROM char * ROM cReadCommunities[] = SNMP_READ_COMMUNITIES;
		static ROM char * ROM cWriteCommunities[] = SNMP_WRITE_COMMUNITIES;
		ROM char * strCommunity;
		
		for(i = 0; i < SNMP_MAX_COMMUNITY_SUPPORT; i++)
		{
			// Get a pointer to the next community string
			strCommunity = cReadCommunities[i];
			if(i >= sizeof(cReadCommunities)/sizeof(cReadCommunities[0]))
				strCommunity = "";

			// Ensure we don't buffer overflow.  If your code gets stuck here, 
			// it means your SNMP_COMMUNITY_MAX_LEN definition in TCPIPConfig.h 
			// is either too small or one of your community string lengths 
			// (SNMP_READ_COMMUNITIES) are too large.  Fix either.
			if(strlenpgm(strCommunity) >= sizeof(AppConfig.readCommunity[0]))
				while(1);
			
			// Copy string into AppConfig
			strcpypgm2ram((char*)AppConfig.readCommunity[i], strCommunity);

			// Get a pointer to the next community string
			strCommunity = cWriteCommunities[i];
			if(i >= sizeof(cWriteCommunities)/sizeof(cWriteCommunities[0]))
				strCommunity = "";

			// Ensure we don't buffer overflow.  If your code gets stuck here, 
			// it means your SNMP_COMMUNITY_MAX_LEN definition in TCPIPConfig.h 
			// is either too small or one of your community string lengths 
			// (SNMP_WRITE_COMMUNITIES) are too large.  Fix either.
			if(strlenpgm(strCommunity) >= sizeof(AppConfig.writeCommunity[0]))
				while(1);

			// Copy string into AppConfig
			strcpypgm2ram((char*)AppConfig.writeCommunity[i], strCommunity);
		}
	}
	#endif

	// Load the default NetBIOS Host Name
	memcpypgm2ram(AppConfig.NetBIOSName, (ROM void*)MY_DEFAULT_HOST_NAME, 16);
	FormatNetBIOSName(AppConfig.NetBIOSName);



	#if defined(EEPROM_CS_TRIS)
	{
		BYTE c;
		
	    // When a record is saved, first byte is written as 0x60 to indicate
	    // that a valid record was saved.  Note that older stack versions
		// used 0x57.  This change has been made to so old EEPROM contents
		// will get overwritten.  The AppConfig() structure has been changed,
		// resulting in parameter misalignment if still using old EEPROM
		// contents.
		XEEReadArray(0x0000, &c, 1);
	    if(c == 0x60u)
		    XEEReadArray(0x0001, (BYTE*)&AppConfig, sizeof(AppConfig));
	    else
	        SaveAppConfig();
	}

	#endif

	MyStaticIPAddr = AppConfig.MyIPAddr; //save this read static IP to display on web page
}


#endif //#if defined(__18F97J60) || defined(_18F97J60)


/*unused in this project */
void SetDefaultValue(void)
{	

	unsigned char count, count2;

	Manager_IP.v[0]=192;
	Manager_IP.v[1]=168;
	Manager_IP.v[2]=1;
	Manager_IP.v[3]=10;

	Read2[0][0]=192;
	Read2[0][1]=168;
	Read2[0][2]=1;
	Read2[0][3]=10;


	AppConfig.Flags.bIsDHCPEnabled=FALSE;
	AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2<<8ul | MY_DEFAULT_IP_ADDR_BYTE3<<16ul | MY_DEFAULT_IP_ADDR_BYTE4<<24ul;
	AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
	AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2<<8ul | MY_DEFAULT_MASK_BYTE3<<16ul | MY_DEFAULT_MASK_BYTE4<<24ul;
	AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
	AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2<<8ul | MY_DEFAULT_GATE_BYTE3<<16ul | MY_DEFAULT_GATE_BYTE4<<24ul;
	AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2<<8ul  | MY_DEFAULT_PRIMARY_DNS_BYTE3<<16ul  | MY_DEFAULT_PRIMARY_DNS_BYTE4<<24ul;
	AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2<<8ul  | MY_DEFAULT_SECONDARY_DNS_BYTE3<<16ul  | MY_DEFAULT_SECONDARY_DNS_BYTE4<<24ul;

	SaveAppConfig();
	
	strcpypgm2ram(webpassinfo.webpassword, "admin");

    count=0;
	while(webpassinfo.webpassword[count]!=0)
	{
		count++;	
	}
    if(count >8)
		strcpypgm2ram(webpassinfo.webpassword, "admin");


	for(count2=0;count2<count;count2++)
	{

		if((webpassinfo.webpassword[count2]<0x30)||(webpassinfo.webpassword[count2]>0x7A))
		{
			strcpypgm2ram(webpassinfo.webpassword, "admin");
			break;
		}
	}

#if defined(STACK_USE_SNMP_SERVER)
	Load_config();
#endif

	SaveSetting();

}


