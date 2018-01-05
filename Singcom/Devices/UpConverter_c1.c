#include "define.h"
#include "Global.h"

#include "oled_c1.h"
#include "string.h"
#include "Util.h"


void UpCvt_vPrintFreq(void)
{
	strncpy(LCDPrintBuffer[1], RFFreqString1,sizeof(RFFreqString1));	
}


void UpCvt_vPrintPower(void)
{

	strncpy(LCDPrintBuffer[1], sUCPower,sizeof(sUCPower));

}

void UpCvt_vPrintStPwr(void)
{
	strncpy(LCDPrintBuffer[1], sUPC_StPwr,sizeof(sUPC_StPwr));

}


void UpCvt_vPrintLo(void)
{

	if (stConverter.UpCVT.u8AlarmStatus == 1)
	{
		strncpy(LCDPrintBuffer[1], LOStatusString1,sizeof(LOStatusString1));
	}
	else
	{
		strncpy(LCDPrintBuffer[1], LOStatusString2,sizeof(LOStatusString2));
	}

}

void UpCvt_vPrintAttn(void)
{

	strncpy(LCDPrintBuffer[1], AttenString1,sizeof(AttenString1));

}

void UpCvt_vPrintALC(void)
{

	strncpy(LCDPrintBuffer[1], sALCa,sizeof(sALCa));
}



void UpCvt_vPrintMute(void)
{	

	if(stConverter.UpCVT.u8Mute==1)
		strncpypgm2ram(sMutea,"ENABLED ",8);
	else
		strncpypgm2ram(sMutea,"DISABLED",8);
 
	strncpy(LCDPrintBuffer[1], sMutea,sizeof(sMutea));	  
}

void UpCvt_vSendI2C()
{
	stI2CUCMsg.u16GainCompenVale = 0xFFFF; /* Don't write tables */
	SetI2C(MODULE0);

}


void UpCvt_vSetFreq(void)
{

}


void UpCvt_vSetAttn(void)
{

}

void UpCvt_vSetPower(void)
{
/*Disabled and moved to 	UpCvt_vSetStPwr */		
    
}


void UpCvt_vSetStPwr(void)
{


}


void UpCvt_vSetALC(void)
{

    
}

void UpCvt_vSetMute()
{

    
}





