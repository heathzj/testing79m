#include "define.h"



unsigned char LCDBufferPosition[2];
char LOStatusString1[9] = {"LOCKED  "}; 
char LOStatusString2[9] = {"UNLOCKED"}; 

/*JuZh 950.000MHz to 2150.000MHz */
char RFFreqString1[12] = {"0950.000MHz"}; /* must update to 3 digits decimal */
char RFFreqString2[12] = {"0950.000MHz"}; /* must update to 3 digits decimal */
char AttenString1[8] = {"00.0 dB"}; 
char AttenString2[8] = {"00.0 dB"}; 

char IPString[16] = {"192.168.001.001"}; 
char MNString[18] = {"SCMDC62001SN10-RM"}; 
char SNString[9] = {"12345678"}; 

char s18VDC[4] = {"OFF"};
char s24VDC[4] = {"OFF"};
char sLNBREF[4] = {"OFF"};
char sBUCREF[4] = {"OFF"};


char sUCPower[10] = {"-30.0 dBm"};
char sDCPower[10] = {"-30.0 dBm"};
char sUPC_StPwr[10] = {"-00.0 dBm"};
char sDNC_StPwr[10] = {"-00.0 dBm"};

char sALCa[4]= {"OFF"};
char sALCb[4] = {"OFF"};
char sMutea[9] = {"DISABLED"};
char sMuteb[9] = {"DISABLED"};

unsigned char KeyPressedFlag = FALSE, PressedKey = NULL;
