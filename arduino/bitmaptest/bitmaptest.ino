#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include "bitmaptest.h"
#include "tetris_bitmaps.h"
#include <SPI.h>
#include <MemoryFree.h>

#define TFT_CS     10
#define TFT_RST    8
#define TFT_DC     9

#define TFT_SCLK   13
#define TFT_MOSI   11

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

#define PIECE_NUM   7
#define PIECE_H     4  
#define PIECE_W     4
#define PIECE_SIZE (PIECE_H*PIECE_W)

#define PP_BLOCK_H  8
#define PP_BLOCK_W  8
#define PP_BLOCK  (PP_BLOCK_W*PP_BLOCK_H)

#define SCREEN_PIECE_SIZE (PP_BLOCK*PIECE_SIZE/8)

Piece pieces [PIECE_NUM];
uint8_t screenPiece[SCREEN_PIECE_SIZE];

uint8_t piece_buff[PIECE_SIZE*4];
uint8_t *dump;

void initPieces(){

  int i,j,k,r;
  //O piece
  pieces[0].data = o_piece;
  pieces[0].rotation = 1;
  pieces[0].color = tft.Color565(255,255,0);

  //I piece
  pieces[1].data = i_piece;
  pieces[1].rotation = 2;
  pieces[1].color = tft.Color565(0,204,255);

}

void generateBitmapGivenPPB(uint8_t * pieceData, uint8_t * bmpDst,uint8_t pixelsPerBlock) {
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
  memcpy(bmpDst, bmp, pp_block_w * pp_block_h * PIECE_SIZE / 8);
  free(bmp);
}

void setup() {
  Serial.begin(9600);
  Serial.print("Hello! ST7735 TFT Test");
  tft.initR(INITR_144GREENTAB);
  tft.setRotation(2);
  Serial.print("Initialized");
  tft.fillScreen(ST7735_BLACK);
  initPieces();
}

int availableMemory()
{
  int size = 8192;
  byte *buf;
  while ((buf = (byte *) malloc(--size)) == NULL)
    size/=2;
  free(buf);
  return size;
} 

void loop() {
  int i,j;
  int rot=0;
  uint8_t r=255;
  uint8_t g=51;
  uint8_t b=204;
  int x = 4;
  int y = 4;
  dump = o_bmp;
  dump = i_bmp;
  dump = s_bmp;
  dump = z_bmp;
  dump = l_bmp;
  dump = j_bmp;
  dump = t_bmp;
  dump = o_piece;
  dump = i_piece;
  dump = s_piece;
  dump = z_piece;
  dump = l_piece;
  dump = j_piece;
  dump = t_piece;
  for(i=0;i<PIECE_SIZE*2;i++){
     piece_buff[i] = pgm_read_byte(s_piece+i);
  }
  generateBitmapGivenPPB(s_piece, screenPiece, 2);
  tft.drawBitmap(x, y, screenPiece, 8, 8, tft.Color565(0,255,255));

  x += 10;
  generateBitmapGivenPPB(s_piece, screenPiece, 4);
  tft.drawBitmap(x,y,screenPiece,16,16,tft.Color565(255,255,0));

  uint8_t bmp6[72]={0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00,
                  0x00, 0x0f, 0xff,
                  0x00, 0x0f, 0xff,
                  0x00, 0x0f, 0xff,
                  0x00, 0x0f, 0xff,
                  0x00, 0x0f, 0xff,
                  0x00, 0x0f, 0xff,
                  0x03, 0xff, 0xc0,
                  0x03, 0xff, 0xc0,
                  0x03, 0xff, 0xc0,
                  0x03, 0xff, 0xc0,
                  0x03, 0xff, 0xc0,
                  0x03, 0xff, 0xc0,
                  0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00 
                  };

 // x += 18;
  //tft.drawBitmap(x,y,bmp6,24,24,tft.Color565(255,255,0));
  tft.fillScreen(ST7735_WHITE);
  x += 26;
  for(i=0;i<PIECE_SIZE*4;i++){
     piece_buff[i] = pgm_read_byte(l_piece+i);
  }
  while(1){
      Serial.println(freeMemory());
      generateBitmapGivenPPB(&piece_buff[rot*PIECE_SIZE], screenPiece, 8);
      tft.drawBitmap(x, y, screenPiece, 32, 32, tft.Color565(255,0,255));
      delay(100);
      tft.fillRect(x,y,32,32,ST7735_BLACK);
      rot=(rot+1)%4;
      //Serial.print(rot);
  }

}

