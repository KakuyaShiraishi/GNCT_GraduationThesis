//�����̐U���X�s�[�J�̃v���O����

#include <16f873a.h>
//#include "dtmftable.h"
//#include "vibtable.h"
#include "vibtable2.h"

#fuses HS,NOWDT,NOPROTECT,PUT,BROWNOUT,NOLVP
//#fuses XT,NOWDT,NOPROTECT,PUT,BROWNOUT,NOLVP
#use delay(CLOCK = 20000000)
//#use delay(CLOCK = 1250000)
#use rs232(BAUD = 9600, XMIT = PIN_C6, RCV = PIN_C7)
#use fast_io(c)

//#define CODE_TIME 50 // timer1 50msec
#define CODE_TIME 5 // timer1 80msec

/* �O���[�o���ϐ���` */
static unsigned char pwmData;
static unsigned char rowcount = 0;
//static unsigned char columcount = 0;
long int timer1_count = 0;
int output_flag = 0;					//�M���o�̓t���O 			
int leave_flag = 0;						//�{�^���̉����󋵃t���O
int option_flag = 0;					// S7 ON���P

static int cnt = 0;
static unsigned char msk = 0x01,Num = 0;
static int state = 0;

unsigned char local_pattern = 0;		// KEY����
unsigned char Pattern = 0;				// KEY���͏d��
unsigned char num_push = 0;				// ���͍��v��

unsigned char calc_num_push(void);
//void Dtmf(void);
//void set_dtmf(int, int);
void set_dtmf(int);

//PIC���o�͏����ݒ�
void pic873init(char pa,char pb,char pc)
{
	set_tris_a(pa);
	set_tris_b(pb);
	set_tris_c(pc);
//	port_b_pullups(TRUE);
//	output_a(0);
//	output_b(0);
//	output_c(0);
}

