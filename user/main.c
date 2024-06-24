#include "stm32f10x.h"
#include "led.h"

//#define    MI_ERR    (-2)





#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif


static u8  fac_us=0;							
static u16 fac_ms=0;							
	
	
#if SYSTEM_SUPPORT_OS							
#ifdef 	OS_CRITICAL_METHOD							
#define delay_osrunning		OSRunning			
#define delay_ostickspersec	OS_TICKS_PER_SEC	
#define delay_osintnesting 	OSIntNesting		
#endif

//支持UCOSIII
#ifdef 	CPU_CFG_CRITICAL_METHOD					
#define delay_osrunning		OSRunning			
#define delay_ostickspersec	OSCfg_TickRate_Hz	
#define delay_osintnesting 	OSIntNestingCtr		
#endif

void delay_osschedlock(void)
{
#ifdef CPU_CFG_CRITICAL_METHOD   				
	OS_ERR err; 
	OSSchedLock(&err);							
#else										
	OSSchedLock();								
#endif
}

//us级延时时,恢复任务调度
void delay_osschedunlock(void)
{	
#ifdef CPU_CFG_CRITICAL_METHOD   				
	OS_ERR err; 
	OSSchedUnlock(&err);						
#else										
	OSSchedUnlock();						
#endif
}

void delay_ostimedly(u32 ticks)
{
#ifdef CPU_CFG_CRITICAL_METHOD
	OS_ERR err; 
	OSTimeDly(ticks,OS_OPT_TIME_PERIODIC,&err);	
#else
	OSTimeDly(ticks);						
#endif 
}
 
void SysTick_Handler(void)
{	
	if(delay_osrunning==1)				
	{
		OSIntEnter();							
		OSTimeTick();       					       
		OSIntExit();       	 	
	}
}
#endif

void delay_init()
{
#if SYSTEM_SUPPORT_OS  							
	u32 reload;
#endif
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	fac_us=SystemCoreClock/8000000;		
#if SYSTEM_SUPPORT_OS  							
	reload=SystemCoreClock/8000000;				
	reload*=1000000/delay_ostickspersec;		
										
	fac_ms=1000/delay_ostickspersec;		

	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;   	
	SysTick->LOAD=reload; 						
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;   

#else
	fac_ms=(u16)fac_us*1000;					
#endif
}								    

#if SYSTEM_SUPPORT_OS  				

void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;					 
	ticks=nus*fac_us; 								 
	tcnt=0;
	delay_osschedlock();						
	told=SysTick->VAL;        					
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;		//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;				//时间超过/等于要延迟的时间,则退出.
		}  
	};
	delay_osschedunlock();						//恢复OS调度									    
}
//延时nms
//nms:要延时的ms数
void delay_ms(u16 nms)
{	
	if(delay_osrunning&&delay_osintnesting==0)	
	{		 
		if(nms>=fac_ms)						
		{ 
   			delay_ostimedly(nms/fac_ms);		
		}
		nms%=fac_ms;							
	}
	delay_us((u32)(nms*1000));				
}
#else //不用OS时
		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 						 
	SysTick->VAL=0x00;        					
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		  
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	
	SysTick->VAL =0X00;      					
}

void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;			
	SysTick->VAL =0x00;							
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	
	SysTick->VAL =0X00;       					
} 
#endif 

	
u8 flag=0;

void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); 
	
	TIM_TimeBaseStructure.TIM_Period = arr; 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); 

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 


	TIM_Cmd(TIM3, ENABLE);  
}

void TIM3_IRQHandler(void)   
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  
		{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );
		flag++;
			if(flag==12)
			{
				flag=0;	
			}
			
		}
}

void display(void){}

void Delay(__IO u32 nCount);
	
s16 kval;
int g = 8, y = 2;	//
int lg = 0, ly = 0;
int sg = 0, sy = 0;
int rg = 0, ry = 0;
int sr = 10, lr = 20, rr = 20;
	
