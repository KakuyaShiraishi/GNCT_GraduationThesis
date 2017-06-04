//6点点字キーボード型点字入力装置のプログラム

#include <16f873a.h>
#include "dtmftable.h"

#fuses HS,NOWDT,NOPROTECT,PUT,BROWNOUT,NOLVP
#use delay(CLOCK = 20000000)
//RS232C使用宣言
//#use rs232(BAUD = 9600, XMIT = PIN_C6, RCV = PIN_C7)
#use rs232(BAUD = 19200, XMIT = PIN_C6, RCV = PIN_C7) //2014_03_27
//I2C使用宣言
#use i2c(MASTER, SDA=PIN_C4, SCL=PIN_C3, ADDRESS=0xa0, FORCE_HW) /////////////
//#use i2c(MASTER, SDA=PIN_C4, SCL=PIN_C3, ADDRESS=0xa0) /////////////

#use fast_io(c)

#define CODE_TIME 50

//EEPROM プロトタイプ宣言
void clear_mem(void);
void fill_mem(void);
void disp_mem(void);
void write_ext_eeprom(int chip, long adrs, int wdata);
int read_ext_eeprom(int chip, long adrs);
/////グローバル変数、定数宣言
#define MAX_EEPROM 0x0400     // Maximum 0x20000   ////////////////
static int	Chipadrs;		  //EEPROMチップアドレス
static long Address;		  //EEPROMアドレス

/* DTMF グローバル変数定義 */
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

//RS232C初期設定
//char icmd[64]; // シリアルから読んだデータの格納バッファ 2014_03_18 ishii make err74
char icmd[24]; // シリアルから読んだデータの格納バッファ
int n_cmd=0; // バッファ内の文字数

#int_rda
void isr_rcv()
{
	char data;
	data=getc();
	do {
//		if(n_cmd < 60) { // 最大60字に制限 2014_03_18 ishii
		if(n_cmd < 20) { // 最大30字に制限
			icmd[n_cmd]=data;
			n_cmd++;
		}
		data=getc(); 
	} while(data != 'e'); 
	icmd[n_cmd]='e'; //最後にeを追加
	n_cmd++;
//	cmd[n_cmd]=getc(); // チェックサムの読み込み　2014_03_17 ishii
//    n_cmd++; // 2014_03_17 ishii
}

//PIC入出力初期設定
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

//1msﾀｲﾏ
#int_timer1
void interrupt_1(void)				
{		
	set_timer1(60536);// 0.001/(0.00000005*4*1(ﾌﾟﾘｽｹｰﾗ1:1）)=5000
				 	 // 65535(16bit)-5000+1(0ｶｳﾝﾄ）=60536
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

	set_timer0(174);//0.0000328/(0.00000005*4*2(ﾌﾟﾘｽｹｰﾗ2:1）)=82
				    //256(8bit)-82=174
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
//i2C初期設定
	output_float(PIN_C3);   //SCLピン定義  //////////////////
	output_float(PIN_C4);   //SDAピン定義  //////////////////
	
		
//PIO初期設定
//	pic873init(0,1,0);						//PORTB入力
//	pic873init(0xff,0x7f,0xa3);						//PORTB入力
	pic873init(0x3f,0xcf,0xa3);				//2014_03_06 ishii

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

//RS232C初期メッセージ
	printf("START\n\r");  //STARTメッセージ
	int j;
	enable_interrupts(INT_RDA); //シリアル割り込み許可
	enable_interrupts(GLOBAL); //割り込み許可 
	
	while(1)
	{	

		// コマンド入力とチェック
//		printf("\r\nCommand=");  //初期メッセージ
		
		
		KeyScan();//KEY入力

	////コマンド解析
		if(n_cmd!=0) {
			for(j=0;j<n_cmd;j++) {
				;
				printf("%c",icmd[j]); //エコー出力
//				printf("\n\r"); //LF,CRを出力
			}
			if((icmd[1]=='0') || (icmd[1]=='1')) {
			if(icmd[1]=='0') chipadrs=0xA0;  //////////////
			else               chipadrs=0xA2;  //////////////
			switch(icmd[0]) {
				case 'c': clear_mem(); break;   //クリア
				case 'w': fill_mem(); break;    //書き込み
				case 'r': disp_mem(); break;    //読み出し
				default : printf("\r\nError?"); //コマンドエラー
				}
			}
			else
				printf("\r\nError\r\n"); //アドレスエラー
			printf("\r\nCommand="); 
			n_cmd=0;
		}
	}
	return 0;
}



