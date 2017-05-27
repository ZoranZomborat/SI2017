#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>
#include <time.h>

#include "tetris.h"

#define LIMIT_W(x)	x=(x<0)?0:(x>(BOARD_W-PIECE_W)?BOARD_W-PIECE_W:x)
#define LIMIT_H(y)	y=(y>(BOARD_H-PIECE_H+1))?(BOARD_H-PIECE_H+1):(y<(0)?0:y) //leave one more for stucking to rock bottom

#define PIECE_NUM   7
#define PIECE_H     4
#define PIECE_W     4
#define PIECE_SIZE (PIECE_H*PIECE_W)

#define PP_BLOCK_H	8
#define PP_BLOCK_W	8
#define PP_BLOCK	(PP_BLOCK_W*PP_BLOCK_H)

#define BMP_SIZE	(PP_BLOCK * PIECE_SIZE / 8)

#define BOARD_SCREEN_H		15
#define BOARD_SCREEN_W		10

#define BOARD_PAD_H			2
#define BOARD_PAD_LEFT		2
#define BOARD_PAD_RIGHT		1
#define BOARD_PAD_W			(BOARD_PAD_LEFT + BOARD_PAD_RIGHT)

#define BOARD_H				(BOARD_SCREEN_H + BOARD_PAD_H)
#define BOARD_W				(BOARD_SCREEN_W + BOARD_PAD_W)
#define BOARD_SIZE			(BOARD_H * BOARD_W)

#define SCREEN_PAD_LEFT		4
#define SCREEN_PAD_RIGHT	4
#define SCREEN_PAD_TOP		4
#define SCREEN_PAD_BOTTOM	4

#define X_START				(BOARD_SCREEN_W / 2 + BOARD_PAD_LEFT - PIECE_W / 2)
#define Y_START				0
#define ROT_START			0

typedef struct _board{
	uint8_t screen_arr[BOARD_SIZE];
	uint8_t stuck_arr[BOARD_SIZE];
	uint8_t draw_arr[BOARD_SIZE];
}Board;

Board board;
Piece pieces[7];
uint8_t bmp_buff[BMP_SIZE];
uint8_t kernel[PIECE_SIZE];

int x_curr,y_curr,rot_curr;
int x_prev,y_prev,rot_prev;

const char *byte_to_binary(uint8_t x)
{
	static char b[9];
	int z;
	b[0]='\0';
	for (z = 128; z > 0; z >>= 1)
	{
		strcat(b, ((x & z) == z) ? "1" : "0");
	}

	return b;
}

uint8_t *generateBitmapGivenPPB(uint8_t * pieceData, uint8_t pixelsPerBlock) {

	int j,k;
	int pp_block_w = pixelsPerBlock;
	int pp_block_h = pixelsPerBlock;
	int mask=(1<<pp_block_w)-1;
	uint8_t *bmp = (uint8_t *) calloc(pp_block_w * pp_block_h * PIECE_SIZE / 8, sizeof(uint8_t));
	int ph, pw;
	int bmph = 0;
	int bytesPerLine=(pp_block_w * PIECE_W / 8) ;
	for (ph = 0; ph < PIECE_H; ph++) {
		for (pw = 0; pw < PIECE_W; pw++) {
			int pieceIdx = ph * PIECE_W + pw;
			if (pieceData[pieceIdx]) {
				int bmpIdx = ph * pp_block_h * bytesPerLine + (pw * pp_block_w) / 8;
				bmp[bmpIdx] |= (mask << (8*(1 + ((pw * pp_block_w)/8)) - pp_block_w - pw * pp_block_w));
			}
		}
		for (j = 1; j < pp_block_h; j++) { //duplicate rows to match pixels height per block
			for (k = 0; k < bytesPerLine; k++) {
				bmp[bmph + j * bytesPerLine + k] = bmp[bmph + k];
			}
		}
		bmph += pp_block_h*bytesPerLine;
	}

	return bmp;
}

void generateBitmap(uint8_t * pieceData) {

	int j,k;
	int mask=(1<<PP_BLOCK_W)-1;
	uint8_t *bmp = (uint8_t *) calloc(BMP_SIZE, sizeof(uint8_t));
	int ph, pw;
	int bmph = 0;
	int bytesPerLine=(PP_BLOCK_W * PIECE_W / 8) ;
	for (ph = 0; ph < PIECE_H; ph++) {
		for (pw = 0; pw < PIECE_W; pw++) {
			int pieceIdx = ph * PIECE_W + pw;
			if (pieceData[pieceIdx]) {

				int bmpIdx = ph * PP_BLOCK_H * bytesPerLine + (pw * PP_BLOCK_W) / 8;
				bmp[bmpIdx] |= (mask << (8*(1 + ((pw * PP_BLOCK_W)/8)) - PP_BLOCK_W - pw * PP_BLOCK_W));
			}
		}
		for (j = 1; j < PP_BLOCK_H; j++) { //duplicate rows to match pixels height per block
			for (k = 0; k < bytesPerLine; k++) {
				bmp[bmph + j * bytesPerLine + k] = bmp[bmph + k];
			}
		}
		bmph += PP_BLOCK_H*bytesPerLine;
	}
	memcpy(bmp_buff, bmp, BMP_SIZE);
	free(bmp);
}