void all_reset(void)
{
	if(!output_flag)			// KEY���͖���
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

//1ms���
//16ms���
#int_timer1
void interrupt_1(void)				
{		
//	set_timer1(60536); //1ms���
	set_timer1(55536); //16ms���

	if(output_flag)
	{
		if(state == 0)
		{
			if(cnt < num_push && Num < 8)
			{
				if(Pattern & msk)
				{
//					set_dtmf(fH[Num],fL[Num]);
					set_dtmf(fL[Num]);
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
//				set_dtmf(2,4);
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

#int_RTCC				//timer0�I�[�o�[�t���[���荞��
void interrupt_0(void)	//524.8us���ƂɊ��荞�ݔ���
{
//	unsigned char row,colum;
	unsigned char row;
	long int pwm;
		
	disable_interrupts(INT_TIMER0);		//���荞�݋֎~

//	set_timer0(174);					
	set_timer0(92);					
	set_pwm1_duty(pwmData);				//�f���[�e�B�[�l�Z�b�g
	
	row = rowbase[rowcount++];
	if(row == kEndDataMark) {
		rowcount = 0;
		row = rowbase[rowcount++];
	}
		
//	colum = columbase[columcount++];
//	if(colum == kEndDataMark) {
//		columcount = 0;
//		colum = columbase[columcount++];
//	}
	
//	pwm = (long int)row + (long int)colum;
	pwm = (long int)row;
	pwmData = (unsigned char)(pwm >> 1);
		
	enable_interrupts(INT_TIMER0);
}

void KeyScan(void)
{	
	//�e���͂��`�F�b�N
	if(!(input(PIN_C0))) {
		local_pattern |= 0x01;		// S1�`�F�b�N
	}
	if(!(input(PIN_C1))) {
		local_pattern |= (0x01<<1);		// S2�`�F�b�N
	}
	if(!(input(PIN_A0))) {
		local_pattern |= (0x01<<2);		// S3�`�F�b�N
	}
	if(!(input(PIN_A1))) {
		local_pattern |= (0x01<<3);		// S4�`�F�b�N
	}
	if(!(input(PIN_A2))) {
		local_pattern |= (0x01<<4);		// S5�`�F�b�N
	}
	if(!(input(PIN_A3))) {
		local_pattern |= (0x01<<5);		// S6�`�F�b�N
	}
	if(!(input(PIN_A4))) {
		local_pattern |= (0x01<<6);		// S7�`�F�b�N
//		option_flag = 1;
	}
	if(!(input(PIN_A5))) {
		local_pattern |= (0x01<<7);		// S8�`�F�b�N
	}	
	 //�L�[���͖�����all_reset()
	if(input(PIN_C0) && input(PIN_C1) && input(PIN_A0)
	   && input(PIN_A1) && input(PIN_A2) && input(PIN_A3) 
		&& input(PIN_A4) && input(PIN_A5))
			all_reset();
	
	//�L�[���͂���
	if(local_pattern != 0)
	{
		enable_interrupts(INT_TIMER1);
		enable_interrupts(GLOBAL);
//		if((timer1_count > 200) && !output_flag && !leave_flag) // 200ms wait
		if((timer1_count > 12) && !output_flag && !leave_flag)  // 192ms wait
		{
			Pattern = local_pattern;
			num_push = calc_num_push();
			output_flag = 1;
			timer1_count = 0;
//			set_timer1(60536); //1ms���
			set_timer1(55536); //16ms���
		}
	}
}

//Pattern�̏d�݂����߂�
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

/*void set_dtmf(int fh,int fl)
{
	int i;

	switch(fh)
	{
		case 1:
//			for(i = 0;i < 21;i++)
			for(i = 0;i < 20;i++)
				dataH_c[i] = sincoluma[i];
			break;
		case 2:
//			for(i = 0;i < 19;i++)
			for(i = 0;i < 18;i++)
				dataH_c[i] = sincolumb[i];
			break;
		case 3:
//			for(i = 0;i < 52;i++)
			for(i = 0;i < 16;i++)
				dataH_c[i] = sincolumc[i];
			break;
		default:
//			for(i = 0;i < 52;i++)
			for(i = 0;i < 20;i++)
				dataH_c[i] = 0;
			break;
	}
	columbase = dataH_c;
	
	switch(fl)
	{
		case 1:
//			for(i = 0;i < 37;i++)
			for(i = 0;i < 40;i++)
				dataL_c[i] = sinrow1[i];
			break;
		case 2:
//			for(i = 0;i < 33;i++)
			for(i = 0;i < 32;i++)
				dataL_c[i] = sinrow2[i];
				break;
		case 3:
//			for(i = 0;i < 30;i++)
			for(i = 0;i < 27;i++)
				dataL_c[i] = sinrow3[i];
			break;
		case 4:
//			for(i = 0;i < 27;i++)
			for(i = 0;i < 23;i++)		
				dataL_c[i] = sinrow4[i];
			break;	
		default:
//			for(i = 0;i < 37;i++)
			for(i = 0;i < 40;i++)
				dataL_c[i] = 0;
			break;
	}
	rowbase = dataL_c;
}
*/

void set_dtmf(int fl)
{
	int i;
	
	switch(fl)
	{
		case 1:						//40Hz
//			for(i = 0;i < 37;i++)
			for(i = 0;i < 40;i++)
				dataL_c[i] = sinrow1[i];
			break;
		case 2:						//50Hz
			for(i = 0;i < 33;i++)
//			for(i = 0;i < 32;i++)
				dataL_c[i] = sinrow2[i];
				break;
		case 3:						//60Hz
			for(i = 0;i < 28;i++)
//			for(i = 0;i < 27;i++)
				dataL_c[i] = sinrow3[i];
			break;
		case 4:						//70Hz
			for(i = 0;i < 24;i++)
//			for(i = 0;i < 23;i++)		
				dataL_c[i] = sinrow4[i];
			break;	
		case 5:						//80Hz
			for(i = 0;i < 21;i++)
//			for(i = 0;i < 20;i++)
				dataL_c[i] = sinrow5[i];
			break;
		case 6:						//90Hz
			for(i = 0;i < 19;i++)
//			for(i = 0;i < 18;i++)
				dataL_c[i] = sinrow6[i];
			break;
		case 7:						//100Hz
			for(i = 0;i < 17;i++)
//			for(i = 0;i < 16;i++)
				dataL_c[i] = sinrow7[i];
			break;
		case 8:						//110Hz
			for(i = 0;i < 16;i++)
//			for(i = 0;i < 15;i++)
				dataL_c[i] = sinrow8[i];
			break;
		default:
//			for(i = 0;i < 37;i++)
			for(i = 0;i < 40;i++)
				dataL_c[i] = 0;
			break;
	}
	rowbase = dataL_c;
}

int main( void )
{	
//	pic873init(0,1,0);						//PORTB����
//	pic873init(0xff,0x7f,0xa3);						//PORTB����
	pic873init(0x3f,0xcf,0xa3);				//2014_03_06 ishii
	
	//���0�����ݒ�
//	setup_timer_0(RTCC_INTERNAL | RTCC_DIV_2);		
	setup_timer_0(RTCC_INTERNAL | RTCC_DIV_16);		

	//���1�����ݒ�
//	setup_timer_1(T1_INTERNAL | T1_DIV_BY_1);		//1ms���
	setup_timer_1(T1_INTERNAL | T1_DIV_BY_8);		//16ms���
	
	//CCP1�̏����ݒ�
	setup_ccp1(CCP_PWM);					//pwm���[�h
//	setup_timer_2(T2_DIV_BY_1,0xff,1);		//���2�̓t���J�E���g
	setup_timer_2(T2_DIV_BY_16,0xff,1);		// 819.2us����
	
	pwmData = kEndDataMark;					//����PWM�f�[�^��0V
	set_pwm1_duty(pwmData);					//�f���[�e�B��Z�b�g
	
//	set_timer0(174);						//82�J�E���g��32.8us���ƂɊ��荞�ݔ���
	set_timer0(92);						//2624�J�E���g��524.8us���ƂɊ��荞�ݔ���
//	set_timer1(60536);	//1ms���
	set_timer1(55536);  //16ms���

	while(1)
	{		
		KeyScan();

	}
	return 0;
}
	