/*************************************
* メモリクリア処理関数
*************************************/
void clear_mem(void)
{
	printf("\r\nClearing ");
	for (Address=0; Address<MAX_EEPROM; Address++)	//アドレス範囲チェック
	{
		if((Address > 16) && ((Address % 16) == 0))	//16バイトごと
			printf(".");							//16バイト毎に*表示
		write_ext_eeprom(Chipadrs, Address, 0xFF);	//0を書き込む
	}
	printf("\r\nComplete!\r\n");					//完了メッセージ
}
/*****************************
*  書込み処理関数
*****************************/
void fill_mem(void)
{
	int i;

	Address = 0;									//開始アドレスセット
	i = 0x20;									//書込みデータ初期値
	do
	{
		write_ext_eeprom(Chipadrs, Address, i);		//データ書込み
//		printf("\rAddress= %04LX", Address);
//		printf("\rData= %02LX", i);
		
		Address ++;
		i++;
		if (i > 0x4F)								//書込みデータ最終値チェック
		{
			printf("\rAddress= %04LX", Address);		
			i = 0x20;							//再初期値セット
		}
	} while (Address < MAX_EEPROM);				//アドレス最終チェック
	printf("\r\nComplete!\r\n");					//終了メッセージ
}
/*****************************
* 読出し処理関数
*****************************/
void  disp_mem(void)
{
	int rdata, i;

	address = 0;									//アドレス初期値
	do
	{
		printf("\r\n%04LX  ", Address);				//アドレス見出し
		for(i=0; i<32; i++)						//32バイト毎に復帰
		{
			rdata = read_ext_eeprom(Chipadrs, Address);
			Address++;	
			printf("%C ", rdata);					//データを文字で表示
		} 
	} while(Address < MAX_EEPROM);					//アドレス最終チェック
	printf("\r\nComplete!\r\n");					//完了メッセージ
}
/*************************************
*  外付けEEPROM書込みサブ関数
*************************************/
void write_ext_eeprom(int chip, long adrs, char wdata)
{
   i2c_start();						//I2Cスタート
   i2c_write(chip);					//書き込みモード送信
//   i2c_write((adrs >> 8) & 0x7F);		//上位アドレス送信
   i2c_write((adrs >> 8) & 0xff);		//上位アドレス送信
   i2c_write(adrs);					//下位アドレス送信
   i2c_write(wdata);					//書き込みデータ送信
   i2c_stop();						//I2Cストップ
//   delay_ms(5);						//書き込み待ち
   delay_ms(10);						//書き込み待ち
}
/*************************************
*  外付けEEPROM読出しサブ関数
*************************************/
int read_ext_eeprom(int chip, long adrs)
{
   int value;

   i2c_start();						//I2Cスタート
   i2c_write(chip);					//書き込みモード
//   i2c_write((adrs >> 8) & 0x7F);		//上位アドレス
   i2c_write((adrs >> 8) & 0xff);		//上位アドレス
   i2c_write(adrs);					//下位アドレス
   i2c_start();						//リピートスタート
   i2c_write(chip | 0x01);				//読み込みモード
   value = i2c_read(0);				//データ読み込み(ACK返送つき）
   i2c_stop();						//I2Cストップ
   return(value);						//データを返す
}
	
