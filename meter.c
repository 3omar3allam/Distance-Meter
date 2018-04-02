#include <stdint.h>
#include <tm4c123gh6pm.h>
#define speed (float)35 //  in cm/msec
#define ticks_per_ms (uint32_t)16000
#define magic_key (uint32_t)0x4C4F434B

int PA2,PA3, PA4,PA5 ,PA6 ,PB0 ,PB1 ,PB2 ,PB5 ,PB6 , PC4, PC5 ,PC6 ,PC7 ,PE0 ,PE1 ,PE2 ,PE3 ,PB3,PF1,PF2,PF3,PD2,PD1 ;
void SystemInit (void)
{

}
void init(){
	SYSCTL_RCGCGPIO_R |= 0x3F; //clock enabled to all ports
	while((SYSCTL_PRGPIO_R & 0x3F ) != 0x3F);  //peripheral ready register
	
	/*********************************************/


	GPIO_PORTA_DEN_R |= 0xFF;
	GPIO_PORTB_DEN_R|= 0xFF;
	GPIO_PORTC_DEN_R |= 0xFF;
	GPIO_PORTD_DEN_R |= 0xFF;
	GPIO_PORTE_DEN_R |= 0xFF;
	GPIO_PORTF_DEN_R |= 0xFF;
	GPIO_PORTA_DATA_R &= 0x00;
	GPIO_PORTB_DATA_R &=0x00;
	GPIO_PORTC_DATA_R &= 0x00;
	GPIO_PORTF_DATA_R &= 0x00;
	GPIO_PORTE_DATA_R &= 0x00;
	GPIO_PORTD_DATA_R &= 0x00;
  
	
	/*********************************************/
	GPIO_PORTA_DIR_R= 0x64;	 //	 PA2 (trigger)	  PA3,PA4:input	 (echo & switch)
	GPIO_PORTB_DIR_R= 0x6F ;
	GPIO_PORTC_DIR_R= 0xF0 ;
	GPIO_PORTE_DIR_R= 0x0F;
	GPIO_PORTF_DIR_R= 0x0E;
	GPIO_PORTD_DIR_R= 0x06;
	
	
	/*********************************************/
	GPIO_PORTA_AFSEL_R&= 0x00;
	GPIO_PORTB_AFSEL_R &= 0x00;
	GPIO_PORTC_AFSEL_R&= 0x00;
	GPIO_PORTD_AFSEL_R &= 0x00;
	GPIO_PORTE_AFSEL_R &= 0x00;
	GPIO_PORTF_AFSEL_R&= 0x00;
			GPIO_PORTA_LOCK_R = magic_key;
	GPIO_PORTB_LOCK_R = magic_key;
	GPIO_PORTC_LOCK_R = magic_key;
	GPIO_PORTD_LOCK_R = magic_key;
	GPIO_PORTE_LOCK_R = magic_key;
	GPIO_PORTF_LOCK_R = magic_key;
	GPIO_PORTA_CR_R 	|= 0xFF;
	GPIO_PORTB_CR_R 	|= 0xFF;
	GPIO_PORTC_CR_R 	|= 0xFF;
	GPIO_PORTD_CR_R 	|= 0xFF;
	GPIO_PORTE_CR_R 	|= 0xFF;
	GPIO_PORTF_CR_R 	|= 0xFF;

	GPIO_PORTF_PUR_R = 0x10;  //10 
}

void delay(double period){   //in milliseconds (maximum 1 s)
	uint32_t ticks = ticks_per_ms * period;
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_CURRENT_R = 0x000000;
	NVIC_ST_RELOAD_R = (ticks-1)&0xFFFFFF;
	NVIC_ST_CTRL_R = 0x5;
	uint32_t intr=NVIC_ST_CTRL_R&0x10000 ;
	while((NVIC_ST_CTRL_R&0x10000)== 0){intr=NVIC_ST_CTRL_R&0x10000;}

}


