#include <16f873a.h>
#fuses HS,NOWDT,NOPROTECT,PUT,BROWNOUT,NOLVP
//RS232C使用宣言
#use delay(CLOCK = 20000000) //clock 20MHz
#use rs232(BAUD = 9600, XMIT = PIN_C6, RCV = PIN_C7)
//#use rs232(BAUD = 19200, XMIT = PIN_C6, RCV = PIN_C7)
//#use fast_io(B)
#use fast_io(C)
//I2C使用宣言
#use i2c(MASTER, SDA=PIN_C4, SCL=PIN_C3, SLOW, FORCE_HW) /////////////
//プロトタイプ宣言
void dac_0(void);
void dac_128(void);
void dac_255(void);
void write_dac(int chip, int adrs, int wdata);
///// グローバル変数、定数宣言
#define MAX_CH 0x0a							//M62398FP max ch
int	Chipadrs;									//M62398FP chip address
int Address;									//M62398FP slave address
/*************** メイン関数  *****************/
void main(void)
{
//	char cmd, num;
	char cmd;

	output_float(PIN_C3);							//SCLピンハイインピーダンスに
	output_float(PIN_C4);							//SDAピン
	while(1)										//永久ループ
	{
		// コマンド入力とチェック
		printf("\r\nCommand=1-3 ");					//初期メッセージ
		cmd = getc();
		putc(cmd);								//エコー出力
		if((cmd != '1') && (cmd != '2') && (cmd != '3'))
			printf("  Command Error?\r\n");			//コマンドエラー	
		else
		{
//			printf("\r\nDAC Slave= ");					//dac slave addメッセージ
//			num = getc();							//slave アドレス入力
//			putc(num);
//			if ((num != '0') && (num != '1'))		// アドレスチェック
//				printf("  Address Error\r\n");		//アドレスエラー
//			else
//			{
//				printf("\r\n");
//				Chipadrs = num == '0' ? 0x90 : 0x91;//アドレスセット(dac= 0x90)
				Chipadrs = 0x90;//アドレスセット(dac= 0x90)
				switch (cmd)						//コマンド処理実行
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
* dac 0 書込関数（ch0～ch9)
*************************************/
void dac_0(void)
{
	printf("\r\ndac=0 set ");
	for (Address=1; Address<MAX_CH; Address++)	//アドレス範囲チェック
	{
		write_dac(Chipadrs, Address, 0x00);	//0を書き込む
	}
	printf("\r\nComplete!\r\n");					//完了メッセージ
}

/*************************************
* dac 128 書込関数（ch0～ch9)
*************************************/
void dac_128(void)
{
	printf("\r\ndac=128 set ");
	for (Address=1; Address<MAX_CH; Address++)	//アドレス範囲チェック
	{
		write_dac(Chipadrs, Address, 0x7f);	//7fを書き込む
	}
	printf("\r\nComplete!\r\n");					//完了メッセージ
}

/*************************************
* dac 255 書込関数（ch0～ch9)
*************************************/
void dac_255(void)
{
	printf("\r\ndac=255 set ");
	for (Address=1; Address<MAX_CH; Address++)	//アドレス範囲チェック
	{
		write_dac(Chipadrs, Address, 0xff);	//ffを書き込む
	}
	printf("\r\nComplete!\r\n");					//完了メッセージ
}

/*************************************
*  M62398FP DAC 書込みサブ関数
*************************************/
void write_dac(int chip, int adrs, int wdata)
{
   i2c_start();						//I2Cスタート
   i2c_write(chip);					//書き込みモード送信(0X090)
   i2c_write(adrs);					//DAC chアドレス送信
   i2c_write(wdata);				//書き込みデータ送信
   i2c_stop();						//I2Cストップ
   delay_ms(100);						//100ms 書き込み待ち
}

