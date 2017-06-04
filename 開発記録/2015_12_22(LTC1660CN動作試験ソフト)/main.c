//�����̐U���X�s�[�J�̃v���O����3
///// LTC1660CN DAC /////

#include <16f873a.h>
#include "vibtable2.h"

#fuses HS,NOWDT,NOPROTECT,PUT,BROWNOUT,NOLVP
//#fuses XT,NOWDT,NOPROTECT,PUT,BROWNOUT,NOLVP
#use delay(CLOCK = 20000000)
//RS232C�g�p�錾
#use rs232(BAUD = 9600, XMIT = PIN_C6, RCV = PIN_C7)
//#use rs232(BAUD = 19200, XMIT = PIN_C6, RCV = PIN_C7)

#use fast_io(c)

//#define CODE_TIME 50 // timer1 50msec
//#define CODE_TIME 5 // timer1 80msec
#define CODE_TIME 10 // timer1 160msec

//dac �v���g�^�C�v�錾
void dac_511(void);						//dac 511 write
void scale_dac(int chadrs, unsigned char sftdat);	//dac change adres, shift data
void write_dac(int chip, int wdata, int rcount);	//spi output 
///// dac�O���[�o���ϐ��A�萔�錾
//#define MAX_DEV_CS1 0x07					//LTC1660CN cs1 max device ch
//#define MAX_DEV_CS2 0x04					//LTC1660CN cs2 max device ch
#define Maxch 0x0a							//LTC1660 CS1/CS2 max channel 
static int	Chipadrs;						//LTC1660CN chip address(add+udata 4bit)
static int Address;							//LTC1660CN slave address
static int	Chipdat;						//LTC1660CN chip data(ldata 5bit)
static char	Chiproop_sel;					//spi chiproop select
///// SPI MODE �ݒ�
// SPI Mode | MICROCHIP | CCS                          | Data clocked in at 
//----------------------------------------------------------------|------------------- 
//          |  CKP CKE  |                              | 
//    0     |   0   1   | SPI_L_TO_H | SPI_XMIT_L_TO_H | low to high edge 
//    1     |   0   0   | SPI_L_TO_H                   | high to low edge 
//    2     |   1   1   | SPI_H_TO_L                   | high to low edge 
//    3     |   1   0   | SPI_H_TO_L | SPI_XMIT_L_TO_H | low to high edge
#define SPI_MODE_0  (SPI_L_TO_H | SPI_XMIT_L_TO_H) 
#define SPI_MODE_1  (SPI_L_TO_H) 
#define SPI_MODE_2  (SPI_H_TO_L) 
#define SPI_MODE_3  (SPI_H_TO_L | SPI_XMIT_L_TO_H) 

static int16 big_data;									//16bit big data
static int8 data_u;									//8bit upper data
static int8 data_l;									//8bit lower data
int data_in;									//spi input dfata(dummy)
/* pwm �O���[�o���ϐ���` */
static unsigned char pwmData;
static unsigned char rowcount = 0;
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

void setpulse(int);
void set_dtmf(int);

//RS232C�����ݒ�
char icmd[24]; // �V���A������ǂ񂾃f�[�^�̊i�[�o�b�t�@
int n_cmd=0; // �o�b�t�@���̕�����

#int_rda
void isr_rcv()
{
	char data;
	data=getc();
	do {
		if(n_cmd < 20) { // �ő�30���ɐ���
			icmd[n_cmd]=data;
			n_cmd++;
		}
		data=getc(); 
	} while(data != 'e'); 
	icmd[n_cmd]='e'; //�Ō��e��ǉ�
	n_cmd++;
//	cmd[n_cmd]=getc(); // �`�F�b�N�T���̓ǂݍ��݁@2014_03_17 ishii
//    n_cmd++; // 2014_03_17 ishii
}

//PIC���o�͏����ݒ�
void pic873init(char pa,char pb,char pc)
{
	set_tris_a(pa);
	set_tris_b(pb);
	set_tris_c(pc);
}

void all_reset(void)
{
	if(!output_flag)			// KEY���͖���
	{
		disable_interrupts(INT_TIMER0);
		disable_interrupts(INT_TIMER1);
		pwmData = kEndDataMark;
//		set_pwm1_duty(pwmData);
		scale_dac(Address, pwmData);			//127����������
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
//	set_pwm1_duty(pwmData);
	scale_dac(Address, pwmData);			//127����������
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
//			set_pwm1_duty(pwmData);
			scale_dac(Address, pwmData);			//pwmData����������
			
//			setpulse(50); // use deback
		}
		if(timer1_count > CODE_TIME*2 && state == 1)
		{
			timer1_count = 0;
			if(cnt < num_push) {
				state = 0;
			} else if(cnt == num_push && !option_flag) {
				state = 2;
			}
			if(option_flag)
				play_off_reset();
		}
//		if(timer1_count < CODE_TIME && state == 2)
//			enable_interrupts(INT_TIMER0);
		if(timer1_count > CODE_TIME && state == 2)
			play_off_reset();
	}
	timer1_count++;
}

