//6点点字キーボード型点字入力装置のプログラム

#include <16f873.h>
#include "dtmftable.h"

#fuses HS,NOWDT,NOPROTECT,PUT,BROWNOUT,NOLVP
#use delay(CLOCK = 20000000)
#use fast_io(c)

#define CODE_TIME 50

/* グローバル変数定義 */
static unsigned char pwmData;
static unsigned char rowcount = 0;
static unsigned char columcount = 0;
long int timer1_count = 0;
int output_flag = 0;					//信号出力フラグ 			
int leave_flag = 0;						//ボタンの押下状況フラグ
int option_flag = 0;

static int cnt = 0;
static unsigned char msk = 0x01,Num = 0;
static int state = 0;

unsigned char local_pattern = 0;
unsigned char Pattern = 0;
unsigned char num_push = 0;

unsigned char calc_num_push(void);
void Dtmf(void);
void set_dtmf(int, int);


//PIC入出力初期設定
void pic873init(char pa,char pb,char pc)
{
	set_tris_a(pa);
	set_tris_b(pb);
	set_tris_c(pc);
	port_b_pullups(TRUE);
	output_a(0);
	output_b(0);
	output_c(0);
}

void all_reset(void)
{
	if(!output_flag)
	{
		disable_interrupts(INT_TIMER0);
		disable_interrupts(INT_TIMER1);
		pwmData = kEndDataMark;
		set_pwm1_duty(pwmData);
		timer1_count = 0;
		leave_flag = 0;
		option_flag = 0;
		local_pattern = 0;
		Pattern = 0;
		num_push = 0;
	}
}

void play_off_reset(void)
{
	disable_interrupts(INT_TIMER0);	
	pwmData = kEndDataMark;
	set_pwm1_duty(pwmData);
	output_flag = 0;
	leave_flag = 1;
	option_flag = 0;
	msk = 0x01;
	Num = 0;
	state = 0;
	cnt = 0;
}	

//1msﾀｲﾏ
#int_timer1
void interrupt_1(void)				
{		
	set_timer1(60536);

	if(output_flag)
	{
		if(state == 0)
		{
			if(cnt < num_push && Num < 8)
			{
				if(Pattern & msk)
				{
					set_dtmf(fH[Num],fL[Num]);
					cnt++;
					state = 1;
				}
				msk <<= 1;
				Num++;
			}
		}
		if(timer1_count < CODE_TIME && state == 1)
			enable_interrupts(INT_TIMER0);
		if(timer1_count > CODE_TIME && state == 1)
		{
			disable_interrupts(INT_TIMER0);
			pwmData = kEndDataMark;
			set_pwm1_duty(pwmData);
		}
		if(timer1_count > CODE_TIME*2 && state == 1)
		{
			timer1_count = 0;
			if(cnt < num_push) {
				state = 0;
			} else if(cnt == num_push && !option_flag) {
				state = 2;
				set_dtmf(2,4);
			}
			if(option_flag)
				play_off_reset();
		}
		if(timer1_count < CODE_TIME && state == 2)
			enable_interrupts(INT_TIMER0);
		if(timer1_count > CODE_TIME && state == 2)
			play_off_reset();
	}
	timer1_count++;
}

#int_RTCC				//timer0オーバーフロー割り込み
void interrupt_0(void)	//32.8usごとに割り込み発生
{
	unsigned char row,colum;
	long int pwm;
		
	disable_interrupts(INT_TIMER0);		//割り込み禁止

	set_timer0(174);					
	set_pwm1_duty(pwmData);				//デューティー値セット
	
	row = rowbase[rowcount++];
	if(row == kEndDataMark) {
		rowcount = 0;
		row = rowbase[rowcount++];
	}
		
	colum = columbase[columcount++];
	if(colum == kEndDataMark) {
		columcount = 0;
		colum = columbase[columcount++];
	}
	
	pwm = (long int)row + (long int)colum;
	pwmData = (unsigned char)(pwm >> 1);
		
	enable_interrupts(INT_TIMER0);
}

