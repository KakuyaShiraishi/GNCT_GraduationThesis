/*******************************************
	割り込みによるキー入力のチェック
		入力書式： 文字列e文字
			文字列の後に[e]があり、もう1文字　という形
********************************************/

#include <16f873a.h>
#fuses HS,NOWDT,NOPROTECT,PUT,BROWNOUT,NOLVP
//RS232C使用宣言
#use delay(CLOCK = 20000000)
#use rs232(BAUD = 9600, XMIT = PIN_C6, RCV = PIN_C7)
//#use fast_io(B) 2014_03_17 ishii
#use fast_io(C)

char cmd[64]; // シリアルから読んだデータの格納バッファ
int n_cmd=0; // バッファ内の文字数

#int_rda
void isr_rcv()
{
	char data;
	data=getc();
	do {
		if(n_cmd < 60) { // 最大60字に制限
			cmd[n_cmd]=data;
			n_cmd++;
		}
		data=getc(); 
	} while(data != 'e'); 
	
/*	while(data != 'e') // 2014_03_18 ishii
	{
		if(n_cmd < 60) { // 最大60字に制限
			cmd[n_cmd]=data;
			n_cmd++;
		}
		data=getc();
	} */	
	
	cmd[n_cmd]='e'; //最後にeを追加
	n_cmd++;
//	cmd[n_cmd]=getc(); // チェックサムの読み込み　2014_03_17 ishii
//    n_cmd++; // 2014_03_17 ishii
}


////メイン関数
void main()
{
	output_float(PIN_C3);   //SCLピン定義  //////////////////
	output_float(PIN_C4);   //SDAピン定義  //////////////////

	printf("START\n\r");  //初期メッセージ
	int i;
	enable_interrupts(INT_RDA); //シリアル割り込み許可
	enable_interrupts(GLOBAL); //割り込み許可

	while(1)  {             //永久ループ		
		if(n_cmd!=0) {
			for(i=0;i<n_cmd;i++) {
				;
				printf("%c",cmd[i]);
			}
			printf("\n\r"); //LF,CRを出力
			n_cmd=0;
		}
	}
}


