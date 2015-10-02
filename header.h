/*
 * header.h
 *
 *  Created on: Jun 4, 2015
 *      Author: Zachary
 */

#ifndef HEADER_H_
#define HEADER_H_
#define WARNLED BIT3
#define ALERTLED BIT4
#define SAFELED BIT5
#define TXRXLED BIT6
#define TXD BIT2
#define RXD BIT1
#define MAX_STR_LEN 80
#define STATE_ARMED 1
#define STATE_DISARMED 2
#define STATE_ALERT 3
#define STATE_CHANGE 4;

void clockSetup();
void ioSetup();
void adcSetup();
void adcSample();
void findClosest();

char txString[MAX_STR_LEN];
const char ARMED[] = {"ARMED"};
const char ALERT[] = {"ALERT"};
const char DISARMED[] = {"DISARMED"};
const char clear_screen[] = {"\033[2J"};
const char line_1[] = {"\033[1;1H"};
const char line_2[] = {"\033[2;1H"};

unsigned char rxData = 0;
unsigned char change_lights = 1;
unsigned char alarm_state = STATE_DISARMED;
unsigned char state = 0;

int i;
int adc[10] = {0};
int distance;
int avg_adc = 0;

int distanceVals[] = {	628, 630, 635, 722, 749, 748, 737, 714,
						671, 626, 589, 557, 520, 486, 452, 428,
						404, 387, 364, 347, 326, 315, 304, 303,
						293, 283, 277, 261, 255, 225, 215, 210,
						212, 199, 193, 188, 177, 172, 182, 169
};


#endif /* HEADER_H_ */
