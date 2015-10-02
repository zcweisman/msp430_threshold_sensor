#include "msp430g2553.h"
#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
	clockSetup();
	ioSetup();
	adcSetup();

	while (1) {
		adcSample();
		avg_adc = ((adc[0]+adc[1]+adc[2]+adc[3]+adc[4]+adc[5]+adc[6]+adc[7]+adc[8]+adc[9]) / 10);;
		findClosest();
		if (distance < 20) {
			if (alarm_state == STATE_DISARMED) state = 0;
			else {
				if (state == 0) {
					alarm_state = STATE_ALERT;
					memset(txString, 0, 80);
					sprintf(txString, "%s%sSystem Status: %s", clear_screen, line_1, ALERT);
					i = 0;
					state = STATE_CHANGE;
					UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
					UCA0TXBUF = txString[i++];
				}
				P2OUT |= ALERTLED;
				P2OUT &= ~(SAFELED + WARNLED);
				change_lights = 1;
			}

		} else if (change_lights == 1) {
			if (alarm_state == STATE_ARMED) {
				P2OUT |= SAFELED;
				P2OUT &= ~(ALERTLED + WARNLED);
			} else if (alarm_state == STATE_DISARMED) {
				P2OUT |= WARNLED;
				P2OUT &= ~(ALERTLED + SAFELED);
			}
		}
	}
}

void clockSetup() {
	WDTCTL = WDTPW + WDTHOLD; // Stop WDT
	DCOCTL = 0; // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_1MHZ; // Set DCO
	DCOCTL = CALDCO_1MHZ;
	UCA0CTL1 |= UCSSEL_2; // SMCLK
}

void ioSetup() {
	P1SEL |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
	P1SEL2 |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
	P2DIR |= SAFELED + ALERTLED + WARNLED;
	P2OUT |= SAFELED + ALERTLED + WARNLED;	// Clear P1OUT which turns off LEDs

	UCA0BR0 = 0x08;										// 1MHz 115200
	UCA0BR1 = 0x00;										// 1MHz 115200
	UCA0MCTL = UCBRS2 + UCBRS0;							// Modulation UCBRSx = 5
	UCA0CTL1 &= ~UCSWRST; 								// **Initialize USCI state machine**
	UC0IE |= UCA0RXIE;									// Enable USCI_A0 RX interrupt
}

void adcSetup() {
	ADC10CTL1 = CONSEQ_2 + INCH_0;						// Repeat single channel, A0
	ADC10CTL0 = ADC10SHT_2 + MSC + ADC10ON + ADC10IE;	// Sample & Hold Time + ADC10 ON + Interrupt Enable
	ADC10DTC1 = 0x0A;									// 10 conversions
	ADC10AE0 |= 0x01;
}

void adcSample() {
	ADC10CTL0 &= ~ENC;									// Disable Conversion
	while (ADC10CTL1 & BUSY);							// Wait if ADC10 busy
	ADC10SA = (int)adc;									// Transfers data to next array (DTC auto increments address)
	ADC10CTL0 |= ENC + ADC10SC;							// Enable Conversion and conversion start
	__bis_SR_register(CPUOFF + GIE);					// Low Power Mode 0, ADC10_ISR
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {
    UCA0TXBUF = txString[i++];							// TX next character
    if (i == sizeof txString - 1)						// TX over?
    	UC0IE &= ~UCA0TXIE;								// Disable USCI_A0 TX interrupt
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
	if (UCA0RXBUF == 'a' && alarm_state != STATE_ALERT) { // 'a' received?
		memset(txString, 0, 80);
		sprintf(txString, "%s%sSystem Status: %s", clear_screen, line_1, ARMED);
		i = 0;
		alarm_state = STATE_ARMED;
		UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
		UCA0TXBUF = txString[i++];
	} else if (UCA0RXBUF == 'd') {
		memset(txString, 0, 80);
		sprintf(txString, "%s%sSystem Status: %s", clear_screen, line_1, DISARMED);
		i = 0;
		state = 0;
		alarm_state = STATE_DISARMED;
		UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
		UCA0TXBUF = txString[i++];
	} else if (UCA0RXBUF == 'r') {
		memset(txString, 0, 80);
		sprintf(txString, "%s%sDistance: %d inches", clear_screen, line_1, distance);
		i = 0;
		state = 0;
		UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
		UCA0TXBUF = txString[i++];
	} else if (UCA0RXBUF == 13) {
		P2OUT |= SAFELED + WARNLED + ALERTLED;
		change_lights = 0;
	}
}

void findClosest() {
	int i, j;
	int curDif = 1000;
	int curInd;

	for (j = 0; j < 2; j++) {
		for (i = 0; i < 40; i++) {
			if (abs(distanceVals[i] - avg_adc) < curDif) {
				curDif = abs(distanceVals[i] - avg_adc);
				curInd = i;
			}
		}
	}

	distance = curInd + 1;
	if (distance < 5) distance = 5;
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}
