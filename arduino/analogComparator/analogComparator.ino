int keyPin = A0;
int press = 0;
int currKeyVal;
int lastKeyVal;
int proc = 0;

volatile unsigned long int timeISR;

#define BUTTON_NUM 5

typedef enum{
  BUTT_LEFT,
  BUTT_ROT,
  BUTT_RIGHT,
  BUTT_DOWN,
  BUTT_PAUSE
} tetrisButton;

typedef struct _button{
  int upperLimit, lowerLimit;
  tetrisButton action;
} Button;

Button buttons[BUTTON_NUM];

void setup() {
  ADCSRB = ADCSRB|(1<<ACME);
  ACSR = 
  (0<<ACD) |   // Analog Comparator: Enabled
  (0<<ACBG) |   // Analog Comparator Bandgap Select: AIN0 is applied to the positive input
  (0<<ACO) |   // Analog Comparator Output: Off
  (1<<ACI) |   // Analog Comparator Interrupt Flag: Clear Pending Interrupt
  (1<<ACIE) |   // Analog Comparator Interrupt: Enabled
  (0<<ACIC) |   // Analog Comparator Input Capture: Disabled
  (1<<ACIS1) | (1<ACIS0);   // Analog Comparator Interrupt Mode: Comparator Interrupt on Rising Output Edge

  pinMode(keyPin,INPUT_PULLUP);
  initButtons();  
  Serial.begin(9600); //  Set MIDI bitrate
}

void initButtons(){
  buttons[BUTT_LEFT].lowerLimit=320;
  buttons[BUTT_LEFT].upperLimit=360;
  buttons[BUTT_LEFT].action=BUTT_LEFT;

  buttons[BUTT_ROT].lowerLimit=600;
  buttons[BUTT_ROT].upperLimit=640;
  buttons[BUTT_ROT].action=BUTT_ROT;

  buttons[BUTT_RIGHT].lowerLimit=750;
  buttons[BUTT_RIGHT].upperLimit=790;
  buttons[BUTT_RIGHT].action=BUTT_RIGHT;

  buttons[BUTT_DOWN].lowerLimit=820;
  buttons[BUTT_DOWN].upperLimit=860;
  buttons[BUTT_DOWN].action=BUTT_DOWN;

  buttons[BUTT_PAUSE].lowerLimit=180;
  buttons[BUTT_PAUSE].upperLimit=220;
  buttons[BUTT_PAUSE].action=BUTT_PAUSE;
  
}


int decodeKey(int val){
  int i;
  for(i=0;i<BUTTON_NUM;i++){
    if((val>=buttons[i].lowerLimit)&&(val<=buttons[i].upperLimit))
      return i;
  }
  return -1;
}

void loop() {
 if(press){
  proc=1;
  int button;

  lastKeyVal = 0;
  currKeyVal = analogRead(keyPin);

  while(abs(currKeyVal-lastKeyVal)>10){
    lastKeyVal = currKeyVal;
    currKeyVal = analogRead(keyPin);
  }
  if((button = decodeKey(currKeyVal))>=0)
    Serial.println(button);
  Serial.print("time");
  Serial.println(micros()-timeISR);
  press=0;
  proc=0;
 }
}

ISR(ANALOG_COMP_vect)
{
  //Serial.println("ISR");
  if(!proc){
    timeISR=micros();
    press=1;
  }
}



