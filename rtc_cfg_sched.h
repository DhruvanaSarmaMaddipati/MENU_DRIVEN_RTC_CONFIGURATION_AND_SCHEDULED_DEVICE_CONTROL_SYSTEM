//proj.h
#include <stdbool.h>
#include "types.h"
typedef struct{
unsigned char hour;
unsigned char min;
unsigned char sec;
}Timeschedule;


extern volatile bool menu_req_flag;
extern bool schedule_val;
extern Timeschedule on_time;
extern Timeschedule off_time;
bool is_leap_yr(u32 yr);
bool validate_date(u8 d,u8 m,u8 y);

//LCD
void Init_LCD(void);
void WRITE_LCD_CMD(u8 cmd);
void WRITE_LCD_DATA(u8 ascii);
void strlcd(s8 *str);
void lcd_clear(void);
void U32LCD(u32 n);

//keypad
void InitKPM(void);
u32 key_scan(void);
u32 colscan(void);
u32 rowcheck(void);
u32 colcheck(void);
u32 input_num(const char*str,u8 max_len);

//ext interrupt
void eint1_enable(void);

//RTC
void RTC_Init(void);
void SetRTCTimeInfo(u32 hour,u32 min,u32 sec);
void SetRTCDateInfo(u32 date,u32 mon,u32 yr);
void SetRTCDayInfo(u32 day);
void GetRTCTimeInfo(u32 *hour,u32 *min,u32 *sec);
void GetRTCDateInfo(u32 *date,u32 *mon,u32 *yr);
void GetRTCDayInfo(u32 *day);

void DisplayRTCTime(u32 hour,u32 min,u32 sec);
void DisplayRTCDate(u32 date,u32 mon,u32 yr);
void DisplayRTCDay(u32 day);

//control and menu
void execution_menu_loop(void);
void check_cntrl_LED(void);