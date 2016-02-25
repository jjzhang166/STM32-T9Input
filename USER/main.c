#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"   
#include "lcd.h"   
#include "key.h" 
#include "usmart.h" 
#include "malloc.h"  
#include "MMC_SD.h" 
#include "ff.h"  
#include "exfuns.h"
#include "fontupd.h"
#include "text.h"	   
#include "touch.h"
#include "pyinput.h"
#include "string.h"
//ALIENTEK Mini STM32开发板扩展实验8
//T9拼音输入法实验
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司  
   

   							   	 
//数字表
const u8* kbd_tbl[9]={"←","2","3","4","5","6","7","8","9",};
//字符表
const u8* kbs_tbl[9]={"DEL","abc","def","ghi","jkl","mno","pqrs","tuv","wxyz",};

//加载键盘界面
//x,y:界面起始坐标
void py_load_ui(u16 x,u16 y)
{
	u16 i;
	POINT_COLOR=RED;
	LCD_DrawRectangle(x,y,x+180,y+120);						   
	LCD_DrawRectangle(x+60,y,x+120,y+120);						   
	LCD_DrawRectangle(x,y+40,x+180,y+80);
	POINT_COLOR=BLUE;
	for(i=0;i<9;i++)
	{
		Show_Str_Mid(x+(i%3)*60,y+4+40*(i/3),(u8*)kbd_tbl[i],16,60);		
		Show_Str_Mid(x+(i%3)*60,y+20+40*(i/3),(u8*)kbs_tbl[i],16,60);		
	}  		 					   
}
//按键状态设置
//x,y:键盘坐标
//key:键值（0~8）
//sta:状态，0，松开；1，按下；
void py_key_staset(u16 x,u16 y,u8 keyx,u8 sta)
{		  
	u16 i=keyx/3,j=keyx%3;
	if(keyx>8)return;
	if(sta)LCD_Fill(x+j*60+1,y+i*40+1,x+j*60+59,y+i*40+39,GREEN);
	else LCD_Fill(x+j*60+1,y+i*40+1,x+j*60+59,y+i*40+39,WHITE); 
	Show_Str_Mid(x+j*60,y+4+40*i,(u8*)kbd_tbl[keyx],16,60);		
	Show_Str_Mid(x+j*60,y+20+40*i,(u8*)kbs_tbl[keyx],16,60);		 
}
//得到触摸屏的输入
//x,y:键盘坐标
//返回值：按键键值（1~9有效；0,无效）
u8 py_get_keynum(u16 x,u16 y)
{
	u16 i,j;
	static u8 key_x=0;//0,没有任何按键按下；1~9，1~9号按键按下
	u8 key=0;
	tp_dev.scan(0); 		 
	if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
	{	
		for(i=0;i<3;i++)
		{
			for(j=0;j<3;j++)
			{
			 	if(tp_dev.x[0]<(x+j*60+60)&&tp_dev.x[0]>(x+j*60)&&tp_dev.y[0]<(y+i*40+40)&&tp_dev.y[0]>(y+i*40))
				{	
					key=i*3+j+1;	 
					break;	 		   
				}
			}
			if(key)
			{	   
				if(key_x==key)key=0;
				else 
				{
					py_key_staset(x,y,key_x-1,0);
					key_x=key;
					py_key_staset(x,y,key_x-1,1);
				}
				break;
			}
		}  
	}else if(key_x) 
	{
		py_key_staset(x,y,key_x-1,0);
		key_x=0;
	} 
	return key; 
}
							   
