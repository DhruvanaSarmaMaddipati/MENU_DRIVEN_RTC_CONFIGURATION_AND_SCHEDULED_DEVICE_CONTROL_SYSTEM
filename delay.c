//delay.c
void delay_us(unsigned int delayus)
{
for(delayus*=12;delayus>0;delayus--);
}
void delay_ms(unsigned int delayms)
{
for(delayms*=12000;delayms>0;delayms--);
}
void delay_s(unsigned int delays)
{
for(delays*=12000000;delays>0;delays--);
}