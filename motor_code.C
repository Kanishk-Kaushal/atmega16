#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <stdlib.h>

int mapped(int z,float input_l,float input_h,float output_l,float output_h)
{
	
	float mapf = (z - input_l)/(input_h-input_l)*(output_h-output_l)+output_l;
	int mapfdone = (int)mapf;
	
	
	return(mapfdone);
	
}

int x;
int y;

void PWM_init()
{
	
	TCCR0 = (1<<WGM01)|(1<<WGM00) |  (1<<COM01) |(1<<CS00);
	TCCR2 = (1<<WGM21)|(1<<WGM20) | (1<<COM21)| (1<<CS02);
	DDRB|=(1<<PINB3); //OCR0 PINB3 Led pwm led
	DDRD|=(1<<PIND7); //OCR2 PIND7 Right pwm led
	
}

unsigned int ADC_read(unsigned char chnl)
{
	chnl&=0b00000111; // select adc channel between 0 to 7
	ADMUX|=chnl;
	
	
	ADCSRA|=(1<<ADSC);   // start conversion
	while(!(ADCSRA & (1<<ADIF)));   // wait for ADIF conversion complete return
	ADCSRA|=(1<<ADIF);   // clear ADIF when conversion complete by writing 1
	
	uint8_t val = ADCL;
	uint16_t value = (ADCH<<8)|val;
	
	
	return (value); //return calculated ADC value
}



