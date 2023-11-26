#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>

typedef struct key {
	int clk;
	int x;
	int y;
}key ;

void loadSpiDriver();
void spiSetup (int spiChannel);
int myAnalogRead(int spiChannel,int channelConfig,int analogChannel);
void print_info();
key mcp3008();