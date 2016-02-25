#ifndef __PYINPUT_H
#define __PYINPUT_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序参考自网络并加以修改
//ALIENTEK战舰STM32开发板
//拼音输入法 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/22
//版本：V1.0			    
//广州市星翼电子科技有限公司  	 												    								  
//////////////////////////////////////////////////////////////////////////////////


//拼音码表与拼音的对应表
typedef struct
{
  u8 *py_input;//输入的字符串
  u8 *py;	   //对应的拼音
  u8 *pymb;	   //码表
}py_index;

#define MAX_MATCH_PYMB 	10	//最大匹配数

//拼音输入法
typedef struct
{
  u8(*getpymb)(u8 *instr);			//字符串到码表获取函数
  py_index *pymb[MAX_MATCH_PYMB];	//码表存放位置
}pyinput;

extern pyinput t9;
u8 str_match(u8*str1,u8*str2);
u8 get_matched_pymb(u8 *strin,py_index **matchlist);
u8 get_pymb(u8* str);
void test_py(u8 *inputstr);
#endif








