#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include "tetris.h"
#include "tetris_windows.h"
#include <SPI.h>
#include <MemoryFree.h>

#define TFT_CS     10
#define TFT_RST    8
#define TFT_DC     9

#define TFT_SCLK   13
#define TFT_MOSI   11

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

volatile uint16_t timer=3036; //start timer 2 seconds
uint8_t button_pin = A0;
volatile int button_pressed = 0, button_process = 0;
volatile tetrisEvent event=EVENT_NULL;
volatile tetrisEvent lastevent=EVENT_NULL;

Piece pieces [PIECE_NUM];
uint8_t screenPiece[BMP_SIZE];

int x_curr,y_curr,rot_curr;
int x_prev,y_prev,rot_prev;
uint8_t piece_curr, piece_next;

uint8_t kernel[PIECE_SIZE]; //data that gets write to screen at x_curr y_curr

typedef struct _board{
  uint8_t stuck_arr[BOARD_SIZE];
}Board;

Board board;

Button buttons[BUTTON_NUM];

void initTimer(){
  // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = timer;            // preload timer 65536-8MHz/256
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}

void initAnalogComp(){
  noInterrupts();
  ADCSRB = ADCSRB|(1<<ACME);
  ACSR = 
  (0<<ACD) |   // Analog Comparator: Enabled
  (0<<ACBG) |   // Analog Comparator Bandgap Select: AIN0 is applied to the positive input
  (0<<ACO) |   // Analog Comparator Output: Off
  (1<<ACI) |   // Analog Comparator Interrupt Flag: Clear Pending Interrupt
  (1<<ACIE) |   // Analog Comparator Interrupt: Enabled
  (0<<ACIC) |   // Analog Comparator Input Capture: Disabled
  (1<<ACIS1) | (1<ACIS0);   // Analog Comparator Interrupt Mode: Comparator Interrupt on Rising Output Edge
  interrupts();
}

void setup() {
  Serial.begin(9600);
  Serial.print("Hello! ST7735 TFT Test");
  tft.initR(INITR_144GREENTAB);
  tft.setRotation(2);
  Serial.print("Initialized");
  tft.fillScreen(ST7735_BLACK);
  initPieces();
  initButtons();
  randomSeed(analogRead(0));
  pinMode(button_pin,INPUT);
  initAnalogComp();
  initTimer();
}

ISR(TIMER1_OVF_vect)
{
  event = EVENT_FALL;
  //Serial.println("timer time");
  TCNT1 = timer;
}


ISR(ANALOG_COMP_vect)
{
  if(!button_process)
    button_pressed = 1;
}

void initPieces(){
  
  //O piece
  pieces[0].data = o_piece;
  pieces[0].rotation = 1;
  pieces[0].color = tft.Color565(255,255,0);
  pieces[0].numWindows = 1;
  pieces[0].windows = win_o;

  //I piece
  pieces[1].data = i_piece;
  pieces[1].rotation = 2;
  pieces[1].color = tft.Color565(0,255,255);
  pieces[1].numWindows = 1;
  pieces[1].windows = win_i;

  //S piece
  pieces[2].data = s_piece;
  pieces[2].rotation = 2;
  pieces[2].color = tft.Color565(0,255,0);
  pieces[2].numWindows = 2;
  pieces[2].windows = win_s;

  //Z piece
  pieces[3].data = z_piece;
  pieces[3].rotation = 2;
  pieces[3].color = tft.Color565(255,0,0);
  pieces[3].numWindows = 2;
  pieces[3].windows = win_z;

  //L piece
  pieces[4].data = l_piece;
  pieces[4].rotation = 4;
  pieces[4].color = tft.Color565(255,128,0);
  pieces[4].numWindows = 2;
  pieces[4].windows = win_l;

  //J piece
  pieces[5].data = j_piece;
  pieces[5].rotation = 4;
  pieces[5].color = tft.Color565(0,0,255);
  pieces[5].numWindows = 2;
  pieces[5].windows = win_j;

  //T piece
  pieces[6].data = t_piece;
  pieces[6].rotation = 4;
  pieces[6].color = tft.Color565(128,0,255);
  pieces[6].numWindows = 2;
  pieces[6].windows = win_t;
  
}