#int_RTCC				//timer0�I�[�o�[�t���[���荞��
void interrupt_0(void)	//524.8us���ƂɊ��荞�ݔ���
{
	unsigned char row;
	long int pwm;
		
	disable_interrupts(INT_TIMER0);		//���荞�݋֎~

//	set_timer0(174);					
	set_timer0(92);					
//	set_pwm1_duty(pwmData);				//�f���[�e�B�[�l�Z�b�g
	scale_dac(Address, pwmData);			//pwmData����������

//	setpulse(50); // use deback	

	row = rowbase[rowcount++];
	if(row == kEndDataMark) {
		rowcount = 0;
		row = rowbase[rowcount++];
	}
		

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


void set_dtmf(int fl)
{
	int i;
	
	switch(fl)
	{
		case 1:						//40Hz
			for(i = 0;i < 40;i++)
				dataL_c[i] = sinrow1[i];
			break;
		case 2:						//50Hz
			for(i = 0;i < 33;i++)
				dataL_c[i] = sinrow2[i];
				break;
		case 3:						//60Hz
			for(i = 0;i < 28;i++)
				dataL_c[i] = sinrow3[i];
			break;
		case 4:						//70Hz
			for(i = 0;i < 24;i++)		
				dataL_c[i] = sinrow4[i];
			break;	
		case 5:						//80Hz
			for(i = 0;i < 21;i++)
				dataL_c[i] = sinrow5[i];
			break;
		case 6:						//90Hz
			for(i = 0;i < 19;i++)
				dataL_c[i] = sinrow6[i];
			break;
		case 7:						//100Hz
			for(i = 0;i < 17;i++)
				dataL_c[i] = sinrow7[i];
			break;
		case 8:						//110Hz
			for(i = 0;i < 16;i++)
				dataL_c[i] = sinrow8[i];
			break;
		default:
			for(i = 0;i < 40;i++)
				dataL_c[i] = 0;
			break;
	}
	rowbase = dataL_c;
}

//    check pulse port pb4, use deback
void setpulse(int i)
{	
	output_bit(PIN_B4,1);		//pb4 on
	delay_us(i);				//i usec delay	
	output_bit(PIN_B4,0);		//pb4 off
}	

int main( void )
{	
//SPI���[�h�ݒ�
	setup_spi(SPI_MASTER | SPI_MODE_0 | SPI_CLK_DIV_16 | SPI_SS_DISABLED); //spi clk 1.25MHz
//	setup_spi(SPI_MASTER | SPI_MODE_0 | SPI_CLK_DIV_4 | SPI_SS_DISABLED); //spi clk 5MHz
//PIO�����ݒ�
//	pic873init(0xff,0x7f,0xa3);						//PORTB����
//	pic873init(0x3f,0xcf,0xa3);				//2014_03_06 ishii
	pic873init(0x3f,0xe2,0x93);	
///	dac1 spi dummy initial ///	
	output_low(PIN_B2);							//CS1 low�o��
	data_in = spi_read();						//dac dummy read
	output_high(PIN_B2);						//CS1 High�o��
/// dac2 spi dummy initial ///	
	output_low(PIN_B3);							//CS2 High�o��
	data_in = spi_read();						//dac dummy read
	output_high(PIN_B3);						//CS2 High�o��	
	//���0�����ݒ�
//	setup_timer_0(RTCC_INTERNAL | RTCC_DIV_2);		
	setup_timer_0(RTCC_INTERNAL | RTCC_DIV_16);		

	//���1�����ݒ�
//	setup_timer_1(T1_INTERNAL | T1_DIV_BY_1);		//1ms���
	setup_timer_1(T1_INTERNAL | T1_DIV_BY_8);		//16ms���
	
	//CCP1�̏����ݒ�
//	setup_ccp1(CCP_PWM);					//pwm���[�h
	setup_timer_2(T2_DIV_BY_1,0xff,1);		//���2�̓t���J�E���g�A51.2u����
//	setup_timer_2(T2_DIV_BY_16,0xff,1);		// 819.2us����

	//DAC�̏����ݒ�
//	pwmData = kEndDataMark;					//����PWM�f�[�^��0V
//	set_pwm1_duty(pwmData);					//�f���[�e�B��Z�b�g
	dac_511();								//ch1����ch9��511����������	
//	set_timer0(174);						//82�J�E���g��32.8us���ƂɊ��荞�ݔ���
	set_timer0(92);						//2624�J�E���g��524.8us���ƂɊ��荞�ݔ���
//	set_timer1(60536);	//1ms���
	set_timer1(55536);  //16ms���

//RS232C�������b�Z�[�W
//	printf("START\n\r");  //START���b�Z�[�W
// �R�}���h���͂ƃ`�F�b�N
	printf("\r\nDAC ch=1-9 ?");					//�������b�Z�[�W
	int j;
	enable_interrupts(INT_RDA); //�V���A�����荞�݋���
	enable_interrupts(GLOBAL); //���荞�݋��� 
	
	while(1)
	{		
		KeyScan();//KEY����
	
		////�R�}���h���
		if(n_cmd!=0) {
			for(j=0;j<n_cmd;j++) {
				;
				printf("%c",icmd[j]); //�G�R�[�o��
//				printf("\n\r"); //LF,CR���o��
			}
			
			if((icmd[0] > '9') || (icmd[0] < '1')){
			printf("  Command Error?\r\n");			//�R�}���h�G���[	
			n_cmd=0;
			}
			else
//			printf("\r\n");
			switch (icmd[0])						//�R�}���h�������s
			{
				case '1': Address = 1; break;	//dac ch1 set
				case '2': Address = 2; break;	//dac ch2 set
				case '3': Address = 3; break;	//dac ch3 set
				case '4': Address = 4; break;	//dac ch4 set
				case '5': Address = 5; break;	//dac ch5 set
				case '6': Address = 6; break;	//dac ch6 set
				case '7': Address = 7; break;	//dac ch7 set
				case '8': Address = 8; break;	//dac ch8 set
				case '9': Address = 9; break;	//dac ch8 set
				default : break;
			}
			printf("\r\nDAC ch=1-9 ?");	
			n_cmd=0;
		}	
	}
	return 0;
}

/*************************************
* dac 511 �����֐��ich0�`ch9)
*************************************/
void dac_511(void)
{
//	big_data = 0x1ff;
	big_data = 0x1fc;		// 127 -> 2bit shift= 508
	data_u = (big_data >> 6)&0x0f;
	data_l = (big_data << 2)&0xfc;
//	printf("\r\ndac=512 set ");
	for (Address=1; Address<Maxch; Address++)	//�A�h���X�͈̓`�F�b�N
	{
		if (Address <= 6){
			Chiproop_sel = PIN_B2;	//CS1=B2 select
		}
		else{
			Chiproop_sel = PIN_B3;	//CS2=B3 select
		}
		Chipadrs = ((Address<<4)&0xf0) | data_u;
		Chipdat = data_l;
		write_dac(Chipadrs, Chipdat, Chiproop_sel);	//511����������
	}
//	printf("\r\nComplete!\r\n");					//�������b�Z�[�W
}
/*************************************
* dac adres wdata �ϊ�
*************************************/
void scale_dac(int chadrs, unsigned char sftdat)
{
//	big_data = 0x1ff;
	data_u = (sftdat >> 4)&0x0f;
	data_l = (sftdat << 2)&0xfc;
	if (chadrs <= 6){
		Chiproop_sel = PIN_B2;	//CS1=B2 select
		Chipadrs = ((chadrs<<4)&0xf0) | data_u;			
	}
	else{
		Chiproop_sel = PIN_B3;	//CS2=B3 select
		Chipadrs = (((chadrs-6)<<4)&0xf0) | data_u;
	}
	Chipdat = data_l;
	write_dac(Chipadrs, Chipdat, Chiproop_sel);	//DAC�Ƀf�[�^����������
//	printf("\r\nComplete!\r\n");					//�������b�Z�[�W
}
/*************************************
*  LTC1660CN DAC �����݃T�u�֐�
*************************************/
void write_dac(int chip, int wdata, int rcount)
{

	output_low(rcount);				//CS1/CS2 Low�o��
//	delay_us(1);					//1us �������ݑ҂�
	spi_write(chip);					//DAC ch�A�h���X+���4bit�f�[�^���M
	spi_write(wdata);				//�������݃f�[�^���M
//	delay_us(1);					//1us �������ݑ҂�
	output_high(rcount);			//CS1/CS2 High�o��
//	delay_ms(1);						//1ms �������ݑ҂�
}