//显示结果.
//index:0,表示没有一个匹配的结果.清空之前的显示
//   其他,索引号	
void py_show_result(u8 index)
{
 	LCD_ShowNum(30+144,125,index,1,16); 		//显示当前的索引
	LCD_Fill(30+40,125,30+40+48,130+16,WHITE);	//清除之前的显示
 	LCD_Fill(30+40,145,30+200,145+48,WHITE);	//清除之前的显示    
	if(index)
	{
 		Show_Str(30+40,125,200,16,t9.pymb[index-1]->py,16,0); 	//显示拼音
		Show_Str(30+40,145,160,48,t9.pymb[index-1]->pymb,16,0); //显示对应的汉字
		printf("\r\n拼音:%s\r\n",t9.pymb[index-1]->py);	//串口输出拼音
		printf("结果:%s\r\n",t9.pymb[index-1]->pymb);	//串口输出结果
	}
}	 	


 int main(void)
 { 
 	u8 i=0;	    	  	    
	u8 result_num;
	u8 cur_index;
	u8 key;
	u8 inputstr[7];		//最大输入6个字符+结束符
	u8 inputlen;		//输入长度	   
		 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2 
	delay_init();	    	 //延时函数初始化	  
	uart_init(9600);	 	//串口初始化为9600			 
	LCD_Init();			//初始化液晶 
	LED_Init();         //LED初始化	 
	KEY_Init();			//按键初始化	  													    
	TP_Init();     		//触摸屏初始化
	usmart_dev.init(72);//usmart初始化	
 	mem_init();	//初始化内部内存池	

RESTART:
	POINT_COLOR=RED;      
 	while(font_init()) 				//检查字库
	{	    
		LCD_ShowString(60,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(60,50,240,66,WHITE);//清除显示	     
	}
 	Show_Str(60,5,200,16,"Mini STM32开发板",16,0);				    	 
	Show_Str(60,25,200,16,"拼音输入法实验",16,0);				    	 
	Show_Str(60,45,200,16,"正点原子@ALIENTEK",16,0);				    	 
 	Show_Str(30,65,200,16,"  KEY1:上翻  KEY0:下翻",16,0);	 
 	Show_Str(30,85,200,16,"KEY_UP:重新输入",16,0);	 
	Show_Str(30,105,200,16,"输入:        匹配:  ",16,0);
 	Show_Str(30,125,200,16,"拼音:        当前:  ",16,0);
 	Show_Str(30,145,210,32,"结果:",16,0);
	py_load_ui(30,195);
	memset(inputstr,0,7);	//全部清零
	inputlen=0;				//输入长度为0
	result_num=0;			//总匹配数清零
	cur_index=0;			
	while(1)
	{
		i++;
		delay_ms(10);
		key=py_get_keynum(30,195);
		if(key)
		{
			if(key==1)//删除
			{
				if(inputlen)inputlen--;
				inputstr[inputlen]='\0';//添加结束符
			}else 
			{
				inputstr[inputlen]=key+'0';//输入字符
				if(inputlen<7)inputlen++;
			}
			if(inputstr[0]!=NULL)
			{
				key=t9.getpymb(inputstr);	//得到匹配的结果数
				if(key)//有部分匹配/完全匹配的结果
				{
					result_num=key;			//总匹配结果
					cur_index=1;			//当前为第一个索引 
					if(key&0X80)		   	//是部分匹配
					{
						inputlen=key&0X7F;	//有效匹配位数
						inputstr[inputlen]='\0';//不匹配的位数去掉
						if(inputlen>1)result_num=t9.getpymb(inputstr);//重新获取完全匹配字符数
					}  
				}else 						//没有任何匹配
				{				   	
					inputlen--;
					inputstr[inputlen]='\0';
				}
			}else
			{
				cur_index=0;
				result_num=0;
			}
			LCD_Fill(30+40,105,30+40+48,110+16,WHITE);	//清除之前的显示
			LCD_ShowNum(30+144,105,result_num,1,16); 	//显示匹配的结果数
			Show_Str(30+40,105,200,16,inputstr,16,0);	//显示有效的数字串		 
	 		py_show_result(cur_index);					//显示第cur_index的匹配结果
		}	 
		if(result_num)	//存在匹配的结果	
		{	  
			key=KEY_Scan(0);
			switch(key)
			{
				case KEY1_PRES://上翻
					if(cur_index<result_num)cur_index++;
					else cur_index=1;
					py_show_result(cur_index);	//显示第cur_index的匹配结果
					break;
 				case KEY0_PRES://下翻
	   				if(cur_index>1)cur_index--;
					else cur_index=result_num;
					py_show_result(cur_index);	//显示第cur_index的匹配结果
					break;
				case WKUP_PRES://清除输入
 					LCD_Fill(30+40,145,30+200,145+48,WHITE);	//清除之前的显示    
					goto RESTART;	 		 	   
			}   	 
		}
		if(i==30)
		{
			i=0;
			LED0=!LED0;
		}		   
	}     										    			    
}


