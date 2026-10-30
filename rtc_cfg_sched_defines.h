//defines.h
#define LCD_DATA 8 //p0.8 to p0.15
#define LCD_RS 16  //P0.16
#define LCD_RW 18  //P0.18
#define LCD_EN 17 //PO.17

#define ROW0 16 //p1.16
#define ROW1 17 //p1.17
#define ROW2 18 //p1.18
#define ROW3 19 //p1.19

#define COL0 20 //p1.20
#define COL1 21 //p1.21
#define COL2 22 //p1.22
#define COL3 23 //p1.23

#define LED_PIN 19//p0.19
#define SWITCH_PIN 1

#define EINT0_CHNO 14

#define CLEAR_LCD 0x01
#define RET_CUR_HOME 0x02
#define MODE_8BIT_1LINE 0x30
#define MODE_8BIT_2LINE 0x38
#define MODE_4BIT_1LINE 0x20
#define MODE_4BIT_2LINE 0x28
#define DISP_OFF 0x08
#define DISP_ON_CUR_OFF 0x0C
#define DISP_ON_CUR_ON 0x0E
#define DISP_ON_CUR_BLINK 0x0F
#define SHIFT_CUR_RIGHT 0x06
#define GOTO_LINE1_POS0 0x80
#define GOTO_LINE2_POS0 0xC0
#define GOTO_LINE3_POS0 0x94
#define GOTO_LINE4_POS0 0xD4
#define GOTO_CGRAM 0x40