void changegree() {
		kval = Read_KeyValue();
		if (kval == 0) {
			lg = 0, ly = 0, lr = 20;
			sr = 20, sg = 9, sy = 3;
			rr = 10, rg = 9, ry = 3;
			LED1(OFF); LED3(OFF); LED5(OFF); LED6(OFF); LED7(OFF);
			LED2(ON); LED4(ON); LED8(ON);
		}else if (kval == 1) {
			lr = 10, lg = 0, ly = 0;
			sg = 0, sy = 0, sr = 10;
			rr = 20, rg = 9, ry = 3;
			LED2(OFF); LED3(OFF); LED4(OFF); LED6(OFF); LED7(OFF);
			LED1(ON); LED5(ON); LED8(ON);
		}else if (kval == 2) {
			lg = 0, ly = 0;
			sg = 0, sy = 0;
			rg = 0, ry = 0;
			sr = 10, lr = 20, rr = 20;
			LED2(OFF); LED3(OFF); LED5(OFF); LED6(OFF); LED8(OFF);
			LED1(ON); LED4(ON); LED7(ON);
		}
}

int main(void)
{	
		SysTick_Init();     //初始化系统时钟
		KeyBoard_Init();    //初始化矩阵键盘GPIOA
		//NixieTubes_Init();  //使用GPIOD、GPIOE控制数码管

	/* LED 端口初始化 */
	  delay_init();	     
		int i;
	  LED_Init();
		LED_GPIO_Config();
	  LED_Init2();		  	//数码管初始化函数
		/*
		while(1)
		{
			LED1(ON);
			LED2(ON);
		//	for(i=0;i<100;i++)eight();
			for(i=0;i<100;i++) xin();
			
		}
	*/
		while(1)
		{
				LED2(OFF);
				LED3(OFF);
				LED5(OFF);
				LED6(OFF);
				LED7(OFF);
				LED8(OFF);
				LED1( ON );
				Delay(0x0FFFEF);
				LED4( ON );
				Delay(0x0FFFEF);
				LED7( ON );
				Delay(0x0FFFEF);
			
				g = 8, y = 2;	//
				lg = 0, ly = 0;
				sg = 0, sy = 0;
				rg = 0, ry = 0;
				sr = 10, lr = 20, rr = 20;
					
				for (i = 0; i >= 0; i ++) {
						changegree();
						
						if (rg < g) {
							LED2(OFF); LED3(OFF); LED5(OFF); LED6(OFF); LED8(OFF);
							LED1(ON); LED4(ON); LED7(ON);
							rg ++;
							
								eight();                                                                                             
							int j;
							int a = lr / 10;
							int b = lr % 10;
							int c = sr / 10;
							int d = sr % 10;
							for (j = 100; j >= 0; j --) {
								eight();
								changegree();
								if (rg >= g ) break;
								Display2(a);F0=0;delay_ms(1);F0=1;
								Display2(b);F1=0;delay_ms(1);F1=1;
								
								Display2(c);F3=0;delay_ms(1);F3=1;
								Display2(d);F4=0;delay_ms(1);F4=1;
								
								Display2(0);F6=0;delay_ms(1);F6=1;
								Display2(g - rg);F7=0;delay_ms(1);F7=1;
							}
							
							lr --;sr --;
							continue;
						}
						changegree();
								
								nine();
						if (ry < y) {
							ry ++;
							if (ry == y) {
								BEEP_ON;
								delay_ms(500);
								BEEP_OFF;
							}
							
							int a = lr / 10;
							int b = lr % 10;
							int c = sr / 10;
							int d = sr % 10;
							int j;
							for (j = 100; j >= 0; j --) {
								nine();
								changegree();
								if (ry >= y ) break;
								Display2(a);F0=0;delay_ms(1);F0=1;
								Display2(b);F1=0;delay_ms(1);F1=1;
								
								Display2(c);F3=0;delay_ms(1);F3=1;
								Display2(d);F4=0;delay_ms(1);F4=1;
								
								Display2(0);F6=0;delay_ms(1);F6=1;
								Display2(y - ry);F7=0;delay_ms(1);F7=1;
							}
							
							lr --;
							sr --;
							LED8(ON);
							Delay(0x0FFFEF);
							LED7(OFF);
							
							continue;
						}
						changegree();
						
								nine();
						if (sg < g) {
							LED2(OFF); LED3(OFF); LED4(OFF); LED6(OFF); LED7(OFF);
							LED1(ON); LED5(ON); LED8(ON);
							sg ++;
						
							int a = lr / 10;
							int b = lr % 10;
							int c = rr / 10;
							int d = rr % 10;
							int j;
							for (j = 100; j >= 0; j --) {
								nine();
								changegree();
								if (sg >= g ) break;
								Display2(a);F0=0;delay_ms(1);F0=1;
								Display2(b);F1=0;delay_ms(1);F1=1;
								
								Display2(0);F3=0;delay_ms(1);F3=1;
								Display2(g - sg);F4=0;delay_ms(1);F4=1;
								
								Display2(c);F6=0;delay_ms(1);F6=1;
								Display2(d);F7=0;delay_ms(1);F7=1;
							}
							
							lr --;
							rr --;
							continue;
						}
						changegree();
								nine();
								
						if (sy < y) {
							sy ++;
							if (sy == y) {
								BEEP_ON;
								delay_ms(500);
								BEEP_OFF;
							}
							LED5(OFF);
							LED6(ON);
							Delay(0x0FFFEF);
							sr = 20;
							
							int j;
							int a = lr / 10;
							int b = lr % 10;
							int c = rr / 10;
							int d = rr % 10;
							for (j = 100; j >= 0; j --) {
								nine();
								changegree();
								if (sy >= y) break;
								Display2(a);F0=0;delay_ms(1);F0=1;
								Display2(b);F1=0;delay_ms(1);F1=1;
								
								Display2(0);F3=0;delay_ms(1);F3=1;
								Display2(y - sy);F4=0;delay_ms(1);F4=1;
								
								Display2(c);F6=0;delay_ms(1);F6=1;
								Display2(d);F7=0;delay_ms(1);F7=1;
							}
							
							lr --;
							rr --;
							continue;
						}
						
						changegree();
								nine();
								
						if (lg < g) {
							lg ++;
							
							LED1(OFF); LED3(OFF); LED5(OFF); LED6(OFF); LED7(OFF);
							LED2(ON); LED4(ON); LED8(ON);
							
							int j;
							int a = sr / 10;
							int b = sr % 10;
							int c = rr / 10;
							int d = rr % 10;
							for (j = 100; j >= 0; j --) {
								nine();
								changegree();
								if (lg >=g) break;
								Display2(0);F0=0;delay_ms(1);F0=1;
								Display2(g - lg);F1=0;delay_ms(1);F1=1;
								
								Display2(a);F3=0;delay_ms(1);F3=1;
								Display2(b);F4=0;delay_ms(1);F4=1;
								
								Display2(c);F6=0;delay_ms(1);F6=1;
								Display2(d);F7=0;delay_ms(1);F7=1;
							}
							
								sr --;
								rr --;
							continue;
						}
						changegree();
								nine();
								
						if (ly < y) {
							ly ++;
							if (ly == y) {
								BEEP_ON;
								delay_ms(500);
								BEEP_OFF;
							}
							LED4(ON);
							LED2(OFF);
							LED3(ON);
							Delay(0x0FFFEF);
							
							int j;
							int a = sr / 10;
							int b = sr % 10;
							int c = rr / 10;
							int d = rr % 10;
							for (j = 100; j >= 0; j --) {
								nine();
								changegree();
								if (ly >= y ) break;
								Display2(0);F0=0;delay_ms(1);F0=1;
								Display2(y - ly);F1=0;delay_ms(1);F1=1;
								
								Display2(a);F3=0;delay_ms(1);F3=1;
								Display2(b);F4=0;delay_ms(1);F4=1;
								
								Display2(c);F6=0;delay_ms(1);F6=1;
								Display2(d);F7=0;delay_ms(1);F7=1;
							}
							
								rr --;
								sr --;
							continue;
						}
						break;
				}
				
		}
	
}

void Delay(__IO u32 nCount)	 //简单的延时函数
{
	for(; nCount != 0; nCount--);
} 