int main(void)
{
	
	DDRA = 0b00000000;    //ADC as input
	
	DDRB|= (1<<PINB0);
	DDRB|= (1<<PINB1);
	
	PWM_init();			// Enabling PWM
	
	
	ADMUX=(1<<REFS0);      // Selecting internal reference voltage
	ADCSRA|=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);   //Pre-scalar of 64
	ADMUX&=~(1<<ADLAR);			// for left adjust value
	
	
	
	while (1)
	{
		
		switch(ADMUX)
		{
			
			case 0b01000000:
			
			x = ADC_read(0);
			
			//ADMUX&=0b11111000;
			
			ADMUX = 0b01000001;
			break;
			
			case 0b01000001:
			
			y = ADC_read(1);
			
			//ADMUX&=0b11111000;
			
			ADMUX = 0b01000000;
			break;
			
			default:
			//Default code
			break;
			
			
		}
		
		ADCSRA|=(1<<ADSC);
		
		//x = x/4;
		//y = y/4;
		
		int X_val = mapped(y, 0, 1023, -1023, 1023);
		int Y_val = mapped(x, 0, 1023, -1023, 1023);
		
		//int X_val = y-512;
		//int Y_val = x-512;
		
		X_val = X_val/4;
		Y_val = Y_val/4;
		
		//	int X_val = y;
		//	int Y_val = x;
		
		
		
		
		
		
		float theta = atan2f(Y_val,X_val);
		
		int remap = abs(abs(X_val)-abs(Y_val));
		
		
		
		if(abs(X_val)<60&&abs(Y_val)<60)                    //Buffer around centre
		{
			OCR0 = 0;
			OCR2 = 0;
			PORTB&=~(1<<PINB0);
			PORTB&=~(1<<PINB1);
			_delay_us(10);
		}
		
		else if(theta>0.70f&&theta<0.87f)		                //diagonal 1
		{
			
			OCR2 = 0;
			PORTB&=~(1<<PINB1);
			OCR0 = abs(X_val);
			//PORTB|=(1<<PINB0);
			//_delay_us(5);
			
		}
		
		else if(theta>2.23f&&theta<2.47f)                   //diagonal 2
		{
			OCR0 = 0;
			PORTB&=~(1<<PINB0);
			OCR2 = abs(X_val);
			//PORTB|=(1<<PINB1);
			//_delay_us(5);
		}
		
		else if(theta>-2.40f&&theta<-2.30f)                 //diagonal 3
		{
			OCR2 = 0;
			PORTB&=~(1<<PINB0);
			OCR0 = abs(X_val);
			//PORTB|=(1<<PINB0);
			//_delay_us(5);
		}
		
		else if(theta>-0.90f&&theta<-0.60f)                 //diagonal 4
		{
			OCR0 = 0;
			PORTB&=~(1<<PINB1);
			OCR2 = abs(X_val);
			//PORTB|=(1<<PINB1);
		}
		
		else if(theta>0.0f&&theta<0.79f)                 //Octet 1
		{
			
			//_delay_us(30);
			
			//analogWrite(LeftF,X_val);
			OCR0 = X_val;
			PORTB |= (1<<PINB0);
			
			//analogWrite(RightB,remap);
			OCR2 = remap;
			PORTB&=~(1<<PINB1);
			
			//_delay_us(5);
			
		}


		else if(theta>0.79f&&theta<1.57f)                 //Octet 2
		{
			
			//_delay_us(30);
			
			//analogWrite(LeftF,Y_val);
			OCR0 = Y_val;
			PORTB|=(1<<PINB0);
			
			//analogWrite(RightF,remap);
			if(theta>0.70f&&theta<0.87f)
			OCR2 = 0;
			else
			OCR2 = remap;
			PORTB|=(1<<PINB1);
			
			//_delay_us(5);

			
			
		}

		else if(theta>1.57f&&theta<2.36f)                   //Octet 3
		{
			
			//_delay_us(30);
			
			//analogWrite(LeftF,remap);
			
			if(theta>2.23f&&theta<2.47f)
			OCR0 = 0;
			else
			OCR0 = remap;
			
			PORTB|=(1<<PINB0);
			
			//analogWrite(RightF,Y_val);
			OCR2 = Y_val;
			PORTB|=(1<<PINB1);
			
			//_delay_us(5);
			
			
		}

		else if(theta>2.36f&&theta<3.14f)                   //Octet 4
		{
			//_delay_us(30);
			
			//analogWrite(LeftB,remap);
			
			if(theta>=2.23f&&theta<=2.47f)
			OCR0 = 0;
			else
			OCR0 = remap;
			
			PORTB&=~(1<<PINB0);
			
			//analogWrite(RightF,abs(X_val));
			OCR2 = abs(X_val);
			PORTB|=(1<<PINB1);
			

			//_delay_us(5);
			
			
		}

		else if(theta>-3.14f&&theta<=-2.36f)                //Octet 5
		{
			//_delay_us(30);
			
			//analogWrite(LeftB,abs(X_val));
			OCR0 = abs(X_val);
			PORTB&=~(1<<PINB0);
			
			//analogWrite(RightF,remap);
			
			if(theta>-2.40f&&theta<-2.30f)
			OCR2 = 0;
			else
			OCR2 = remap;
			PORTB|=(1<<PINB1);
			
			
			//_delay_us(5);
			
			
		}

		else if(theta>-2.36f&&theta<-1.57f)                   //Octet 6
		{
			
			//	_delay_us(30);
			
			//analogWrite(LeftB,abs(Y_val));
			OCR0 = abs(Y_val);
			PORTB&=~(1<<PINB0);
			
			
			//analogWrite(RightB,remap);
			OCR2 = remap;
			PORTB&=~(1<<PINB1);

			
			
			//_delay_us(5);
			
			
		}

		else if(theta>-1.57f&&theta<-0.79f)                     //Octet 7
		{
			
			//_delay_us(30);
			
			//analogWrite(LeftB,remap);
			
			//if(remap<40)
			//OCR0 = 0;
			//else
			OCR0 = remap;
			
			
			PORTB&=~(1<<PINB0);
			
			//analogWrite(RightB,abs(Y_val));
			OCR2 = abs(Y_val);
			PORTB&=~(1<<PINB1);

			//_delay_us(5);
			
		}

		else if(theta>-0.79f&&theta<0.0f)                       //Octet 8
		{
			
			//responsible for led off at diagonal
			//_delay_us(30);
			
			//analogWrite(LeftF,remap);
			
			if(theta>-0.90f&&theta<-0.60f)
			OCR0 = 0;
			else
			OCR0 = remap;
			
			
			PORTB|=(1<<PINB0);
			
			//analogWrite(RightB,X_val);
			OCR2 = X_val;
			PORTB&=~(1<<PINB1);

			//_delay_us(5);
			
			
		}
		
		
	}

}