void traceWindows(){
	int i;
	Window window;

	for(i=0;i<PIECE_NUM;i++){
		Piece piece = pieces[i];
		for(j=0;j<PIECE_SIZE;j++){
			if(piece.data[j]){
				int j,k;
				window.x0=j%PIECE_W;
				window.y0=j/PIECE_W;

			}
		}
	}
}

FILE *fout;
void formatBitmap(uint8_t *bmp, uint8_t last){

	int i, j;
	for (i = 0; i < PIECE_H * PP_BLOCK_H; i++) {
		for (j = 0; j < (PIECE_W * PP_BLOCK_W / 8); j++) {
			int inIdx = i * (PIECE_W * PP_BLOCK_W / 8) + j;
			if(last && i==( PIECE_H * PP_BLOCK_H - 1)&& j== (PIECE_W * PP_BLOCK_W / 8 - 1))
				fprintf(fout,"0x%X\t", bmp[inIdx]);
			else
				fprintf(fout,"0x%x,\t", bmp[inIdx]);
		}
		fprintf(fout,"\n");
	}
	fprintf(fout,"\n");
}

void generateAllBitmaps(){
	int i,j;
	fout = fopen("tetris_bitmaps.h","wt");
	if(fout ==NULL){
		printf("err at fopen!\n");
		exit(2);
	}
	
	fprintf(fout,"#ifndef TETRIS_BITMAPS_H\n#define TETRIS_BITMAPS_H\n\n");
	for(i=0;i<PIECE_NUM;i++){
		fprintf(fout,"const uint8_t %c_bmp[%d] PROGMEM = {\n", pieces[i].name, pieces[i].rotation*BMP_SIZE);
		for(j=0;j<pieces[i].rotation;j++){
			int idx = j*PIECE_SIZE;
			generateBitmap(&(pieces[i].data[idx]));
			if(j==pieces[i].rotation-1)
				formatBitmap(bmp_buff,1);
			else
				formatBitmap(bmp_buff,0);
		}
		fprintf(fout,"};\n\n\n");
	}
	fprintf(fout,"\n#endif\n");
	fclose(fout);
}

void initPieces(){

  int i,j,k,r;
  //O piece
  pieces[0].name = 'o';
  pieces[0].data = o_piece;
  pieces[0].rotation = 1;
  pieces[0].numWindows = 1;
  pieces[0].windows = win_o;

  //I piece
  pieces[1].name = 'i';
  pieces[1].data = i_piece;
  pieces[1].rotation = 2;
  pieces[1].numWindows = 2;
  pieces[1].windows = win_i;

  //S piece
  pieces[2].name = 's';
  pieces[2].data = s_piece;
  pieces[2].rotation = 2;
  pieces[2].numWindows = 2;
  pieces[2].windows = win_s;

  //Z piece
  pieces[3].name = 'z';
  pieces[3].data = z_piece;
  pieces[3].rotation = 2;
  pieces[3].numWindows = 2;
  pieces[3].windows = win_z;

  //L piece
  pieces[4].name = 'l';
  pieces[4].data = l_piece;
  pieces[4].rotation = 4;
  pieces[4].numWindows = 4;
  pieces[4].windows = win_l;

  //J piece
  pieces[5].name = 'j';
  pieces[5].data = j_piece;
  pieces[5].rotation = 4;
  pieces[5].numWindows = 4;
  pieces[5].windows = win_j;

  //T piece
  pieces[6].name = 't';
  pieces[6].data = t_piece;
  pieces[6].rotation = 4;
  pieces[6].numWindows = 4;
  pieces[6].windows = win_t;

}

void padBoard(){
	int i, j;
	int idx;
	for (i = 0; i < BOARD_H; i++) {	//pad two vertical lines left and one right
		idx = i * BOARD_W;
		board.stuck_arr[idx] = board.stuck_arr[idx + 1] = board.stuck_arr[idx + BOARD_W - 1] = 0xff;
	}
	
	for(i = 0; i < BOARD_W-1; i++){	//pad one horizontal line at the bottom
		for(j=0;j<BOARD_PAD_H;j++){
			idx=(BOARD_H - 1 - j) * BOARD_W + i;
			board.stuck_arr[idx]=0xff;
		}
	}
	memcpy(board.draw_arr,board.stuck_arr,BOARD_SIZE);
}

