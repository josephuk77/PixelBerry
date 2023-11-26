/********************************************************************************************
*Filename      : readmcp300x.c
*Description   : read ADC mcp3008 value with raspberry pi
*Author        : Alan
*Website       : www.osoyoo.com
*Update        : 2017/06/29
********************************************************************************************/
//#define _GNU_SOURCE

#include "mcp3008.h"
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>
 
#define TRUE                (1==1)
#define FALSE               (!TRUE)
#define CHAN_CONFIG_SINGLE  8
#define CHAN_CONFIG_DIFF    0
 
static int myFd ;
 
char *usage = "Usage: mcp3008 all|analogChannel[1-8] [-l] [-ce1] [-d]";
// -l   = load SPI driver,  default: do not load
// -ce1  = spi analogChannel 1, default:  0
// -d   = differential analogChannel input, default: single ended
 
void loadSpiDriver()
{
    if (system("gpio load spi") == -1)
    {
        fprintf (stderr, "Can't load the SPI driver: %s\n", strerror (errno)) ;
        exit (EXIT_FAILURE) ;
    }
}
 
void spiSetup (int spiChannel)
{
    if ((myFd = wiringPiSPISetup (spiChannel, 10000)) < 0)
    {
        fprintf (stderr, "Can't open the SPI bus: %s\n", strerror (errno)) ;
        exit (EXIT_FAILURE) ;
    }
}
 
int myAnalogRead(int spiChannel,int channelConfig,int analogChannel)
{
    if(analogChannel<0 || analogChannel>7)
        return -1;
    unsigned char buffer[3] = {1}; // start bit
    buffer[1] = (channelConfig+analogChannel) << 4;
    wiringPiSPIDataRW(spiChannel, buffer, 3);
    return ( (buffer[1] & 3 ) << 8 ) + buffer[2]; // get last 10 bits
}

key mcp3008()
{
    int loadSpi=FALSE;
    int analogChannel=0;
    int spiChannel=0;
    int channelConfig=CHAN_CONFIG_SINGLE;

	struct key keyinput;

    if(loadSpi==TRUE)
        loadSpiDriver();
    wiringPiSetup () ;
    spiSetup(spiChannel);
    	//print_info();

		keyinput.clk = myAnalogRead(spiChannel,channelConfig,0);
		keyinput.x = myAnalogRead(spiChannel,channelConfig,1);
		keyinput.y = myAnalogRead(spiChannel,channelConfig,2);
    
    close (myFd) ;
    return keyinput;
}