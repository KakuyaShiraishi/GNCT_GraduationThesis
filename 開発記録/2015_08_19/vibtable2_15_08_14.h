//VIBRATIONテーブル

#define kEndDataMark	127		//アナログ0V


//40Hz
static const unsigned char sinrow1[40] = {
	148, 167, 186, 203, 218, 231, 241, 249, 253, 255,
	253, 249, 241, 231, 218, 203, 186, 167, 148, 128,
	108, 89, 71, 54, 38, 25, 15, 7, 3, 1,
	3, 7, 15, 25, 38, 53, 70, 88, 108, kEndDataMark
};

//50Hz
//static const unsigned char sinrow2[32] = {
//	153, 177, 199, 218, 234, 245, 253, 255, 253, 245,
//	234, 218, 199, 177, 153, 128, 103, 80, 58, 38,
//	23, 11, 4, 1, 3, 11, 22, 38, 57, 79,
//	103, kEndDataMark
//};
static const unsigned char sinrow2[33] = {
	154, 177, 198, 217, 232, 244, 252, 255, 254, 248, 238,
	223, 206, 186, 163, 140, 116, 92, 70, 49, 32
	18, 8, 2, 1, 4, 12, 24, 40, 58, 80
	103, kEndDataMark
};

//60Hz
//static const unsigned char sinrow3[27] = {
//	157, 185, 210, 230, 245, 253, 255, 250, 238, 221,
//	198, 172, 143, 114, 85, 59, 36, 18, 7, 1,
//	3, 11, 26, 46, 70, 98, kEndDataMark
//};
static const unsigned char sinrow3[28] = {
	154, 181, 205, 226, 242, 251, 255, 252, 243, 228, 208,
	185, 158, 129, 101, 74, 50, 29, 14, 4, 1,
	4, 13, 28, 48, 72, 99, kEndDataMark
};
//70Hz
//static const unsigned char sinrow4[23] = {
//	162, 194, 221, 241, 252, 255, 248, 232, 208, 179,
//	146, 111, 78, 48, 25, 9, 1, 4, 15, 35,
//	61, 93, kEndDataMark
//};
static const unsigned char sinrow4[24] = {
//	154, 185, 213, 235, 249, 255, 252, 241, 221, 196, 165,
//	132, 99, 68, 41, 20, 6, 1, 5, 17, 37,
//	63, 94, kEndDataMark
	160, 191, 217, 238, 250, 255, 251, 238, 218, 192, 162,
	129, 96, 65, 39, 19, 6, 1, 5, 17, 37,
	63, 94, kEndDataMark
};
//80Hz
//static const unsigned char sinrow5[20] = {
//	167, 203, 231, 249, 255, 249, 231, 203, 167, 128,
//	89, 54, 25, 7, 1, 7, 25, 53, 88, kEndDataMark
//};
static const unsigned char sinrow5[21] = {
	167, 200, 228, 247, 255, 252, 238, 214, 183, 147, 109,
	73, 42, 18, 4, 1, 9, 28, 55, 89, kEndDataMark
};
//90Hz
//static const unsigned char sinrow6[18] = {
//	171, 210, 238, 253, 253, 238, 210, 172, 129, 85,
//	47, 18, 3, 3, 18, 46, 84, kEndDataMark
//};
static const unsigned char sinrow6[19] = {
167, 204, 233, 250, 255, 245, 223, 190, 151, 109, 69,
36, 12, 2, 4, 21, 49, 85, kEndDataMark
};
//100Hz
//static const unsigned char sinrow7[16] = {
//	177, 218, 245, 255, 245, 218, 177, 128, 80, 38,
//	11, 1, 11, 38, 79, kEndDataMark
//};
static const unsigned char sinrow7[17] = {
173, 213, 241, 254, 250, 230, 196, 152, 106, 62, 27,
6, 1, 14, 42, 81, kEndDataMark

};
//110Hz
//static const unsigned char sinrow8[15] = {
//	180, 222, 249, 254, 238, 203, 155, 102, 54, 18,
//	2, 7, 33, 76, kEndDataMark
//};
static const unsigned char sinrow8[16] = {
	173, 215, 244, 255, 247, 220, 179, 131, 82, 40, 11,
	1, 10, 37, 79, kEndDataMark
};
/*
fL
1:40Hz
2:50Hz
3:60Hz
4:70Hz
5:80Hz
6:90Hz
7:100Hz
8:110Hz
*/

static const int fL[] = {
	1,2,3,4,5,6,7,8
};
//コピー用配列
unsigned char dataL_c[40];		//rowデータテーブル用
unsigned char *rowbase;




