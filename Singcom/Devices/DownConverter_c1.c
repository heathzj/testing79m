#include "define.h"
#include "Global.h"

#include "i2c_c1.h"
#include "oled_c1.h"
#include "string.h"
#include "Util.h"
tstI2CMsg  stI2CUCMsg;
tstI2CMsg  stI2CDCMsg;
tstI2CMsgVOL  stI2CREFMsg;


void DownCvt_vSendI2C()
{
	stI2CDCMsg.u16GainCompenVale = 0xFFFF; /* Don't write tables */
	SetI2C(MODULE1);
}

void VOLCTRL_vSendI2C()
{
}

   
 






