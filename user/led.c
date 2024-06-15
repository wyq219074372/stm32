#include "led.h"
#include "stm32f10x.h"

#define COL_ALL GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3
#define ROW_ALL GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7
#define BEEP PBout(8)        //BEEP 

static __IO u32 TimingDelay;
void SysTick_Init(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	if(SysTick_Config(SystemCoreClock /1000))
	{
		while(1);
	}

	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void Delay_ms(__IO u32 nTime)
{
  TimingDelay = nTime;	
  SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
  while(TimingDelay != 0);
}

void TimingDelay_Decrement(void)
{
	if (TimingDelay != 0x00)
	{
		TimingDelay--;
  }
}


void KeyBoard_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		
	GPIO_InitStructure.GPIO_Pin = COL_ALL;//pa0--pa3对应从左到右四根列线
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//置为推挽输出模式
	GPIO_Init(GPIOA, &GPIO_InitStructure);
		
	GPIO_InitStructure.GPIO_Pin = ROW_ALL;//pa4--pa7对应从上到下四根行线
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//置为下拉输入模式
	GPIO_Init(GPIOA, &GPIO_InitStructure);
		
	GPIO_SetBits(GPIOA, COL_ALL);//列线全部置1
}

/*
   4*4的矩阵键盘
   从上到下，从左到右
   键号依次为0-3,
             4-7，
             8,9,A,B，
             C,D,E,F
*/
s16 Read_KeyValue(void)
{
	s16 KeyValue=-1;//无按键
	
	if((GPIO_ReadInputData(GPIOA)&0xff)!=0x0f)//列线全部置1后读入的行线值不等于全0，则表明有键按下
	{
		//Delay_ms(2);//软件消抖
		if((GPIO_ReadInputData(GPIOA)&0xff)!=0x0f)//延时等待后如果读入的行值仍然不全等于0，则确认有键按下
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_0);//仅第1列列线置1,检查按键是否位于第1列
			GPIO_ResetBits(GPIOA, GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3);//第2、3、4列列线置0
			switch(GPIO_ReadInputData(GPIOA)&0xff)//获取PA口低8位PA[7..4]PA[3..0]
			{
				case 0x11: KeyValue = 0; break;//PA[7..4]PA[3..0]=0001_0001表明第1行第1列之间的按键闭合
				case 0x21: KeyValue = 4; break;//PA[7..4]PA[3..0]=0010_0001表明第2行第1列之间的按键闭合
				case 0x41: KeyValue = 8; break;//PA[7..4]PA[3..0]=0100_0001表明第3行第1列之间的按键闭合
				case 0x81: KeyValue = 0x0C;break;//PA[7..4]PA[3..0]=1000_0001表明第4行第1列之间的按键闭合
			}
			GPIO_SetBits(GPIOA, GPIO_Pin_1);//仅第2列列线置1，检查按键是否位于第2列
			GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3);//第1、3、4列列线置0
			switch(GPIO_ReadInputData(GPIOA)&0xff)//获取PA口低8位PA[7..4]PA[3..0]
			{
				case 0x12: KeyValue = 1; break;//PA[7..4]PA[3..0]=0001_0010表明第1行第2列之间的按键闭合
				case 0x22: KeyValue = 5; break;//PA[7..4]PA[3..0]=0010_0010表明第2行第2列之间的按键闭合
				case 0x42: KeyValue = 9;break;//PA[7..4]PA[3..0]=0100_0010表明第3行第2列之间的按键闭合
				case 0x82: KeyValue = 0x0D;break;//PA[7..4]PA[3..0]=1000_0010表明第4行第2列之间的按键闭合
			}
			GPIO_SetBits(GPIOA, GPIO_Pin_2);//仅第3列列线置1，检查按键是否位于第3列
			GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3);
			switch(GPIO_ReadInputData(GPIOA)&0xff)
			{
				case 0x14: KeyValue = 2; break;
				case 0x24: KeyValue = 6; break;
				case 0x44: KeyValue = 0x0A;break;
				case 0x84: KeyValue = 0x0E;break;
			}
			GPIO_SetBits(GPIOA, GPIO_Pin_3);//仅第4列列线置1，检查按键是否位于第4列
			GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2);
			switch(GPIO_ReadInputData(GPIOA)&0xff)
			{
				case 0x18: KeyValue = 3; break;
				case 0x28: KeyValue = 7; break;
				case 0x48: KeyValue = 0x0B;break;
				case 0x88: KeyValue = 0x0F;break;
			}
			GPIO_SetBits(GPIOA, COL_ALL);//全部列线置1
			while((GPIO_ReadInputData(GPIOA)&0xff)!=0x0f);//等待按键释放
			return KeyValue;
		}
	}
	return -1;//没有键按下
}

