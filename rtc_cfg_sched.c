#include<lpc21xx.h>
#include"types.h"
#include "delay.h"
#include "mini_defines.h"
#include "mini_proj.h"
#include "defines.h"

extern volatile bool menu_req_flag=false;
Timeschedule on_time={9,0,0};
Timeschedule off_time={10,0,0};
bool schedule_val=true;



//LCD
void WRITE_LCD_CMD(u8 cmd)
{
        //perform write operation(rw=0)
        SCLRBIT(IOCLR0,LCD_RW);
        //select command register(rs=0)
        SCLRBIT(IOCLR0,LCD_RS);
        //write cmd on to the data pins
        WRITEBYTE(IOPIN0,LCD_DATA,cmd);
        //apply H to L pulse on EN
        SSETBIT(IOSET0,LCD_EN); //EN=1
        delay_us(1);
        SCLRBIT(IOCLR0,LCD_EN); //EN=0
        //delay for internal process
        delay_ms(2);
}

void Init_LCD(void)
{

        //cfg p0.8 t0 p0.15(lcd data pins) as output
        WRITEBYTE(IODIR0,LCD_DATA,0xFF);
        // cfg p0.16(rs),p0.17(rw) and p0.18(en) as output
        SETBIT(IODIR0,LCD_RS);
        SETBIT(IODIR0,LCD_RW);
        SETBIT(IODIR0,LCD_EN);
        delay_ms(15);
        WRITE_LCD_CMD(MODE_8BIT_1LINE);
        delay_ms(5);
        WRITE_LCD_CMD(MODE_8BIT_1LINE);
        delay_us(100);
        WRITE_LCD_CMD(MODE_8BIT_1LINE);
        WRITE_LCD_CMD(MODE_8BIT_2LINE);
        WRITE_LCD_CMD(DISP_ON_CUR_OFF);
        WRITE_LCD_CMD(CLEAR_LCD);
        WRITE_LCD_CMD(SHIFT_CUR_RIGHT);
}

void WRITE_LCD_DATA(u8 ascii)
{
        SCLRBIT(IOCLR0,LCD_RW);//select data register(rs=1)
        SSETBIT(IOSET0,LCD_RS);
        //write data on to the data pins
        WRITEBYTE(IOPIN0,LCD_DATA,ascii);
        //apply H to L pulse on EN
        SSETBIT(IOSET0,LCD_EN); //EN=1
        delay_us(1);
        SCLRBIT(IOCLR0,LCD_EN); //EN=0
        //delay for internal process
        delay_ms(2);
}

void strlcd(s8* str)

{

        while(*str)

        {
              WRITE_LCD_DATA(*str++);
        }
}

void U32LCD(u32 n)
{
        u8 a[10];
        s32 i=0;
        if(n==0)
        {
                WRITE_LCD_DATA('0');
                              }
        else
                while(n)
                {
                        a[i++]=n%10+48;
                        n/=10;
                }
                for(--i; i>=0; i--)
                {
                        WRITE_LCD_DATA(a[i]);
                }
}

void S32LCD(s32 n)
{
        if(n<0)
        {
                WRITE_LCD_DATA('-');
                n=-n;
        }
        U32LCD(n);}

void F32LCD(f32 fn, u8 nDP)
{
        u32 iNum,i;
        if(fn<0)
        {
                WRITE_LCD_DATA('-');
                fn=-fn;
        }
        iNum=fn;
        U32LCD(iNum);
        WRITE_LCD_DATA('.');
        for(i=0; i<nDP; i++)
        {
                fn=(fn-iNum)*10;
                iNum=fn;
                WRITE_LCD_DATA(iNum+'0');
        }
}

void lcd_clear(void)
{
WRITE_LCD_CMD(CLEAR_LCD);
}

//kpm
u8 kpmLUT[4][4]={{'1','2','3','/'},
                 {'4','5','6','*'},
                 {'7','8','9','-'},
                 {'C','0','=','+'}};
