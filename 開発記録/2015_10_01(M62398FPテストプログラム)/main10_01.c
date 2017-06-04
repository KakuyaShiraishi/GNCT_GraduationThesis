#include <16f873a.h>
#fuses HS,NOWDT,NOPROTECT,PUT,BROWNOUT,NOLVP
//RS232C�g�p�錾
#use delay(CLOCK = 20000000) //clock 20MHz
#use rs232(BAUD = 9600, XMIT = PIN_C6, RCV = PIN_C7)
//#use rs232(BAUD = 19200, XMIT = PIN_C6, RCV = PIN_C7)
//#use fast_io(B)
#use fast_io(C)
//I2C�g�p�錾
#use i2c(MASTER, SDA=PIN_C4, SCL=PIN_C3, SLOW, FORCE_HW) /////////////
//�v���g�^�C�v�錾
void dac_0(void);
void dac_128(void);
void dac_255(void);
void write_dac(int chip, int adrs, int wdata);
///// �O���[�o���ϐ��A�萔�錾
#define MAX_CH 0x0a							//M62398FP max ch
int	Chipadrs;									//M62398FP chip address
int Address;									//M62398FP slave address
/*************** ���C���֐�  *****************/
void main(void)
{
//	char cmd, num;
	char cmd;

	output_float(PIN_C3);							//SCL�s���n�C�C���s�[�_���X��
	output_float(PIN_C4);							//SDA�s��
	while(1)										//�i�v���[�v
	{
		// �R�}���h���͂ƃ`�F�b�N
		printf("\r\nCommand=1-3 ");					//�������b�Z�[�W
		cmd = getc();
		putc(cmd);								//�G�R�[�o��
		if((cmd != '1') && (cmd != '2') && (cmd != '3'))
			printf("  Command Error?\r\n");			//�R�}���h�G���[	
		else
		{
//			printf("\r\nDAC Slave= ");					//dac slave add���b�Z�[�W
//			num = getc();							//slave �A�h���X����
//			putc(num);
//			if ((num != '0') && (num != '1'))		// �A�h���X�`�F�b�N
//				printf("  Address Error\r\n");		//�A�h���X�G���[
//			else
//			{
//				printf("\r\n");
//				Chipadrs = num == '0' ? 0x90 : 0x91;//�A�h���X�Z�b�g(dac= 0x90)
				Chipadrs = 0x90;//�A�h���X�Z�b�g(dac= 0x90)
				switch (cmd)						//�R�}���h�������s
				{
					case '1': dac_0(); break;	//dac 0 set
					case '2': dac_128(); break;	//dac 128 set
					case '3': dac_255(); break;	//dac 255 set
					default : break;
				}
//			}
		}
	}
}
/*************************************
* dac 0 �����֐��ich0�`ch9)
*************************************/
void dac_0(void)
{
	printf("\r\ndac=0 set ");
	for (Address=1; Address<MAX_CH; Address++)	//�A�h���X�͈̓`�F�b�N
	{
		write_dac(Chipadrs, Address, 0x00);	//0����������
	}
	printf("\r\nComplete!\r\n");					//�������b�Z�[�W
}

/*************************************
* dac 128 �����֐��ich0�`ch9)
*************************************/
void dac_128(void)
{
	printf("\r\ndac=128 set ");
	for (Address=1; Address<MAX_CH; Address++)	//�A�h���X�͈̓`�F�b�N
	{
		write_dac(Chipadrs, Address, 0x7f);	//7f����������
	}
	printf("\r\nComplete!\r\n");					//�������b�Z�[�W
}

/*************************************
* dac 255 �����֐��ich0�`ch9)
*************************************/
void dac_255(void)
{
	printf("\r\ndac=255 set ");
	for (Address=1; Address<MAX_CH; Address++)	//�A�h���X�͈̓`�F�b�N
	{
		write_dac(Chipadrs, Address, 0xff);	//ff����������
	}
	printf("\r\nComplete!\r\n");					//�������b�Z�[�W
}

/*************************************
*  M62398FP DAC �����݃T�u�֐�
*************************************/
void write_dac(int chip, int adrs, int wdata)
{
   i2c_start();						//I2C�X�^�[�g
   i2c_write(chip);					//�������݃��[�h���M(0X090)
   i2c_write(adrs);					//DAC ch�A�h���X���M
   i2c_write(wdata);				//�������݃f�[�^���M
   i2c_stop();						//I2C�X�g�b�v
   delay_ms(100);						//100ms �������ݑ҂�
}

