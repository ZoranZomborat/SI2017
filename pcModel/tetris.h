#ifndef TETRIS_H
#define TETRIS_H

typedef struct _piece{
	uint8_t *data;		//pointer to piece data
	char	name;		//piece name
	uint8_t rotation;	//rotation count 1 .. 4
	uint16_t color;		//colour RGB 565
}Piece;

uint8_t o_piece[16]={
		   0x0,		0x0,	0x0,	0x0, 
		   0x0,		0xff,	0xff,	0x0,
		   0x0,		0xff,	0xff,	0x0,
		   0x0,		0x0,	0x0,	0x0};

uint8_t i_piece[2*16]={
		   0x0, 	0x0, 	0x0,	0x0, //rot 1
		   0xff, 	0xff, 	0xff,	0xff,
		   0x0, 	0x0, 	0x0,	0x0,
		   0x0, 	0x0, 	0x0,	0x0
		   ,
		   0x0, 	0x0, 	0xff,	0x0, //rot 2
		   0x0, 	0x0, 	0xff,	0x0,
		   0x0, 	0x0, 	0xff,	0x0,
		   0x0, 	0x0, 	0xff,	0x0};
		   
uint8_t s_piece[2*16]={
		   0x0, 	0x0, 	0x0, 	0x0, //rot 1
		   0x0, 	0x0, 	0xff, 	0xff,
		   0x0, 	0xff, 	0xff, 	0x0,
		   0x0, 	0x0, 	0x0, 	0x0
		   ,
		   0x0, 	0x0, 	0xff, 	0x0, //rot 2
		   0x0, 	0x0, 	0xff, 	0xff,
		   0x0, 	0x0, 	0x0, 	0xff,
		   0x0, 	0x0, 	0x0, 	0x0
			};
			
uint8_t z_piece[2*16]={
		   0x0, 	0x0, 	0x0,	0x0, //rot 1
		   0x0, 	0xff, 	0xff,	0x0,
		   0x0, 	0x0, 	0xff,	0xff,
		   0x0, 	0x0, 	0x0,	0x0
		   ,
		   0x0, 	0x0, 	0x0,	0xff, //rot 2
		   0x0, 	0x0, 	0xff,	0xff,
		   0x0, 	0x0, 	0xff,	0x0,
		   0x0, 	0x0, 	0x0,	0x0
			};
			
uint8_t l_piece[4*16]={
		   0x0, 	0x0, 	0x0,	0x0, //rot 1
		   0x0, 	0xff, 	0xff,	0xff,
		   0x0, 	0xff, 	0x0,	0x0,
		   0x0, 	0x0, 	0x0,	0x0
		   ,
		   0x0, 	0x0, 	0xff,	0x0, //rot 2
		   0x0, 	0x0, 	0xff,	0x0,
		   0x0, 	0x0, 	0xff,	0xff,
		   0x0, 	0x0, 	0x0,	0x0
		   ,
		   0x0, 	0x0, 	0x0,	0xff, //rot 3
		   0x0, 	0xff, 	0xff,	0xff,
		   0x0, 	0x0, 	0x0,	0x0,
		   0x0, 	0x0, 	0x0,	0x0
		   ,
		   0x0, 	0xff, 	0xff,	0x0, //rot 4
		   0x0, 	0x0, 	0xff,	0x0,
		   0x0, 	0x0, 	0xff,	0x0,
		   0x0, 	0x0, 	0x0,	0x0
			};
			
uint8_t j_piece[4*16]={
		   0x0, 	0x0, 	0x0,	0x0, //rot 1
		   0x0, 	0xff, 	0xff,	0xff,
		   0x0, 	0x0, 	0x0,	0xff,
		   0x0, 	0x0, 	0x0,	0x0
		   ,
		   0x0, 	0x0, 	0xff,	0xff, //rot 2
		   0x0, 	0x0, 	0xff,	0x0,
		   0x0, 	0x0, 	0xff,	0x0,
		   0x0, 	0x0, 	0x0,	0x0
		   ,
		   0x0, 	0xff, 	0x0,	0x00, //rot 3
		   0x0, 	0xff, 	0xff,	0xff,
		   0x0, 	0x0, 	0x0,	0x0,
		   0x0, 	0x0, 	0x0,	0x0
		   ,
		   0x0, 	0x0, 	0xff,	0x0, //rot 4
		   0x0, 	0x0, 	0xff,	0x0,
		   0x0, 	0xff, 	0xff,	0x0,
		   0x0, 	0x0, 	0x0,	0x0
			};
			
uint8_t t_piece[4*16]={
		   0x0, 	0x0, 	0x0,	0x0, //rot 1
		   0x0, 	0xff, 	0xff,	0xff,
		   0x0, 	0x0, 	0xff,	0x0,
		   0x0, 	0x0, 	0x0,	0x0
		   ,
		   0x0, 	0x0, 	0xff,	0x0, //rot 2
		   0x0, 	0x0, 	0xff,	0xff,
		   0x0, 	0x0, 	0xff,	0x0,
		   0x0, 	0x0, 	0x0,	0x0
		   ,
		   0x0, 	0x0, 	0xff,	0x00, //rot 3
		   0x0, 	0xff, 	0xff,	0xff,
		   0x0, 	0x0, 	0x0,	0x0,
		   0x0, 	0x0, 	0x0,	0x0
		   ,
		   0x0, 	0x0, 	0xff,	0x0, //rot 4
		   0x0, 	0xff, 	0xff,	0x0,
		   0x0, 	0x0, 	0xff,	0x0,
		   0x0, 	0x0, 	0x0,	0x0
			};
#endif