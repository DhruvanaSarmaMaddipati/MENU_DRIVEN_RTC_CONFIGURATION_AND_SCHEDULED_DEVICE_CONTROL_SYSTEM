//test.c

#include <lpc21xx.h>
#include "defines.h"
#include "mini_proj.h"
#include"delay.h"
#include"mini_defines.h"
int main(void)

{
u32 h,m,s,date,mon,yr,day;
IODIR0|=1<<LED_PIN;
IOCLR0|=1<<LED_PIN;

Init_LCD();
InitKPM();
RTC_Init();
eint1_enable();

SetRTCTimeInfo(9,58,7);
SetRTCDateInfo(29,10,2026);
SetRTCDayInfo(4);

while(1)
{
if(menu_req_flag)
{
     execution_menu_loop();
         lcd_clear();
}
else
{
GetRTCTimeInfo((u32*)&h,(u32*)&m,(u32*)&s);
GetRTCDateInfo((u32*)&date,(u32*)&mon,(u32*)&yr);
GetRTCDayInfo((u32*)&day);

DisplayRTCTime(h,m,s);
DisplayRTCDate(date,mon,yr);
DisplayRTCDay(day);

check_cntrl_LED();
delay_ms(200);
}
}
}
