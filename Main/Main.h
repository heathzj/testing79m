/*********************************************************************
 *
 *                  Headers for TCPIP Demo App
 *
 *********************************************************************
 * FileName:        Main.h
 * Dependencies:    Compiler.h
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.30 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2009 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *		used in conjunction with a Microchip ethernet controller for
 *		the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * E. Wood				4/26/08 Copied from MainDemo.c
 ********************************************************************/
#ifndef _MAIN_H
#define _MAIN_H

//#define BAUD_RATE       (19200)		// bps
#define BAUD_RATE       (9600)		// bps
#if !defined(THIS_IS_STACK_APPLICATION)
	extern BYTE AN0String[8];
#endif

void DoUARTConfig(void);
void DoUARTConfig2(void);


#if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS)
	void SaveAppConfig(void);
    void SaveSetting(void);
#else
	#define SaveAppConfig()
#endif

void SMTPDemo(void);
void PingDemo(void);
void SNMPTrapDemo(void);
void GenericTCPClient(void);
void GenericTCPServer(void);
void BerkeleyTCPClientDemo(void);
void BerkeleyTCPServerDemo(void);
void BerkeleyUDPClientDemo(void);

// An actual function defined in Main.c for displaying the current IP 
// address on the UART and/or LCD.
#if defined(__18F97J60) || defined(_18F97J60)
void DisplayIPValue(IP_ADDR IPVal);
void DisplayIPValue2(IP_ADDR IPVal);
#endif





void UpdateData(void);
void UpdateDataMenuSys(void);
void UpdateDataModuleSys(void);
void UpdateDataUartSys(void);
void UpdateDataLEDSys(void);


#endif // _MAIN_H