void initButtons(){
  buttons[BUTT_LEFT].lowerLimit=320;
  buttons[BUTT_LEFT].upperLimit=360;
  buttons[BUTT_LEFT].action=EVENT_LEFT;

  buttons[BUTT_ROT].lowerLimit=600;
  buttons[BUTT_ROT].upperLimit=640;
  buttons[BUTT_ROT].action=EVENT_ROT;

  buttons[BUTT_RIGHT].lowerLimit=750;
  buttons[BUTT_RIGHT].upperLimit=790;
  buttons[BUTT_RIGHT].action=EVENT_RIGHT;

  buttons[BUTT_DOWN].lowerLimit=820;
  buttons[BUTT_DOWN].upperLimit=860;
  buttons[BUTT_DOWN].action=EVENT_FALL;

  buttons[BUTT_PAUSE].lowerLimit=180;
  buttons[BUTT_PAUSE].upperLimit=220;
  buttons[BUTT_PAUSE].action=EVENT_PAUSE;
  
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

void translateCoordinates(int x_board, int y_board, int *x_screen, int *y_screen){
  *(x_screen)=(x_board - BOARD_PAD_LEFT) * PP_BLOCK_W + SCREEN_PAD_LEFT;
  *(y_screen)=(y_board) * PP_BLOCK_H + SCREEN_PAD_TOP;
}

void padBoard(){
  int i, j;
  int idx;
  for (i = 0; i < BOARD_H; i++) { //pad two vertical lines left and one right
    idx = i * BOARD_W;
    board.stuck_arr[idx] = board.stuck_arr[idx + 1] = board.stuck_arr[idx + BOARD_W - 1] = 0xff;
  }
  for(i = 0; i < BOARD_W-1; i++){ //pad one horizontal line at the bottom
    for(j=0;j<BOARD_PAD_H;j++){
      idx=(BOARD_H - 1 - j) * BOARD_W + i;
      board.stuck_arr[idx]=0xff;
    }
  }
}

void initScreenBorder(){
  int i;
  for(i=0;i<SCREEN_PAD_LEFT; i++){
    tft.drawFastVLine(i, 0, SCREEN_H_PADDED, ST7735_WHITE);
  }
  for(i=0;i<SCREEN_PAD_RIGHT; i++){
    tft.drawFastVLine(SCREEN_PAD_LEFT + BOARD_SCREEN_PX_W + i, 0, SCREEN_H_PADDED, ST7735_WHITE);
  }
  for(i=0;i<SCREEN_PAD_TOP; i++){
    tft.drawFastHLine(SCREEN_PAD_LEFT, i, BOARD_SCREEN_PX_W, ST7735_WHITE);
  }
  for(i=0;i<SCREEN_PAD_BOTTOM; i++){
    tft.drawFastHLine(SCREEN_PAD_LEFT,SCREEN_PAD_TOP + BOARD_SCREEN_PX_H + i, BOARD_SCREEN_PX_W, ST7735_WHITE);
  }
}

void stickBlock(uint8_t *piece, int x, int y){
  int i,j;
  for (i = 0; i < PIECE_H; i++) {
    for (j = 0; j < PIECE_W; j++) {
      int idx = (i + y) * BOARD_W + j + x;
      int pidx = i * PIECE_W + j;
      board.stuck_arr[idx] |= piece[pidx];
    }
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

void updateScreenWindows(Piece *piece){
  uint8_t i;
  uint8_t x0, y0, x1, y1;
  uint16_t j;
  uint16_t sizeArea;
  uint8_t pieceRot = rot_prev % piece->rotation; //clear last move
  for(i=0;i<piece->numWindows;i++){
    uint8_t wIdx= WIN_VERTICES*(piece->numWindows*pieceRot+i);
    uint8_t *pWindows = &piece->windows[wIdx];
    x0=pWindows[0] + TRANSLATE_X(x_prev);
    y0=pWindows[1] + TRANSLATE_Y(y_prev);
    x1=pWindows[2] + TRANSLATE_X(x_prev);
    y1=pWindows[3] + TRANSLATE_Y(y_prev);
    tft.setAddrWindow(x0, y0, x1, y1);
    sizeArea = (x1-x0+1)*(y1-y0+1);
    j=0;
    while(j<sizeArea){
      tft.pushColor(ST7735_BLACK);
      j++;
    }
  }
  pieceRot = rot_curr % piece->rotation; //draw current move
  for(i=0;i<piece->numWindows;i++){
    uint8_t wIdx= WIN_VERTICES*(piece->numWindows*pieceRot+i);
    uint8_t *pWindows = &piece->windows[wIdx];
    x0=pWindows[0] + TRANSLATE_X(x_curr);
    y0=pWindows[1] + TRANSLATE_Y(y_curr);
    x1=pWindows[2] + TRANSLATE_X(x_curr);
    y1=pWindows[3] + TRANSLATE_Y(y_curr);
    tft.setAddrWindow(x0, y0, x1, y1);
    sizeArea = (x1-x0+1)*(y1-y0+1);
    j=0;
    while(j<sizeArea){
      tft.pushColor(piece->color);
      j++;
    }
  }
}

int8_t tryDrawBlock(Piece *piece, int x, int y) {
  int i, j;
  int pieceRot = rot_curr % piece->rotation;
  uint8_t * pieceData = & piece->data[pieceRot*PIECE_SIZE];
  if (!checkDrawBlock(pieceData, x, y)) {
    if(isStuck(pieceData, x, y)){
      stickBlock(pieceData, x_prev, y_prev);
      //tryMakeLine(y_prev);
      x_prev = x_curr = X_START;
      y_prev = y_curr = Y_START;
      rot_prev = rot_curr= ROT_START;
      piece_curr=piece_next;
      piece_next=random(PIECE_NUM);
      if(tryDrawBlock(&pieces[piece_curr], x_curr, y_curr) == 0)
        return -1;
      return 1;
    }
    return 0;
  } else {
    updateScreenWindows(piece);
    rot_prev=rot_curr;
    x_prev=x;
    y_prev=y;
    return 1;
  }
}

void serialPoll(){
  char c;
  while(!Serial.available());
    c=Serial.read();
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
}

int decodeKey(int val){
  int i;
  for(i=0;i<BUTTON_NUM;i++){
    if((val>=buttons[i].lowerLimit)&&(val<=buttons[i].upperLimit))
      return buttons[i].action;
  }
  return EVENT_NULL;
}

void buttonPoll(){

  while(event==EVENT_NULL && !button_pressed);
  if(!event){
    button_process = 1;
    int button;
  
    int lastKeyVal = 0;
    int currKeyVal = analogRead(button_pin);
  
    while(abs(currKeyVal-lastKeyVal)>5){
      lastKeyVal = currKeyVal;
      currKeyVal = analogRead(button_pin);
    }
    event = decodeKey(currKeyVal);
    button_pressed = 0;
    button_process = 0;
    if(event==EVENT_NULL){
      lastevent=EVENT_NULL;
      return;
    }
  }
  
  switch(event){
    case EVENT_LEFT:
      x_curr--;
      LIMIT_W(x_curr);
      break;
    case EVENT_RIGHT:
      x_curr++;
      LIMIT_W(x_curr);
      break;
    case EVENT_FALL:
      y_curr++;
      y_curr=LIMIT_H(y_curr);
      break;
    case EVENT_ROT:
      rot_curr++;
      rot_curr%=4;
      break;
    default:
      break;
  }
  lastevent=event;
  event=0x0;
}

void tetris(){
  int i, j;
  char c;
  padBoard();
  x_prev = x_curr = X_START;
  y_prev = y_curr = 9;
  rot_prev = rot_curr= ROT_START;
  piece_curr=random(PIECE_NUM);
  piece_next=random(PIECE_NUM);
  while(1){
    x_curr=x_prev;
    y_curr=y_prev;
    rot_curr=rot_prev;
    buttonPoll();
    //Serial.printf("x curr:%d prev:%d\ny curr:%d prev:%d\nrot curr:%d prev:%d\n\n",x_curr,x_prev,y_curr,y_prev,rot_curr,rot_prev));
    if(lastevent!=EVENT_NULL){
      if(tryDrawBlock(&pieces[piece_curr], x_curr, y_curr)<0)
        return;
    }
    //Serial.println(freeMemory());
  }
}

void testButtons(){
  while(1){
    buttonPoll();
    Serial.print("EVENT:");
    Serial.println(lastevent);
  }
}

void loop() {

  initScreenBorder();
  tetris();
  while(1)
  {
    Serial.println("GAME OVER");
    delay(2000);
  }
  //testButtons();
  
}