void fillBoard(){
	int idx;
	int lines=3;
	int hole;
	int i;
	idx=(BOARD_H - BOARD_PAD_H - lines)*BOARD_W;
	memset(&board.stuck_arr[idx],0xff,lines*BOARD_W);
	for(i=0;i<3;i++){
		//hole=rand()%(BOARD_SCREEN_W) + BOARD_PAD_LEFT;
		hole=5;
		idx=(BOARD_H - BOARD_PAD_H - i - 1)*BOARD_W + hole;
		board.stuck_arr[idx]=0x00;
	}
	idx=(BOARD_H - BOARD_PAD_H - lines)*BOARD_W + 4;
	board.stuck_arr[idx]=0x00;
	memcpy(board.draw_arr,board.stuck_arr,BOARD_SIZE);
}

void printBoard(){
	int i,j;
	for(i=0; i<BOARD_H; i++){
		for(j=0; j<BOARD_W; j++){
			printf("%d",(board.draw_arr[i*BOARD_W+j])?1:0);
		}
		printf("\t");
		for(j=0; j<BOARD_W; j++){
			printf("%d",(board.stuck_arr[i*BOARD_W+j])?1:0);
		}
		printf("\t");
		for(j=0; j<BOARD_W; j++){
			printf("%d",(board.screen_arr[i*BOARD_W+j])?1:0);
		}
		printf("\n");
	}
}

void printBitmap(uint8_t *bmp) {

	int i, j;
	for (i = 0; i < PIECE_H * PP_BLOCK_H; i++) {
		for (j = 0; j < (PIECE_W * PP_BLOCK_W / 8); j++) {
			int inIdx = i * (PIECE_W * PP_BLOCK_W / 8) + j;
			printf("[%s]\t", byte_to_binary(bmp[inIdx]));
		}
		printf("\n");
	}
}

uint8_t isStuck(uint8_t *piece, int x, int y){
	int i,j;
	if(y_curr==y_prev)
		return 0;
	for(i=0;i<PIECE_W;i++){
		for(j=PIECE_H-1;j>=0;j--){
			int pidx = j * PIECE_W + i;
			int idx=(j+y)*BOARD_W + i + x;
			if(piece[pidx] && (i+x)>=BOARD_PAD_LEFT && (i+x)<(BOARD_W-BOARD_PAD_RIGHT)){
				if(piece[pidx] & board.stuck_arr[idx])
					return 1;
				else
					j=-1;
			}
		}
	}
	return 0;
}

uint8_t checkDrawBlock(uint8_t *piece, int x, int y) {

	int i, j;
	for (i = 0; i < PIECE_H; i++) {
		for (j = 0; j < PIECE_W; j++) {
			int idx = (i + y) * BOARD_W + j + x;
			int pidx = i * PIECE_W + j;
			if (board.stuck_arr[idx] & piece[pidx])
				return 0;
		}
	}

	return 1;
}

void stickBlock(uint8_t *piece, int x, int y){
	int i,j;
	for (i = 0; i < PIECE_H; i++) {
		for (j = 0; j < PIECE_W; j++) {
			int idx = (i + y) * BOARD_W + j + x;
			int pidx = i * PIECE_W + j;
			board.stuck_arr[idx] = board.draw_arr[idx]= board.stuck_arr[idx] | piece[pidx];
		}
	}
}

void drawBlock(uint8_t *piece, int x, int y) {
	int i, j;
	for (i = 0; i < PIECE_H; i++) {
		for (j = 0; j < PIECE_W; j++) {
			int idx = (i + y) * BOARD_W + j + x;
			int pidx = i * PIECE_W + j;
			board.draw_arr[idx] = board.stuck_arr[idx] | piece[pidx];
		}
	}
}

void translateCoordinates(int x_board, int y_board, int *x_screen, int *y_screen){
	*(x_screen)=(x_board - BOARD_PAD_LEFT) * PP_BLOCK_W + SCREEN_PAD_LEFT;
	*(y_screen)=(y_board) * PP_BLOCK_H + SCREEN_PAD_TOP;
}

void updateScreen(){
	int i,j;
	int pidx, idx;
	int screen_x;
	int screen_y;
	translateCoordinates(x_prev,y_prev,&screen_x,&screen_y);
	for(i=0;i<PIECE_H;i++) {
		for(j=0;j<PIECE_W;j++){
			idx = (i+x_prev)*BOARD_W + j + x_prev;
			board.screen_arr[idx]=0x0;
		}
	}
	translateCoordinates(x_curr,y_curr,&screen_x,&screen_y);
	for(i=0;i<PIECE_H;i++) {
		for(j=0;j<PIECE_W;j++){
			pidx = i * PIECE_W + j;
			idx = (i+x_curr)*BOARD_W + j + x_curr;
			board.screen_arr[idx]=kernel[pidx];
		}
	}
}

