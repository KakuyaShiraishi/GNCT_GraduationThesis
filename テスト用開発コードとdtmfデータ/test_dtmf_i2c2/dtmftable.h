//DTMFテーブル

#define kEndDataMark	127		//アナログ0V

/* 低群 */
//697Hz
static const unsigned char sinrow1[37] = {
	149, 170, 190, 208, 224, 236, 246, 253, 255, 254,
	250, 242, 230, 216, 199, 180, 160, 138, 117, 90, 75,
	56, 39, 25, 13, 5, 1, 0, 2, 9, 19, 31, 47, 65, 85,
	106, kEndDataMark
};
//770Hz
static const unsigned char sinrow2[33] = {
	152, 175, 197, 216, 232, 244, 252, 255, 254, 248,
	238, 224, 207, 186, 164, 140, 115, 91, 69, 48, 31,
	17, 7, 1, 0, 3, 11, 23, 39, 58, 80, 103, kEndDataMark
};
//852Hz
static const unsigned char sinrow3[30] = {
	154, 180, 203, 223, 238, 249, 255, 255, 249, 238,
	223, 203, 180, 154, 128, 101, 75, 52, 32, 17, 6,
	0, 0, 6, 17, 32, 52,75, 101, kEndDataMark
};
//941Hz
static const unsigned char sinrow4[27] = {
	157, 185, 210, 230, 245, 254, 255, 250, 238, 221,
	198, 171, 142, 113, 84, 57, 34, 17, 5, 0, 1, 10,
	25, 45, 70, 98, kEndDataMark
};

/* 高群 */
//1209Hz
static const unsigned char sincoluma[21] = {
	168, 200, 228, 247, 255, 252, 238, 215, 183, 147,
	108, 72, 40, 17, 3, 0, 8, 27, 55, 90, kEndDataMark
};
//1336Hz
static const unsigned char sincolumb[19] = {
	169, 206, 235, 252, 255, 245, 222, 188, 149, 106,
	67, 33, 10, 0, 3, 20, 49, 86, kEndDataMark
};
//1477Hz
static const unsigned char sincolumc[52] = {
	173, 212, 241, 255, 252, 233, 200, 158, 112, 68, 32,
	8, 0, 8, 32, 68, 112, 158, 200, 233, 252, 255, 241,
	212, 173, 128, 82, 43, 14, 0, 3, 22, 55, 97, 143,187,
	223, 247, 255, 247, 223, 187, 143, 97, 55, 22, 3, 0,
	14, 43, 82, kEndDataMark
};


/*
fH
1:1209Hz
2:1336Hz
3:1477Hz

fL
1:697Hz
2:770Hz
3:852Hz
4:941Hz
*/

static const int fH[] = {
	1,1,1,2,2,2,2,3
};
static const int fL[] = {
	1,2,3,1,2,3,4,4
};

//コピー用配列
unsigned char dataL_c[37];		//rowデータテーブル用
unsigned char dataH_c[52];		//columデータテーブル用

unsigned char *rowbase;
unsigned char *columbase;