void trig(void){
	GPIO_PORTA_DATA_R &= 0xFB;  //trig low
	delay(10);
	GPIO_PORTA_DATA_R |= 0x04;  //trig high
	delay(2);
	GPIO_PORTA_DATA_R &= 0xFB;	//trig low
}

uint32_t echo(void){
	while((GPIO_PORTA_DATA_R & 0x08) == 0){NVIC_ST_CTRL_R = 0;}   //wait till echo high
	NVIC_ST_CURRENT_R = 0x000000;
	NVIC_ST_RELOAD_R = 0xFFFFFE;
	uint32_t Current ;
	NVIC_ST_CTRL_R |= 0x5;
	uint32_t inter=NVIC_ST_CTRL_R&0x10000;
	while((GPIO_PORTA_DATA_R & 0x08) != 0){Current=NVIC_ST_CURRENT_R&0xFFFFFF;}
	
	double tick = (0xFFFFFE - Current)/2;
	double time = tick / (double)ticks_per_ms; 
	return time*speed;
}

uint32_t measure_distance(void){
	uint32_t sum ;
for (int i=0 ;i<10;i++)
	{
		trig();
		sum += echo();
	}
		GPIO_PORTA_DATA_R &= 0xF3;  //clear trig and echo (mesh 3aref leh)
		
	 return sum/10;
}
void decodeUnits(int digit){
	switch (digit){
		case 0: PB0 = PB1 = PB2 =PB5 =PE0 =PF2 = 1; PF3 = 0; break;
		case 1: PB1 = PB2 = 1 ; PB0 =PB5 =PE0 =PF2 =PF3 = 0; break;
		case 2: PB0 = PB1 = PB5 = PE0 = PF3 = 1; PB2 = PF2 = 0; break;
		case 3: PB0 = PB1 = PF3 = PB2 = PB5 = 1; PE0 = PF2 = 0; break;
		case 4:PB0 = PB5 = PE0 = 0; PB1 = PB2 = PF2 = PF3 =1; break;
		case 5:PB1 = PE0 = 0 ; PB0 = PB2 = PB5 = PF2 = PF3 = 1; break;
		case 6:PB1 = 0 ; PB0 = PB2 = PB5 = PE0 = PF2 = PF3 = 1; break;
		case 7: PB0 = PB1 = PB2 = 1 ; PB5 = PE0= PF2= PF3 = 0;break;
		case 8:PB0 = PB1 = PB2 = PB5 = PE0 = PF2 = PF3 =1; break;
		case 9: PE0 = 0 ; PB0 = PB1 = PB2 = PB5  = PF2 = PF3 =1; break;
		case 10: PB0 = PE0 = PF2 = PF3 = 1; PB1 = PB2 = PB5 = 0; break;
		case 11: PE0 = PF3 = 1; PB0 = PB1 = PB2 = PB5 = PF2 = 0 ; break;
	}
}
	void decodeTens(int digit){
	switch (digit){
		case 0: PB6 = PD2 = PC4 =PC5 =PC6 =PC7 = 1; PB3 = 0; break;
		case 1: PD2 = PC4 = 1 ; PB6 =PC5 =PC6 =PC7 =PB3 = 0; break;
		case 2: PB6 = PD2 = PC5 = PC6 = PB3 = 1; PC4 = PC7 = 0; break;
		case 3: PB6 = PD2 = PB3 = PC4 = PC5 = 1; PC6 = PC7 = 0; break;
		case 4 : PB6 = PC5 = PC6 = 0; PD2 = PC4 = PC7 = PB3 =1; break;
		case 5: PD2 = PC6 = 0 ; PB6 = PC4 = PC5 = PC7 = PB3 = 1; break;
		case 6: PD2 = 0 ; PB6 = PC4 = PC5 = PC6 = PC7 = PB3 = 1; break;
		case 7: PB6 = PD2 = PC4 = 1 ; PC5 = PC6= PC7= PB3 = 0; break;
		case 8: PB6 = PD2 = PC4 = PC5 = PC6 = PC7 = PB3 =1; break;
		case 9: PC6 = 0 ; PB6 = PD2 = PC4 = PC5  = PC7 = PB3 =1; break;
		case 10: PB6 = PC6 = PC7 = PB3 = 1; PD2 = PC4 = PC5 = 0; break;
		case 11: PC6 = PB3 = 1; PB6 = PD2 = PC4 = PC5 = PC7 = 0; break;
	}
}
	void decodeHundreds(int digit){
	
		if(digit==0) {PA5 = PA6 = PD1 =PE1 =PE2 =PE3 = 1; PF1 = 0;}
	else	if(digit==1){PA6 = PD1 = 1 ; PA5 =PE1 =PE2 =PE3 =PF1 = 0;} 
	else	if(digit==2){ PA5 = PA6 = PE1 = PE2 = PF1 = 1; PD1 = PE3 = 0;}
	else	if(digit==3){ PA5 = PA6 = PF1 = PD1 = PE1 = 1; PE2 = PE3 = 0;}
	else	if(digit==4) {PA5 = PE1 = PE2 = 0; PA6 = PD1 = PE3 = PF1 =1; }
	else	if(digit==5){ PA6 = PE2 = 0 ; PA5 = PD1 = PE1 = PE3 = PF1 = 1;} 
	else	if(digit==6){PA6 = 0 ; PA5 =1; PD1 =1; PE1 =1; PE2 =1; PE3 = 1;PF1 = 1; }
	else	if(digit==7) {PA5 = PA6 = PD1 = 1 ; PE1 = PE2= PE3= PF1 = 0; }
	else	if(digit==8) {PA5 = PA6 = PD1 = PE1 = PE2 = PE3 = PF1 =1; }
	else	if(digit==9){ PE2 = 0 ; PA5 = PA6 = PD1 = PE1  = PE3 = PF1 =1;} 
	else	if(digit==10){PA5 = PE2 = PE3 = PF1 = 1; PA6 = PD1 = PE1 = 0; }
	else	if(digit==11){ PE2 = PF1 = 1; PA5 = PA6 = PD1 = PE1 = PE3 = 0; }
	
	}
	void Display()
	{
	
	GPIO_PORTA_DATA_R=(GPIO_PORTA_DATA_R & 0xFFFFFF9F)|( ((PA5<<5)|(PA6<<6)) & 0x00000060);
  GPIO_PORTE_DATA_R=(GPIO_PORTE_DATA_R & 0xFFFFFFF0)| ((PE0 |(PE1<<1)|(PE2<<2)|(PE3<<3)) & 0x0000000F);
	GPIO_PORTF_DATA_R=(GPIO_PORTF_DATA_R & 0xFFFFFFF1)| (((PF1<<1)|(PF2<<2)|(PF3<<3)) & 0x0000000E);
   GPIO_PORTC_DATA_R=(GPIO_PORTC_DATA_R & 0xFFFFFF0F)|  (((PC4<<4) |(PC5<<5)|(PC6<<6)|(PC7<<7)) & 0x000000F0) ;
 GPIO_PORTB_DATA_R=(GPIO_PORTB_DATA_R & 0xFFFFFF90)|   ((PB0 |(PB1<<1)|(PB2<<2)|(PB3<<3)|(PB5<<5)|(PB6<<6)) & 0x0000006F);
	GPIO_PORTD_DATA_R=(GPIO_PORTD_DATA_R & 0xFFFFFFF9)|( ((PD2<<2)|(PD1<<1)) & 0x00000006);


	}
int main(void){
	init();
	 uint32_t distance ;
		while(1)
		{
		 	
		
			
			while( (GPIO_PORTF_DATA_R & 0x10) == 0x10 ){}
		 distance = measure_distance();
		
		if(distance > 300){
		decodeUnits(11) ;
		decodeTens(10);
		decodeHundreds(0);
	              }
		else
			{
	decodeUnits(distance%10);
	distance/=10;
	decodeTens(distance%10) ;
distance/=10;
	decodeHundreds(distance);	
				
			}
			Display();
	}
}