void extractKernel(uint8_t *piece, int x, int y) {
	int i,j;
	int pidx, idx;
	for(i=0;i<PIECE_H;i++) {
		for(j=0;j<PIECE_W;j++){
			pidx = i * PIECE_W + j;
			idx = (i+y)*BOARD_W + j + x;
			kernel[pidx]=piece[pidx]|board.stuck_arr[idx];
		}
	}
}

void eliminateLines(int line){
	int i,j;
	int idx;
	int top =0;
	for(i=line;i>top;i--){ //swift lines downwards
		for(j=BOARD_PAD_LEFT;j<BOARD_W-BOARD_PAD_RIGHT;j++)
		{
			idx=(i * BOARD_W) + j;
			board.stuck_arr[idx]=board.stuck_arr[idx-BOARD_W];
		}
	}
	for(j=BOARD_PAD_LEFT;j<BOARD_W-BOARD_PAD_RIGHT;j++)
	{
		idx=(top * BOARD_W) + j;
		board.stuck_arr[idx]=0x00;
	}
}

int tryMakeLine(int y){

	uint8_t i,j;
	int idx;
	int foundLine;
	for(i=0;i<PIECE_H;i++){
		foundLine = i+y;
		if(foundLine>=BOARD_SCREEN_H) //don't chew into padding
			break;
		for(j=BOARD_PAD_LEFT;j<BOARD_W-BOARD_PAD_RIGHT;j++){
			idx=(i+y)*BOARD_W + j;
			if(!board.stuck_arr[idx]){
				foundLine=-1;
				break;
			}
		}
		if(foundLine>=0)
			eliminateLines(foundLine);
	}
	memcpy(board.screen_arr,board.stuck_arr,BOARD_SIZE);
	return -1;
}

uint8_t tryDrawBlock(uint8_t *piece, int x, int y) {
	int i, j;
	if (!checkDrawBlock(piece, x, y)) {
		if(isStuck(piece, x, y)){
			stickBlock(piece, x_prev, y_prev);
			printf("\nSTUCK!!\n");
			tryMakeLine(y_prev);
			x_prev = x_curr = X_START;
			y_prev = y_curr = Y_START;
			rot_prev = rot_curr= ROT_START;
			return 1;
		}
		return 0;
	} else {
		for (i = 0; i < PIECE_H; i++) {
			drawBlock(piece, x, y);
		}
		extractKernel(piece,x_curr,y_curr);
		updateScreen();
		rot_prev=rot_curr;
		x_prev=x;
		y_prev=y;
		memcpy(board.screen_arr,board.draw_arr,BOARD_SIZE);
		return 1;
	}
}

void tetris(){
	int i, j;
	char c;
	padBoard();
	fillBoard();
	//printBoard();
	x_prev = x_curr = X_START;
	y_prev = y_curr = 9;
	rot_prev = rot_curr= ROT_START;

	while(1){
		fflush(stdin);
		c=getchar();
		printf("[%c]\n",c);
		x_curr=x_prev;
		y_curr=y_prev;
		rot_curr=rot_prev;
		switch(c){
		case 'a':
			x_curr--;
			LIMIT_W(x_curr);
			break;
		case 'd':
			x_curr++;
			LIMIT_W(x_curr);
			break;
		case 's':
			y_curr++;
			y_curr=LIMIT_H(y_curr);
			break;
		case 'w':
			rot_curr++;
			rot_curr%=4;
			break;
		default:
			break;
		}
		printf("x curr:%d prev:%d\ny curr:%d prev:%d\nrot curr:%d prev:%d\n\n",x_curr,x_prev,y_curr,y_prev,rot_curr,rot_prev);
		if(!tryDrawBlock(&l_piece[rot_curr * PIECE_SIZE], x_curr, y_curr))
			drawBlock(&l_piece[rot_prev * PIECE_SIZE], x_prev, y_prev);
		printf("\n");
		printBoard();
		memcpy(board.draw_arr, board.stuck_arr, BOARD_SIZE);
		fflush(stdin);
		getch();
		system("cls");
	}
	for(;x_curr >= 0;x_curr--){
		for (j = 0; j < 4; j++) {
			tryDrawBlock(&l_piece[j * PIECE_SIZE], x_curr, y_curr);
			printBoard();
			printf("\n");
			memcpy(board.draw_arr, board.stuck_arr, BOARD_SIZE);
		}
		printf("\n");
	}
}

int main(void) {

	initPieces();
	//generateAllBitmaps();
	tetris();
	return EXIT_SUCCESS;
}

