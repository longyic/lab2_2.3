/*
 * lab2_2.3.c
 *
 * Created: 2018/9/18 14:40:28
 * Author : Lenovo
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "uart.h"
volatile unsigned int edge1, edge2, pulse;
volatile int overflows, rising, counter, memory;
float freq;
void length();

int main(void)
{
	uart_init();

	DDRB |= (1 << PORTB1);                  //set PB1 as output
	DDRD |= (1 << PORTD6);                  //set PD6 as output
	PORTB |= (1 << PORTB1);
	
	TIMSK1 |= (1 << ICIE1) | (1 << TOIE1);      //edge interrupt and overflow interrupt
	TCCR1B |= (1 << CS10) | (1 << ICES1);      //no prescale and input capture
	
	TCCR0A |= (1 << COM0A0) | (1 << WGM01);    //toggle OC0A and CTC
	TCCR0B |= (1 << CS02);                    //prescale 64
	
	TCNT0 = 0;                         //initial time counter0
	TCNT1 = 0;                         //initial time counter1
	
	edge1 = 0;
	edge2 = 0;
	rising = 0;
	overflows = 0;                      
	counter = 0;
	memory = 0;

	sei();
	while (1)
	{
		if (TCNT1 >= 79)
		{
			PORTB ^= (1 << PORTB1);     //change from output to input
			TCNT1 = 0;                  //clear counter
		}
		length();                       //calculate pulse
		TIMSK0 |= (1 << OCIE0A);        //match interrupt
	}
}

void length()
{
	if(edge2 < edge1)
	{
		overflows--;
	}
	pulse = overflows * 65536 + (edge2 - edge1);
}

ISR(TIMER1_CAPT_vect)
{
	if(!rising){
		edge1 = ICR1;
		rising = 1;
		TCCR1B &= ~(1 << ICES1);          //input capture negative edge
		TIFR1 |= (1 << ICR1);             //clear capture flag
	}
	else{
		edge2 = ICR1;
		rising = 0;
		TIFR1 |= (1 << ICR1);             //clear capture flag
		TCCR1B |= (1 << ICES1);           //input capture positive edge
	}
}

ISR(TIMER1_OVF_vect)
{
	overflows++;
}

ISR(TIMER0_COMPA_vect)
{
	memory = pulse;
	if(memory < 500)
	{
		memory = 500;
	}
	if(memory > 5000)
	{
		memory = 5000;
	}
	counter = 14 + (memory/5000.0) * 16;
	OCR0A = counter;                               //set OCR0A compar value
	PORTD ^= (1 << PORTD6);                        //output PD6
	freq = 2093 - (memory/5000.0f) * 1046.5;       //calculate freq
	printf("Frequency is %.2f Hz\n", freq);
	TIMSK0 &= ~(1 << OCIE0A);                      //disable match interrupt
}