void InitKPM(void)
{
        PINSEL2&=~(1<<3);
        IODIR1|=15<<ROW0;
        IODIR1&=~(15<<COL0);
        IOCLR1=(15<<ROW0);
}

u32 ColScan(void)
{
        if(((IOPIN1>>COL0)&15)<15)
                return 0;
        else
                return 1;
}

u32 RowCheck(void)
{
        u32 rno;
        for(rno=0; rno<4; rno++)
        {
                IOPIN1=(IOPIN1&~(15<<ROW0))|((~(1<<rno)&15)<<ROW0);
                if(ColScan()==0)
                break;
        }
        IOCLR1=(15<<ROW0);
        return rno;
}

u32 ColCheck(void)
  {
        u32 cno;
        for(cno=0; cno<4; cno++)
        {  if(((IOPIN1>>(COL0+cno))&1)==0)
                        break;
        }
                    return cno;
}


u32 key_scan(void)
{
    u32 rno, cno, key;
    while(ColScan());
    rno = RowCheck();
    cno = ColCheck();
    key = kpmLUT[rno][cno];
    while(!ColScan()); // Simply wait for key release safely
    delay_ms(20);
    return key;
}


unsigned int input_numeric_value(const char *prompt, unsigned char max_len)
{
    char key; unsigned char index = 0;
    char buffer[8] = {0};
    unsigned int val = 0;
    lcd_clear();
    strlcd((s8*)prompt);
    WRITE_LCD_CMD(GOTO_LINE2_POS0);
    while (index < max_len)
    {
        key = key_scan();
        if (key == '\0') return 0;
        if (key >= '0' && key <= '9')
        {
            buffer[index++] = key;
            WRITE_LCD_DATA(key);
        }
        else if (key == '=')
        {
        // Mapping '=' as enter key
        break;
        }
    }
    buffer[index] = '\0';
    sscanf(buffer, "%u", &val);
    //val=(u32)atoi(buffer);
    return val;
}

//rtc
#define FOSC 12000000
#define CCLK (5*FOSC)
#define PCLK (CCLK/4)

#define PREINT_VAL  ((int)(PCLK/32768)-1)
#define PREFRAC_VAL (PCLK-((PREINT_VAL+1)*32768))

#define RTC_ENABLE (1<<0)
#define RTC_RESET (1<<1)

#define RTC_CLKSRC (1<<4)

#define CPU_LPC2129

s32 hour,min,sec,date,month,year,day;
char week[][4] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};

#define SUN 0
#define MON 1
#define TUE 2
#define WED 3
#define THU 4
#define FRI 5
#define SAT 6

void RTC_Init(void)
{
  // Disable and reset the RTC
  CCR = RTC_RESET;
  #ifndef CPU_LPC2148
  // Set prescaler integer and fractional parts
  PREINT = PREINT_VAL;
  PREFRAC = PREFRAC_VAL;

  // Enable the RTC
                                                         CCR = RTC_ENABLE;  //LPC_2129
  #else
  // Enable the RTC with external clock source
  CCR = RTC_ENABLE | RTC_CLKSRC;  //LPC_2148
  #endif
}

void SetRTCTimeInfo(u32 hour, u32 minute, u32 second)
{
        HOUR = hour;
        MIN = minute;
        SEC = second;
}

void GetRTCTimeInfo(u32 *hour, u32 *minute, u32 *second)
{
        *hour = HOUR;
        *minute = MIN;
        *second = SEC;
}

void DisplayRTCTime(u32 hour, u32 minute, u32 second)
{
        WRITE_LCD_CMD(GOTO_LINE1_POS0);
        WRITE_LCD_DATA(hour/10+48);
        WRITE_LCD_DATA(hour%10+48);
        WRITE_LCD_DATA(':');
        WRITE_LCD_DATA(minute/10+48);
        WRITE_LCD_DATA(minute%10+48);
        WRITE_LCD_DATA(':');
        WRITE_LCD_DATA(second/10+48);
        WRITE_LCD_DATA(second%10+48);
}

