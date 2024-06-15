#include "timer.h"
#include "led.h"
u8 flag=0;

void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //????
	
	//???TIM3???
	TIM_TimeBaseStructure.TIM_Period = arr; //???????????????????????????	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //??????TIMx???????????
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //??????:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM??????
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //??????????TIMx???????
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //?????TIM3??,??????

	//?????NVIC??
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3??
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //?????0?
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //????3?
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ?????
	NVIC_Init(&NVIC_InitStructure);  //???NVIC???


	TIM_Cmd(TIM3, ENABLE);  //??TIMx					 
}
//???3??????
void TIM3_IRQHandler(void)   //TIM3??
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //??TIM3????????
		{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //??TIMx?????? 
		flag++;
			if(flag==12)
			{
				flag=0;	
			}
			
		}
}

void display(void)
{
			while(1)
						{
							
							switch(flag)
							{
								case 1:	nine();break;
								case 2:	eight();break;
								case 3:	seven();break;
								case 4:	six();break;
								case 5:	five();break;
								case 6:	four();break;
								case 7:	three();break;
								case 8:	two();break;
								case 9:	one();break;
								case 10:zero();break;
								case 11:xin();break;
								default:break;
							}		
						}	
}