void KeyScan(void)
{	
	//各入力をチェック
	if(!(input(PIN_B0))) {
		local_pattern |= 0x01;
	}
	if(!(input(PIN_B1))) {
		local_pattern |= (0x01<<1);
	}
	if(!(input(PIN_B2))) {
		local_pattern |= (0x01<<2);
	}
	if(!(input(PIN_B3))) {
		local_pattern |= (0x01<<3);
	}
	if(!(input(PIN_B4))) {
		local_pattern |= (0x01<<4);
	}
	if(!(input(PIN_B5))) {
		local_pattern |= (0x01<<5);
	}
	if(!(input(PIN_B6))) {
		local_pattern |= (0x01<<6);
		option_flag = 1;
	}
	if(!(input(PIN_B7))) {
		local_pattern |= (0x01<<7);
	}	
	if(input(PIN_B0) && input(PIN_B1) && input(PIN_B2)
	   && input(PIN_B3) && input(PIN_B4) && input(PIN_B5) 
		&& input(PIN_B6) && input(PIN_B7))
			all_reset();
	

	if(local_pattern != 0)
	{
		enable_interrupts(INT_TIMER1);
		enable_interrupts(GLOBAL);
		if((timer1_count > 200) && !output_flag && !leave_flag)
		{
			Pattern = local_pattern;
			num_push = calc_num_push();
			output_flag = 1;
			timer1_count = 0;
			set_timer1(60536);
		}
	}
}

//Patternの重みを求める
unsigned char calc_num_push(void)
{
	unsigned char i,n,msk;
	
	n = 0;
	msk = 0x01;
	for(i = 0;i < 8;i++)
	{
		if(Pattern & msk) n++;
		msk <<= 1;
	}
	return n;
}

void set_dtmf(int fh,int fl)
{
	int i;

	switch(fh)
	{
		case 1:
			for(i = 0;i < 21;i++)
				dataH_c[i] = sincoluma[i];
			break;
		case 2:
			for(i = 0;i < 19;i++)
				dataH_c[i] = sincolumb[i];
			break;
		case 3:
			for(i = 0;i < 52;i++)
				dataH_c[i] = sincolumc[i];
			break;
		default:
			for(i = 0;i < 52;i++)
				dataH_c[i] = 0;
			break;
	}
	columbase = dataH_c;
	
	switch(fl)
	{
		case 1:
			for(i = 0;i < 37;i++)
				dataL_c[i] = sinrow1[i];
			break;
		case 2:
			for(i = 0;i < 33;i++)
				dataL_c[i] = sinrow2[i];
				break;
		case 3:
			for(i = 0;i < 30;i++)
				dataL_c[i] = sinrow3[i];
			break;
		case 4:
			for(i = 0;i < 27;i++)
				dataL_c[i] = sinrow4[i];
			break;	
		default:
			for(i = 0;i < 37;i++)
				dataL_c[i] = 0;
			break;
	}
	rowbase = dataL_c;
}



int main( void )
{	

	pic873init(0,1,0);						//PORTB入力
	//ﾀｲﾏ0初期設定
	setup_timer_0(RTCC_INTERNAL | RTCC_DIV_2);		

	//ﾀｲﾏ1初期設定
	setup_timer_1(T1_INTERNAL | T1_DIV_BY_1);		
	
	//CCP1の初期設定
	setup_ccp1(CCP_PWM);					//pwmモード
	setup_timer_2(T2_DIV_BY_1,0xff,1);		//ﾀｲﾏ2はフルカウント
	
	pwmData = kEndDataMark;					//初期PWMデータは0V
	set_pwm1_duty(pwmData);					//デューティ比セット
	
	set_timer0(174);						//82カウントで32.8usごとに割り込み発生
	set_timer1(60536);	
	
	while(1)
	{		
		KeyScan();
		
		

	}
	return 0;
}
	
