/*******************************************
	���荞�݂ɂ��L�[���͂̃`�F�b�N
		���͏����F ������e����
			������̌��[e]������A����1�����@�Ƃ����`
********************************************/

#include <16f873a.h>
#fuses HS,NOWDT,NOPROTECT,PUT,BROWNOUT,NOLVP
//RS232C�g�p�錾
#use delay(CLOCK = 20000000)
#use rs232(BAUD = 9600, XMIT = PIN_C6, RCV = PIN_C7)
//#use fast_io(B) 2014_03_17 ishii
#use fast_io(C)

char cmd[64]; // �V���A������ǂ񂾃f�[�^�̊i�[�o�b�t�@
int n_cmd=0; // �o�b�t�@���̕�����

#int_rda
void isr_rcv()
{
	char data;
	data=getc();
	do {
		if(n_cmd < 60) { // �ő�60���ɐ���
			cmd[n_cmd]=data;
			n_cmd++;
		}
		data=getc(); 
	} while(data != 'e'); 
	
/*	while(data != 'e') // 2014_03_18 ishii
	{
		if(n_cmd < 60) { // �ő�60���ɐ���
			cmd[n_cmd]=data;
			n_cmd++;
		}
		data=getc();
	} */	
	
	cmd[n_cmd]='e'; //�Ō��e��ǉ�
	n_cmd++;
//	cmd[n_cmd]=getc(); // �`�F�b�N�T���̓ǂݍ��݁@2014_03_17 ishii
//    n_cmd++; // 2014_03_17 ishii
}


////���C���֐�
void main()
{
	output_float(PIN_C3);   //SCL�s����`  //////////////////
	output_float(PIN_C4);   //SDA�s����`  //////////////////

	printf("START\n\r");  //�������b�Z�[�W
	int i;
	enable_interrupts(INT_RDA); //�V���A�����荞�݋���
	enable_interrupts(GLOBAL); //���荞�݋���

	while(1)  {             //�i�v���[�v		
		if(n_cmd!=0) {
			for(i=0;i<n_cmd;i++) {
				;
				printf("%c",cmd[i]);
			}
			printf("\n\r"); //LF,CR���o��
			n_cmd=0;
		}
	}
}


