//6�_�_���L�[�{�[�h�^�_�����͑��u�̃v���O����

#include <16f873a.h>
#include "dtmftable.h"

#fuses HS,NOWDT,NOPROTECT,PUT,BROWNOUT,NOLVP
#use delay(CLOCK = 20000000)
//RS232C�g�p�錾
//#use rs232(BAUD = 9600, XMIT = PIN_C6, RCV = PIN_C7)
#use rs232(BAUD = 19200, XMIT = PIN_C6, RCV = PIN_C7) //2014_03_27
//I2C�g�p�錾
#use i2c(MASTER, SDA=PIN_C4, SCL=PIN_C3, ADDRESS=0xa0, FORCE_HW) /////////////
//#use i2c(MASTER, SDA=PIN_C4, SCL=PIN_C3, ADDRESS=0xa0) /////////////

#use fast_io(c)

#define CODE_TIME 50

//EEPROM �v���g�^�C�v�錾
void clear_mem(void);
void fill_mem(void);
void disp_mem(void);
void write_ext_eeprom(int chip, long adrs, int wdata);
int read_ext_eeprom(int chip, long adrs);
/////�O���[�o���ϐ��A�萔�錾
#define MAX_EEPROM 0x0400     // Maximum 0x20000   ////////////////
static int	Chipadrs;		  //EEPROM�`�b�v�A�h���X
static long Address;		  //EEPROM�A�h���X

/* DTMF �O���[�o���ϐ���` */
static unsigned char pwmData;
static unsigned char rowcount = 0;
static unsigned char columcount = 0;
long int timer1_count = 0;
int output_flag = 0;					//�M���o�̓t���O 			
int leave_flag = 0;						//�{�^���̉����󋵃t���O
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

//RS232C�����ݒ�
//char icmd[64]; // �V���A������ǂ񂾃f�[�^�̊i�[�o�b�t�@ 2014_03_18 ishii make err74
char icmd[24]; // �V���A������ǂ񂾃f�[�^�̊i�[�o�b�t�@
int n_cmd=0; // �o�b�t�@���̕�����

#int_rda
void isr_rcv()
{
	char data;
	data=getc();
	do {
//		if(n_cmd < 60) { // �ő�60���ɐ��� 2014_03_18 ishii
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
//	port_b_pullups(TRUE);
//	output_a(0);
//	output_b(0);
//	output_c(0);
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

//1ms���
#int_timer1
void interrupt_1(void)				
{		
	set_timer1(60536);// 0.001/(0.00000005*4*1(��ؽ���1:1�j)=5000
				 	 // 65535(16bit)-5000+1(0���āj=60536
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

#int_RTCC				//timer0�I�[�o�[�t���[���荞��
void interrupt_0(void)	//32.8us���ƂɊ��荞�ݔ���
{
	unsigned char row,colum;
	long int pwm;
		
	disable_interrupts(INT_TIMER0);		//���荞�݋֎~

	set_timer0(174);//0.0000328/(0.00000005*4*2(��ؽ���2:1�j)=82
				    //256(8bit)-82=174
	set_pwm1_duty(pwmData);				//�f���[�e�B�[�l�Z�b�g
	
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
	//�e���͂��`�F�b�N
	if(!(input(PIN_C0))) {
		local_pattern |= 0x01;
	}
	if(!(input(PIN_C1))) {
		local_pattern |= (0x01<<1);
	}
	if(!(input(PIN_A0))) {
		local_pattern |= (0x01<<2);
	}
	if(!(input(PIN_A1))) {
		local_pattern |= (0x01<<3);
	}
	if(!(input(PIN_A2))) {
		local_pattern |= (0x01<<4);
	}
	if(!(input(PIN_A3))) {
		local_pattern |= (0x01<<5);
	}
	if(!(input(PIN_A4))) {
		local_pattern |= (0x01<<6);
		option_flag = 1;
	}
	if(!(input(PIN_A5))) {
		local_pattern |= (0x01<<7);
	}	
	if(input(PIN_C0) && input(PIN_C1) && input(PIN_A0)
	   && input(PIN_A1) && input(PIN_A2) && input(PIN_A3) 
		&& input(PIN_A4) && input(PIN_A5))
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
//i2C�����ݒ�
	output_float(PIN_C3);   //SCL�s����`  //////////////////
	output_float(PIN_C4);   //SDA�s����`  //////////////////
	
		
//PIO�����ݒ�
//	pic873init(0,1,0);						//PORTB����
//	pic873init(0xff,0x7f,0xa3);						//PORTB����
	pic873init(0x3f,0xcf,0xa3);				//2014_03_06 ishii

	//���0�����ݒ�

	setup_timer_0(RTCC_INTERNAL | RTCC_DIV_2);		

	//���1�����ݒ�
	setup_timer_1(T1_INTERNAL | T1_DIV_BY_1);		
	
	//CCP1�̏����ݒ�
	setup_ccp1(CCP_PWM);					//pwm���[�h
	setup_timer_2(T2_DIV_BY_1,0xff,1);		//���2�̓t���J�E���g
	
	pwmData = kEndDataMark;					//����PWM�f�[�^��0V
	set_pwm1_duty(pwmData);					//�f���[�e�B��Z�b�g
	
	set_timer0(174);						//82�J�E���g��32.8us���ƂɊ��荞�ݔ���
	set_timer1(60536);	

//RS232C�������b�Z�[�W
	printf("START\n\r");  //START���b�Z�[�W
	int j;
	enable_interrupts(INT_RDA); //�V���A�����荞�݋���
	enable_interrupts(GLOBAL); //���荞�݋��� 
	
	while(1)
	{	

		// �R�}���h���͂ƃ`�F�b�N
//		printf("\r\nCommand=");  //�������b�Z�[�W
		
		
		KeyScan();//KEY����

	////�R�}���h���
		if(n_cmd!=0) {
			for(j=0;j<n_cmd;j++) {
				;
				printf("%c",icmd[j]); //�G�R�[�o��
//				printf("\n\r"); //LF,CR���o��
			}
			if((icmd[1]=='0') || (icmd[1]=='1')) {
			if(icmd[1]=='0') chipadrs=0xA0;  //////////////
			else               chipadrs=0xA2;  //////////////
			switch(icmd[0]) {
				case 'c': clear_mem(); break;   //�N���A
				case 'w': fill_mem(); break;    //��������
				case 'r': disp_mem(); break;    //�ǂݏo��
				default : printf("\r\nError?"); //�R�}���h�G���[
				}
			}
			else
				printf("\r\nError\r\n"); //�A�h���X�G���[
			printf("\r\nCommand="); 
			n_cmd=0;
		}
	}
	return 0;
}



/*************************************
* �������N���A�����֐�
*************************************/
void clear_mem(void)
{
	printf("\r\nClearing ");
	for (Address=0; Address<MAX_EEPROM; Address++)	//�A�h���X�͈̓`�F�b�N
	{
		if((Address > 16) && ((Address % 16) == 0))	//16�o�C�g����
			printf(".");							//16�o�C�g����*�\��
		write_ext_eeprom(Chipadrs, Address, 0xFF);	//0����������
	}
	printf("\r\nComplete!\r\n");					//�������b�Z�[�W
}
/*****************************
*  �����ݏ����֐�
*****************************/
void fill_mem(void)
{
	int i;

	Address = 0;									//�J�n�A�h���X�Z�b�g
	i = 0x20;									//�����݃f�[�^�����l
	do
	{
		write_ext_eeprom(Chipadrs, Address, i);		//�f�[�^������
//		printf("\rAddress= %04LX", Address);
//		printf("\rData= %02LX", i);
		
		Address ++;
		i++;
		if (i > 0x4F)								//�����݃f�[�^�ŏI�l�`�F�b�N
		{
			printf("\rAddress= %04LX", Address);		
			i = 0x20;							//�ď����l�Z�b�g
		}
	} while (Address < MAX_EEPROM);				//�A�h���X�ŏI�`�F�b�N
	printf("\r\nComplete!\r\n");					//�I�����b�Z�[�W
}
/*****************************
* �Ǐo�������֐�
*****************************/
void  disp_mem(void)
{
	int rdata, i;

	address = 0;									//�A�h���X�����l
	do
	{
		printf("\r\n%04LX  ", Address);				//�A�h���X���o��
		for(i=0; i<32; i++)						//32�o�C�g���ɕ��A
		{
			rdata = read_ext_eeprom(Chipadrs, Address);
			Address++;	
			printf("%C ", rdata);					//�f�[�^�𕶎��ŕ\��
		} 
	} while(Address < MAX_EEPROM);					//�A�h���X�ŏI�`�F�b�N
	printf("\r\nComplete!\r\n");					//�������b�Z�[�W
}
/*************************************
*  �O�t��EEPROM�����݃T�u�֐�
*************************************/
void write_ext_eeprom(int chip, long adrs, char wdata)
{
   i2c_start();						//I2C�X�^�[�g
   i2c_write(chip);					//�������݃��[�h���M
//   i2c_write((adrs >> 8) & 0x7F);		//��ʃA�h���X���M
   i2c_write((adrs >> 8) & 0xff);		//��ʃA�h���X���M
   i2c_write(adrs);					//���ʃA�h���X���M
   i2c_write(wdata);					//�������݃f�[�^���M
   i2c_stop();						//I2C�X�g�b�v
//   delay_ms(5);						//�������ݑ҂�
   delay_ms(10);						//�������ݑ҂�
}
/*************************************
*  �O�t��EEPROM�Ǐo���T�u�֐�
*************************************/
int read_ext_eeprom(int chip, long adrs)
{
   int value;

   i2c_start();						//I2C�X�^�[�g
   i2c_write(chip);					//�������݃��[�h
//   i2c_write((adrs >> 8) & 0x7F);		//��ʃA�h���X
   i2c_write((adrs >> 8) & 0xff);		//��ʃA�h���X
   i2c_write(adrs);					//���ʃA�h���X
   i2c_start();						//���s�[�g�X�^�[�g
   i2c_write(chip | 0x01);				//�ǂݍ��݃��[�h
   value = i2c_read(0);				//�f�[�^�ǂݍ���(ACK�ԑ����j
   i2c_stop();						//I2C�X�g�b�v
   return(value);						//�f�[�^��Ԃ�
}
	