void NixieTubes_Init(void)
 {

	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE,ENABLE);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = 0xff;//使用Pin0--Pin7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);  //GPIOD作为位选
	GPIO_Init(GPIOE, &GPIO_InitStructure);  //GPIOE作为段选
 }
 
 void NixieDisplay(s16 value)
 {
	 //DP G F E D C B A
	 //0  0 1 1 1 1 1 1
	 //以上为适合共阴接法的 0 的字形码
		u16 num[16] = {0x3f, 0x06, 0x5b, 0x4f, 
									 0x66, 0x6d, 0x7d, 0x07, 
									 0x7f, 0x6F, 0x77, 0x7c, 
									 0x39, 0x5e, 0x79, 0x71};
		s16 j;
		j=value;

		if(j>=0)
		{	
		//PD[7..0]对应选择从左到右的8个数码管，低电平选用，高电平禁用		
			GPIOD->ODR = 0xff;//关闭全部显示		
			GPIOE->ODR = ~num[j];//字形表格按共阴接法设计，对于共阳接法，取反使用
			GPIOD->ODR = 0x00;//选择全部管子
			Delay_ms(2);
		}
}



void LED_GPIO_Config(void)
{		
	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*开启GPIOC的外设时钟*/
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE); 

	/*选择要控制的GPIOC引脚*/															   
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;	

	/*设置引脚模式为通用推挽输出*/
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

	/*设置引脚速率为50MHz */   
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*调用库函数，初始化GPIOC*/
  	GPIO_Init(GPIOC, &GPIO_InitStructure);		  

	/* 关闭所有led灯	*/
	GPIO_SetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);	 
}

//h g f e d c b a
//1 1 0 0 0 0 0 0 --共阳方式显示0   
u8 seg_tab2[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
void LED_Init2(void)
{
 
	 GPIO_InitTypeDef  GPIO_InitStructure;
		
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	
		
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;				
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
	 GPIO_Init(GPIOD, &GPIO_InitStructure);				
	 GPIO_SetBits(GPIOD,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);


	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);//??PORTA,PORTE??

	 GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;	//KEY0-KEY2
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //???????
 	 GPIO_Init(GPIOF, &GPIO_InitStructure);//???GPIOE2,3,4
	 GPIO_SetBits(GPIOF,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9);

}

void Display2(u8 index)	//从字模数组中取出数据，处理某一行的8列
{
	FX0 = seg_tab2[index]&0x01;//pa处理列值
	FX1 = (seg_tab2[index]>>1)&0x01;
	FX2 = (seg_tab2[index]>>2)&0x01;
	FX3 = (seg_tab2[index]>>3)&0x01;
	FX4 = (seg_tab2[index]>>4)&0x01;
	FX5 = (seg_tab2[index]>>5)&0x01;
	FX6 = (seg_tab2[index]>>6)&0x01;
	FX7 = (seg_tab2[index]>>7)&0x01;
	FX8 = (seg_tab2[index]>>8)&0x01;
	FX9 = (seg_tab2[index]>>9)&0x01;
	

}


void Beep_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	PBout(9) = 0;
	//GPIO_ResetBits(GPIOB, GPIO_Pin_9);
}

void Beep_on(void)
{
	PBout(9) = 1;
}

void Beep_off(void)
{
	PBout(9) = 0;
}


/////

u8 seg_tab[88] = {
									0xff, 0x99, 0x66, 0x7e, 0x7e, 0xbd, 0xdb, 0xe7,      //心
									//0xe7, 0xdb, 0x99, 0x99, 0x99, 0x99, 0xdb, 0xe7,      //0
	                0xe7,0xdb,0xdb,0xdb,0xdb,0xdb,0xdb,0xe7,//0
									0xe7, 0xc7, 0xa7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7,      //1
									0xc3, 0x99, 0x99, 0xf1, 0xe3, 0xc7, 0x8f, 0x81,      //2
									0xc3, 0x99, 0xf9, 0xe3, 0xe3, 0xf9, 0x99, 0xc3,      //3
									0xf3, 0xe3, 0xc3, 0x93, 0xb3, 0x81, 0xf3, 0xf3,      //4
									0xc3, 0x9f, 0x9f, 0x83, 0xc1, 0xf9, 0xf9, 0xc3,      //5
									0xc3, 0x9f, 0x9f, 0x83, 0x81, 0x99, 0x99, 0xc3,      //6
									0x81, 0x81, 0xf9, 0xf1, 0xe3, 0xe7, 0xe7, 0xe7,      //7
									0xc3, 0x99, 0x99, 0x81, 0x81, 0x99, 0x99, 0xc3,      //8
									0xc3, 0x99, 0x99, 0x81, 0xc1, 0xf9, 0xf9, 0xc3       //9				
};

void LED_Init(void){
	GPIO_InitTypeDef  GPIO_InitStructure;
		
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
		
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;				
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
	 GPIO_Init(GPIOB, &GPIO_InitStructure);				
	 GPIO_SetBits(GPIOB,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10);


	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);//??PORTA,PORTE??

	 GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;//KEY0-KEY2
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //???????
 	 GPIO_Init(GPIOE, &GPIO_InitStructure);//???GPIOE2,3,4
	 GPIO_ResetBits(GPIOE,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);
}