void SetRTCDateInfo(u32 date, u32 month, u32 year)
{
        DOM = date;
        MONTH = month;
        YEAR = year;
}

void GetRTCDateInfo(u32 *date, u32 *month, u32 *year)
{
        *date = DOM;
        *month = MONTH;
        *year = YEAR;
}

void DisplayRTCDate(u32 date, u32 month, u32 year)
{
        WRITE_LCD_CMD(GOTO_LINE2_POS0);
        WRITE_LCD_DATA(date/10+48);
        WRITE_LCD_DATA(date%10+48);
        WRITE_LCD_DATA('/');
        WRITE_LCD_DATA(month/10+48);
        WRITE_LCD_DATA(month%10+48);
        WRITE_LCD_DATA('/');
        U32LCD(year);
}

void SetRTCDayInfo(u32 dow)
{
        DOW = dow;
}

void GetRTCDayInfo(u32 *dow)
{
        *dow = DOW;
}

void DisplayRTCDay(u32 day)
{
        WRITE_LCD_CMD(GOTO_LINE1_POS0 + 10);
        if(day<7)
        strlcd((s8*)(week[day]));
}


//eint1
void eint0_isr(void)__irq
{
 menu_req_flag=true;
EXTINT=(1<<0);
VICVectAddr=0;
}

void eint1=0_enable(void)     
{
    // Configure P0.2 to operate under its alternative function as EINT1 (Bits 5:4 = 10)
    PINSEL0 &=  ~(3 << 2);
        PINSEL0 |= (3 << 2);

    VICIntSelect &= ~(1 << EINT0_CHNO);          // Configure EINT1 as an IRQ source
    VICIntEnable = 1 << EINT0_CHNO;              // Enable EINT1 channels inside VIC
    VICVectAddr0 = (u32)eint0_isr;               // Map ISR to Slot 0 vector address registers
    VICVectCntl0 = (1 << 5) | EINT0_CHNO;        // Enable Slot 0 and link it to Channel 15
    EXTMODE |= (1 << 0);                         // Configure EINT1 to trigger on Edge transitions
    EXTPOLAR &= ~(1 << 0);                       // Configure EINT1 to trigger on Falling Edge transitions
}


