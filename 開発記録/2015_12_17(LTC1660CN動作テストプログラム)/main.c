#include <16f873a.h>
#fuses HS,NOWDT,NOPROTECT,PUT,BROWNOUT,NOLVP
//RS232C�g�p�錾
#use delay(CLOCK = 20000000) //clock 20MHz
#use rs232(BAUD = 9600, XMIT = PIN_C6, RCV = PIN_C7)
//#use rs232(BAUD = 19200, XMIT = PIN_C6, RCV = PIN_C7)
//#use fast_io(B)
#use fast_io(C)
//SPI���[�h�ݒ�
//#use spi(FORCE_HW)
//PIO���o�͏����ݒ�
void pic873init(char pa,char pb,char pc)
{
	set_tris_a(pa);
	set_tris_b(pb);
	set_tris_c(pc);
}
//�v���g�^�C�v�錾
void dac_0(void);
void dac_511(void);
void dac_1023(void);
void write_dac(int chip, int wdata, int rcount);
///// �O���[�o���ϐ��A�萔�錾
#define MAX_DEV_CS1 0x07							//LTC1660CN cs1 max device ch
#define MAX_DEV_CS2 0x04							//LTC1660CN cs2 max device ch
int Maxch;										//LTC1660 CS1/CS2 max channel 
int	Chipadrs;									//LTC1660CN chip address(add+udata 4bit)
int Address;									//LTC1660CN slave address
int	Chipdat;									//LTC1660CN chip data(ldata 5bit)
char	Chiproop_sel;					//spi chiproop select
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

int16 big_data;									//16bit big data
int8 data_u;									//8bit upper data
int8 data_l;									//8bit lower data
int data_in;									//spi input dfata(dummy)
/*************** ���C���֐�  *****************/
void main(void)
{
	char cmd, num;
//	char cmd;
	
	
//SPI���[�h�ݒ�
setup_spi(SPI_MASTER | SPI_MODE_0 | SPI_CLK_DIV_16 | SPI_SS_DISABLED); //spi clk 1.25MHz
//setup_spi(SPI_MASTER | SPI_MODE_0 | SPI_CLK_DIV_4 | SPI_SS_DISABLED); //spi clk 5MHz

//PIO�����ݒ�
	pic873init(0x3f,0xe2,0x93);
	
///	initial clear //////////
	big_data = 0;								// big data clear
	data_u = 0;									// upper data clear
	data_l = 0;									// lower data clear
///	dac1 spi dummy initial ///	
	output_low(PIN_B2);							//CS1 low�o��
	data_in = spi_read();						//dac dummy read
	output_high(PIN_B2);						//CS1 High�o��
/// dac2 spi dummy initial ///	
	output_low(PIN_B3);							//CS2 High�o��
	data_in = spi_read();						//dac dummy read
	output_high(PIN_B3);						//CS2 High�o��	
/*************************************
* rs232c dac�o�͒l�ݒ�i0,511,1023)
*************************************/
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
		// DAC_IC�̑I���
			printf("\r\nchip Select=1-2 ");		//dac chip select���b�Z�[�W
			num = getc();
			putc(num);								//�G�R�[�o��
			if ((num != '1') && (num != '2'))		// �`�b�v�`�F�b�N
				printf("  Chip select Error\r\n");		//���ߑI���G���[
			else
			{
//				printf("\r\n");
				if (num == '1'){
					Maxch = MAX_DEV_CS1;		//Maxch = 7
//					Chiproop_sel = 1;		//CS1=B2 select
					Chiproop_sel = PIN_B2;
				}
				else{
					Maxch = MAX_DEV_CS2;		//Maxch = 4
//					Chiproop_sel = 2;		//CS3=B3 select
					Chiproop_sel = PIN_B3;
				}
				switch (cmd)						//�R�}���h�������s
				{
					case '1': dac_0(); break;	//dac 0 set
					case '2': dac_511(); break;	//dac 128 set
					case '3': dac_1023(); break;	//dac 255 set
					default : break;
				}
			}
		}
	}
}
/*************************************
* dac 0 �����֐��ich0�`ch9)
*************************************/
void dac_0(void)
{
	big_data = 0;
	data_u = (big_data >> 6)&0x0f;
	data_l = (big_data << 2)&0xfc;
	printf("\r\ndac=0 set ");
	for (Address=1; Address<Maxch; Address++)	//�A�h���X�͈̓`�F�b�N
	{
		Chipadrs = ((Address<<4)&0xf0) | data_u;
		Chipdat = data_l;
		write_dac(Chipadrs, Chipdat, Chiproop_sel);	//0����������
	}
	printf("\r\nComplete!\r\n");					//�������b�Z�[�W
}

/*************************************
* dac 511 �����֐��ich0�`ch9)
*************************************/
void dac_511(void)
{
	big_data = 0x1ff;
	data_u = (big_data >> 6)&0x0f;
	data_l = (big_data << 2)&0xfc;
	printf("\r\ndac=512 set ");
	for (Address=1; Address<Maxch; Address++)	//�A�h���X�͈̓`�F�b�N
	{
		Chipadrs = ((Address<<4)&0xf0) | data_u;
		Chipdat = data_l;
		write_dac(Chipadrs, Chipdat, Chiproop_sel);	//511����������
	}
	printf("\r\nComplete!\r\n");					//�������b�Z�[�W
}

/*************************************
* dac 1023 �����֐��ich0�`ch9)
*************************************/
void dac_1023(void)
{
	big_data = 0x3ff;
	data_u = (big_data >> 6)&0x0f;
	data_l = (big_data << 2)&0xfc;
	printf("\r\ndac=1024 set ");
	for (Address=1; Address<Maxch; Address++)	//�A�h���X�͈̓`�F�b�N
	{
		Chipadrs = ((Address<<4)&0xf0) | data_u;
		Chipdat = data_l;
		write_dac(Chipadrs, Chipdat, Chiproop_sel);	//1024����������
	}
	printf("\r\nComplete!\r\n");					//�������b�Z�[�W
}

/*************************************
*  LTC1660CN DAC �����݃T�u�֐�
*************************************/
void write_dac(int chip, int wdata, int rcount)
{
//	if (rcount == 1)
//		output_low(PIN_B2);				//CS1 Low�o��
//	else
//		output_low(PIN_B3);				//CS2 Low�o��
		output_low(rcount);				//CS1/CS2 Low�o��
//		delay_us(1);					//1us �������ݑ҂�
	   spi_write(chip);					//DAC ch�A�h���X+���4bit�f�[�^���M
	   spi_write(wdata);				//�������݃f�[�^���M
//	   delay_us(1);					//1us �������ݑ҂�
//	if (rcount == 1)
//	   output_high(PIN_B2);				//CS1 High�o��
//	else
//		output_high(PIN_B3);				//CS2 Low�o��
		output_high(rcount);			//CS1/CS2 High�o��
//	   delay_ms(1);						//1ms �������ݑ҂�
}