void Display(u8 index)	{
	DX0 = seg_tab[index]&0x01;//pa处理列值
	DX1 = (seg_tab[index]>>1)&0x01;
	DX2 = (seg_tab[index]>>2)&0x01;
	DX3 = (seg_tab[index]>>3)&0x01;
	DX4 = (seg_tab[index]>>4)&0x01;
	DX5 = (seg_tab[index]>>5)&0x01;
	DX6 = (seg_tab[index]>>6)&0x01;
	DX7 = (seg_tab[index]>>7)&0x01;
	
}
void xin(void){
				Display(0);
    		D0=0;
				delay_ms(1);
				D0=1;
		
				Display(1);
    		D1=0;
				delay_ms(1);
				D1=1;
		
				Display(2);
    		D2=0;
				delay_ms(1);
				D2=1;
		
			  Display(3);
    		D3=0;
				delay_ms(1);
				D3=1;
		
				Display(4);
    		D4=0;
				delay_ms(1);
				D4=1;
				
				Display(5);
    		D5=0;
				delay_ms(1);
				D5=1;
				
				Display(6);
    		D6=0;
				delay_ms(1);
				D6=1;
				
				Display(7);
    		D7=0;
				delay_ms(1);
				D7=1;
}
void zero(void){
				
				Display(8);
    		D0=0;//pe处理行值，先擦
				delay_ms(1);
				D0=1;//pe处理行值，后显示
		
				Display(9);
    		D1=0;
				delay_ms(1);
				D1=1;
		
				Display(10);
    		D2=0;
				delay_ms(1);
				D2=1;
		
			  Display(11);
    		D3=0;
				delay_ms(1);
				D3=1;
		
				Display(12);
    		D4=0;
				delay_ms(1);
				D4=1;
				
				Display(13);
    		D5=0;
				delay_ms(1);
				D5=1;
				
				Display(14);
    		D6=0;
				delay_ms(1);
				D6=1;
				
				Display(15);
    		D7=0;
				delay_ms(1);
				D7=1;
}
void one(void){
				Display(16);
    		D0=0;
				delay_ms(1);
				D0=1;
		
				Display(17);
    		D1=0;
				delay_ms(1);
				D1=1;
		
				Display(18);
    		D2=0;
				delay_ms(1);
				D2=1;
		
			  Display(19);
    		D3=0;
				delay_ms(1);
				D3=1;
		
				Display(20);
    		D4=0;
				delay_ms(1);
				D4=1;
				
				Display(21);
    		D5=0;
				delay_ms(1);
				D5=1;
				
				Display(22);
    		D6=0;
				delay_ms(1);
				D6=1;
				
				Display(23);
    		D7=0;
				delay_ms(1);
				D7=1;
}

void two(void){
				Display(24);
    		D0=0;
				delay_ms(1);
				D0=1;
		
				Display(25);
    		D1=0;
				delay_ms(1);
				D1=1;
		
				Display(26);
    		D2=0;
				delay_ms(1);
				D2=1;
		
			  Display(27);
    		D3=0;
				delay_ms(1);
				D3=1;
		
				Display(28);
    		D4=0;
				delay_ms(1);
				D4=1;
				
				Display(29);
    		D5=0;
				delay_ms(1);
				D5=1;
				
				Display(30);
    		D6=0;
				delay_ms(1);
				D6=1;
				
				Display(31);
    		D7=0;
				delay_ms(1);
				D7=1;
}
void three(void){
				Display(32);
    		D0=0;
				delay_ms(1);
				D0=1;
		
				Display(33);
    		D1=0;
				delay_ms(1);
				D1=1;
		
				Display(34);
    		D2=0;
				delay_ms(1);
				D2=1;
		
			  Display(35);
    		D3=0;
				delay_ms(1);
				D3=1;
		
				Display(36);
    		D4=0;
				delay_ms(1);
				D4=1;
				
				Display(37);
    		D5=0;
				delay_ms(1);
				D5=1;
				
				Display(38);
    		D6=0;
				delay_ms(1);
				D6=1;
				
				Display(39);
    		D7=0;
				delay_ms(1);
				D7=1;
}
void four(void){
				Display(40);
    		D0=0;
				delay_ms(1);
				D0=1;
		
				Display(41);
    		D1=0;
				delay_ms(1);
				D1=1;
		
				Display(42);
    		D2=0;
				delay_ms(1);
				D2=1;
		
			  Display(43);
    		D3=0;
				delay_ms(1);
				D3=1;
		
				Display(44);
    		D4=0;
				delay_ms(1);
				D4=1;
				
				Display(45);
    		D5=0;
				delay_ms(1);
				D5=1;
				
				Display(46);
    		D6=0;
				delay_ms(1);
				D6=1;
				
				Display(47);
    		D7=0;
				delay_ms(1);
				D7=1;
}
void five(void){}
void six(void){}
void seven(void){}
void eight(void){}
void nine(void){}


/******************* (C) COPYRIGHT 2012 WildFire Team *****END OF FILE************/
