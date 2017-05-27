#ifndef TETRIS_WONDOWS
#define TETRIS_WONDOWS

#define WIN_VERTICES 4

uint8_t win_o[WIN_VERTICES*1]={8, 8, 23, 23};

uint8_t win_i[WIN_VERTICES*2]={0, 8, 31, 15,
								16, 0, 23, 31};

uint8_t win_s[WIN_VERTICES*4]={16, 8, 31, 15,
								8, 16, 23, 23,
								16, 0, 23, 15,
								24, 8, 31, 23};

uint8_t win_z[WIN_VERTICES*4]={8, 8, 23, 15,
								16, 16, 31, 23,
								16, 8, 23, 23,
								24, 0, 31, 15};

uint8_t win_l[WIN_VERTICES*8]={8, 8, 31, 15,
								8, 16, 15, 23,
								16, 0, 23, 23,
								24, 16, 31, 23,
								24, 0, 31, 7,
								8, 8, 31, 15,
								8, 0, 15, 7,
								16, 0, 23, 23};

uint8_t win_j[WIN_VERTICES*8]={8, 8, 31, 15,
								24, 16, 31, 23,
								16, 0, 23, 23,
								24, 0, 31, 7,
								8, 0, 15, 7,
								8, 8, 31, 15,
								8, 16, 15, 23,
								16, 0, 23, 23};

uint8_t win_t[WIN_VERTICES*8]={8, 8, 31, 15,
								16, 16, 23, 23,
								16, 0, 23, 23,
								24, 8, 31, 15,
								16, 0, 23, 7,
								8, 8, 31, 15,
								8, 8, 15, 15,
								16, 0, 23, 23};

#endif