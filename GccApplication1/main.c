/*
 * GccApplication1.c
 *
 * Created: 12/23/2021 12:57:21 AM
 * Author : ahmed
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>



#define EN PC2
#define RW PC1
#define RS PC0

#define LCD_Data_Dir DDRA
#define LCD_Data_Value PORTA

#define LCD_Cmd_Dir DDRC
#define LCD_Cmd_Value PORTC
#define F_CPU 16000000UL

void LCD_Cmd_Fun (unsigned char cmd);
void LCD_Data_Fun (unsigned char data);
void LCD_init(void);
void LCD_String (char *str);
void LCD_String_xy (char row, char pos, char *str);
void LCD_Clear();
void PWM_init();

int main (void){
	LCD_Data_Dir=0xFF; 
	LCD_Cmd_Dir=0xFF;
	
	unsigned int TR=0,TF=0,TR2=0,high=0,period=0;
	char frequency[14],dc[7];
	
	DDRD=0x00;
	PORTD=0xFF;
	
	
	LCD_init();
	_delay_ms(100);
	LCD_Cmd_Fun(0x0C);
	_delay_ms(100);
	
	unsigned char duty;
	
	PWM_init();
	
	while(1)
	{
		TCCR1A = 0x00;
		TCNT1=0;
		TIFR = (1<<ICF1);  	/* Clear ICF (Input Capture flag) flag */

		TCCR1B = 0x41;  	/* Rising edge, no prescaler */
		while((TIFR&(1<<ICF1)) == 0);
		TR = ICR1;  		/* Take value of capture register */
		TIFR =(1<<ICF1); 	/* Clear ICF flag */
		
		TCCR1B = 0x01;  	/* Falling edge, no prescaler */
		while((TIFR&(1<<ICF1))== 0);
		TF = ICR1;  		/* Take value of capture register */
		TIFR =(1<<ICF1);  	/* Clear ICF flag */
		
		
		TCCR1B = 0x41;  	/* Rising edge, no prescaler */
		while((TIFR&(1<<ICF1)) == 0);
		TR2 = ICR1;  		/* Take value of capture register */
		TIFR = (1<<ICF1);  	/* Clear ICF flag */
		

		TCCR1B = 0x00;  		/* Stop the timer */
		
		if(TF>TR && TF<TR2)  	/* Check for valid condition, 
					to avoid timer overflow reading */
		{
			high=TF-TR;
			period=TR2-TR;
			

			
			long freq= F_CPU/period;/* Calculate frequency */

						/* Calculate duty cycle */
            		float duty_cycle =((float) high /(float)period)*100;			
			
			ltoa(freq,frequency,10);
			
			itoa((int)duty_cycle,dc,10);
			
			LCD_Cmd_Fun(0x80);
			LCD_String("F:");
			LCD_String(frequency);
			LCD_String("Hz");
			LCD_Cmd_Fun(0xC0);
			
			LCD_String(" DC:");
			LCD_String(dc);
			LCD_String("%");
			
			
			
			
			
		}
		else
		{
			LCD_Clear();
			LCD_String("out of range");
		}
		_delay_ms(50);
/*-----------------------------*/
			OCR0=100;
	}
	
	return 0;
} 

void LCD_Cmd_Fun (unsigned char cmd){
	LCD_Data_Value=cmd;
	LCD_Cmd_Value &=~(1<<RS); //select command mode 
	LCD_Cmd_Value &=~(1<<RW); // select write mode
	LCD_Cmd_Value |=(1<<EN); // give a pulse
	_delay_ms(2);
	LCD_Cmd_Value &=~(1<<EN);
}
void LCD_Data_Fun (unsigned char data){
	LCD_Data_Value=data;
	LCD_Cmd_Value |=(1<<RS); //select data mode
	LCD_Cmd_Value &=~(1<<RW); // select write mode
	LCD_Cmd_Value |=(1<<EN); // give a pulse
	_delay_ms(2);
	LCD_Cmd_Value &=~(1<<EN);
}
void LCD_init(void){
	LCD_Cmd_Fun(0x38);
	_delay_ms(2);
	LCD_Cmd_Fun(0x01);
	_delay_ms(2);
	LCD_Cmd_Fun(0x02);
	_delay_ms(2);
	LCD_Cmd_Fun(0x06);
	_delay_ms(2);
	LCD_Cmd_Fun(0x80);
	_delay_ms(2);
}
void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Data_Fun (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)	/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Cmd_Fun((pos & 0x0F)|0x80);	/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Cmd_Fun((pos & 0x0F)|0xC0);	/* Command of first row and required position<16 */
	LCD_String(str);		/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Cmd_Fun (0x01);		/* Clear display */
	_delay_ms(2);
	LCD_Cmd_Fun (0x80);		/* Cursor at home position */
}

void PWM_init()
{
	/*set fast PWM mode with non-inverted output*/
	TCCR0 = (1<<WGM00) | (1<<WGM01) | (1<<COM01) | (1<<CS01);
	DDRB|=(1<<PB3);  /*set OC0 pin as output*/
}