bool is_leap_year(unsigned int year)
{
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

bool validate_date(u8 d, u8 m, u8 y)
 {
    unsigned char days_in_month[] ={0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (m < 1 || m > 12 || d < 1 || y < 2000 || y > 2099)
         return true;
    if (m == 2 && is_leap_year(y)) days_in_month[2] = 29;
         return (d <= days_in_month[m]);
 }


void execution_menu_loop(void)
 {
    char main_choice;
    char rtc_choice;
    char sched_choice;
    u32 h, m, s, day, mon, yr, wk_day;

    // Clear screen and draw the high-level Root Menu Directory
    lcd_clear();
    strlcd("1:EdRTC 2:Sched");
    WRITE_LCD_CMD(0xC0); // Move cursor to Line 2
    strlcd("3:Exit");

    main_choice = key_scan();


    // MAIN CHOICE 1: SUB-MENU DRIVEN RTC EDITING
    if (main_choice == '1')
    {
    bool rtc_menu_active = true;

    while (rtc_menu_active)
    {
            // Fetch current metrics first so unmodified parameters remain intact
            GetRTCTimeInfo((u32*)&h,(u32*) &m, (u32*)&s);
            GetRTCDateInfo((u32*)&day, (u32*)&mon, (u32*)&yr);
            GetRTCDayInfo((u32*)&wk_day);

            // Display all options flat across both LCD lines simultaneously
            lcd_clear();
            strlcd((u8*)"1:H 2:M 3:S 4:Dt");
            WRITE_LCD_CMD(GOTO_LINE2_POS0);
            strlcd((u8*)"5:Mo 6:Y 7:D 8:Ex");

            rtc_choice = key_scan();

            // Evaluate sub-menu routing selection paths
            switch (rtc_choice)
            {
                case '1':
                {
                    unsigned int new_hr = input_numeric_value("Enter Hour(0-23)", 2);
                    if (new_hr < 24)
                    { SetRTCTimeInfo(new_hr, m, s);
                      lcd_clear();
                      strlcd((u8*)"Hour Updated!");
                    }
                    else
                   {
                    lcd_clear();
                    strlcd((u8*)"Invalid Hour!");
                   }
                    delay_ms(1500);
                    break;
                                                               }
                case '2':
                {
                    unsigned int new_mn = input_numeric_value("Enter Min(0-59)", 2);
                    if (new_mn < 60)
                    {
                    SetRTCTimeInfo(h, new_mn, s);
                    lcd_clear();
                    strlcd((u8*)"Min Updated!");
                    }
                    else
                    {
                    lcd_clear();
                    strlcd((u8*)"Invalid Minute!");
                    }
                    delay_ms(1500);
                    break;
                }
                case '3':
                {
                    unsigned int new_sc = input_numeric_value("Enter Sec(0-59)", 2);
                    if (new_sc < 60)
                    {
                    SetRTCTimeInfo(h, m, new_sc);
                    lcd_clear();
                    strlcd((u8*)"Sec Updated!");
                    }
                    else
                    {
                    lcd_clear();
                    strlcd((u8*)"Invalid Second!");
                    }
                    delay_ms(1500);
                    break;
                }

               case '4':
               {
                    unsigned int new_dt = input_numeric_value("Enter Date(1-31)", 2);
                    if (validate_date(new_dt, mon, yr))
                    {
                    SetRTCDateInfo(new_dt, mon, yr);
                    lcd_clear();
                    strlcd((u8*)"Date Updated!");
                    }
                    else
                    {
                    lcd_clear();
                    strlcd((u8*)"Invalid Date!");
                    }
                    delay_ms(1500);
                    break;
                }
                case '5':
                {
                    unsigned int new_mo = input_numeric_value("Enter Month(1-12)", 2);
                    if (validate_date(day, new_mo, yr))
                    {
                    SetRTCDateInfo(day, new_mo, yr);
                    lcd_clear();
                    strlcd((u8*)"Month Updated!");
                    }
                    else
                    {
                    lcd_clear();
                    strlcd((u8*)"Invalid Month!");
                    }
                    delay_ms(1500);
                    break;
                }
                case '6':
                {
                    unsigned int new_yr = input_numeric_value("Enter Year(4-dig)", 4);
                    if (validate_date(day, mon, new_yr))
                    {
                    SetRTCDateInfo(day, mon, new_yr);
                    lcd_clear();
                    strlcd((u8*)"Year Updated!");
                    }
                    else
                    {
                    lcd_clear();
                    strlcd((u8*)"Invalid Year!");
                    }
                    delay_ms(1500);
                           break;
                }
              case '7':
              {
                    u32 new_day;
                    lcd_clear();
                    strlcd((u8*)"0:SUN 1:MON 2:TUE");
                    WRITE_LCD_CMD(GOTO_LINE2_POS0);
                    strlcd((u8*)"3:WED..6:SAT");
                    delay_ms(3000);
                    new_day = input_numeric_value("Enter Day (0-6)", 1);
                    if (new_day <= 6)
                    {
                    SetRTCDayInfo(new_day);
                    lcd_clear();
                    strlcd((u8*)"Day Updated!");
                    }
                    else
                    {
                    lcd_clear();
                    strlcd((u8*)"Invalid Day!");
                    }
                    delay_ms(1500);
                    break;
                }
                case '8': default: rtc_menu_active = false; break;
            }
        }
    }

    // MAIN CHOICE 2: SUB-MENU DRIVEN SCHEDULER CONTROLLER

    else if (main_choice == '2')
    {
        bool sched_menu_active = true;
        while (sched_menu_active)
       {
            lcd_clear();
            strlcd((u8*)"1:EditON 2:EditOFF");
            WRITE_LCD_CMD(0xC0);
            strlcd((u8*)"3:Exit");
            sched_choice = key_scan()
            switch (sched_choice)
            {
                case '1':
                {
                    unsigned int on_hr = input_numeric_value("ON Hour (0-23)", 2);
                    unsigned int on_mn = input_numeric_value("ON Min (0-59)", 2);
                    if (on_hr < 24 && on_mn < 60)
                    {
                        if (on_hr == off_time.hour && on_mn == off_time.min)
                        {
                            lcd_clear();
                            strlcd((s8*)"Identical Rejected");
                        }
                        else
                        {
                            on_time.hour = on_hr;
                            on_time.min = on_mn;
                            on_time.sec = 0;
                            schedule_val = true;
                            lcd_clear();
                            strlcd((s8*)"ON Time Saved!");
                        }
                    }
                   else
                   {
                        lcd_clear();
                        strlcd((s8*)"Invalid Data");
                    }
                    delay_ms(1500);
                    break;
                }

                case '2':
                {
                    unsigned int off_hr = input_numeric_value("OFF Hour (0-23)", 2);
                    unsigned int off_mn = input_numeric_value("OFF Min (0-59)", 2);

                    if (off_hr < 24 && off_mn < 60)
                    {
                        if (on_time.hour == off_hr && on_time.min == off_mn)
                        {
                            lcd_clear();
                            strlcd((s8*)"Identical Rejected");
                                                                                               }
                        else
                        {
                            off_time.hour = off_hr;
                            off_time.min = off_mn;
                            off_time.sec = 0;
                            schedule_val = true;
                            lcd_clear();
                            strlcd((s8*)"OFF Time Saved!");
                        }
                    }
                    else
                    {
                        lcd_clear();
                        strlcd((s8*)"Invalid Data");
                    }
                    delay_ms(1500);
                    break;
                }


                case '3': // 3: Exit back out to primary root menu screen directory
                default:
                    sched_menu_active = false;
                    break;
            }
        }
    }

    // Reset background breakout flag completely upon menu exit context completion
    menu_req_flag = false;
}

//check_and_control_device
void check_cntrl_LED(void)
{
    u32 cur_h, cur_m, cur_s;
    unsigned long cur_total_min, on_total_min, off_total_min;

    // Exit immediately if there is no valid active schedule available
    if (!schedule_val) return;

    // Retrieve the current time parameters directly from the Real-Time Clock hardware registers
    GetRTCTimeInfo((u32*)&cur_h, (u32*)&cur_m, (u32*)&cur_s);

    // Convert all time matrices into total elapsed minutes from 00:00 to prevent
    // second-overflow race conditions that could glitch the LED operational states
    cur_total_min = (cur_h * 60UL) + cur_m;
    on_total_min  = (on_time.hour * 60UL) + on_time.min;
    off_total_min = (off_time.hour * 60UL) + off_time.min;

    // Evaluate scheduled operational window bounds
    if (on_total_min < off_total_min)
    {
        // [Path A] Daytime Schedule Handling (e.g., ON at 09:00 and OFF at 17:00)
        // Checks if the current time falls inside the target interval [ON Time <= Current Time < OFF Time]
        if (cur_total_min >= on_total_min && cur_total_min < off_total_min)
        {
        IOSET0 = (1 << LED_PIN); // Assert pin output high to switch the LED ON
        }
        else
        {
            IOCLR0 = (1 << LED_PIN); // Assert pin output low to switch the LED OFF
        }
    }
    else
    {
        // [Path B] Overnight Cross-Midnight Schedule Handling (e.g., ON at 22:00 and OFF at 06:00)
        // Checks if current time is past start hour or before the morning cutoff execution threshold
        if (cur_total_min >= on_total_min || cur_total_min < off_total_min)
        {
            IOSET0 = (1 << LED_PIN); // Assert pin output high to switch the LED ON
        }
        else
        {
            IOCLR0 = (1 << LED_PIN); // Assert pin output low to switch the LED OFF
        }
    }
}
