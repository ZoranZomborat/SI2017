typedef enum{
  EVENT_NULL,
  EVENT_FALL,
  EVENT_LEFT,
  EVENT_RIGHT,
  EVENT_SPIN,
  EVENT_BUTTON
} tetrisEvent;

#define BUTTON_NUM 4
#define BUTTON_IDLE HIGH
#define BUTION_PRESSED LOW

volatile int timer = 3036;
volatile int timer2 = 33;
uint8_t interupt_pin = 3;
uint8_t button_pin_left = 7;
uint8_t button_pin_right = 6;
uint8_t button_pin_spin = 5;
uint8_t button_pin_fall = 4;

uint8_t button_pins[BUTTON_NUM]={7,6,5,4};
uint8_t last_button_state[BUTTON_NUM];

uint8_t debounceDelay=20; 
volatile uint32_t lastDebounceTime = 0;

volatile tetrisEvent event, next_event=EVENT_NULL;
volatile uint8_t polling;

void initTimer(){
  // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR2A = 0;
  TCCR2B = 0;

  //TCNT1 = timer;            // preload timer 65536-8MHz/256*2(2s)
  //TCNT2 = timer2;           // preload timer 256-8MHz/1024/50(20ms)
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  //TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  TCCR2B |= (1 << CS12) | (1 << CS10);    // 1024 prescaler 
  //TIMSK2 |= (1 << TOIE2);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}

void initButtonState(){
  int i;
  for(i=0;i<BUTTON_NUM;i++){
    last_button_state[i]=BUTTON_IDLE;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(button_pin_left, INPUT_PULLUP);
  pinMode(button_pin_right, INPUT_PULLUP);
  pinMode(button_pin_spin, INPUT_PULLUP);
  pinMode(button_pin_fall, INPUT_PULLUP);
  initButtonState();
  attachInterrupt(digitalPinToInterrupt(interupt_pin), event_button, FALLING);
  //initTimer();
}

ISR(TIMER1_OVF_vect)
{
  event=EVENT_FALL;
  TCNT1 = timer;
}

ISR(TIMER2_OVF_vect)
{
  unsigned long time = millis();
  Serial.println(time-lastDebounceTime);
  lastDebounceTime=time;
  TCNT2 = timer2;
}

void event_left() {
  if(event==EVENT_NULL)
    event = EVENT_LEFT;
}

void event_right() {
  if(event==EVENT_NULL)
    event = EVENT_RIGHT;
}

void event_button(){
    event = EVENT_BUTTON;
}

uint8_t debounce(uint8_t pinIdx){
  uint8_t i;
  uint8_t reading;
  reading = digitalRead(button_pins[pinIdx]);
  last_button_state[pinIdx]=reading;
    lastDebounceTime = millis();

  while((millis() - lastDebounceTime) < debounceDelay){
    reading = digitalRead(button_pins[pinIdx]);
    if (reading != last_button_state[pinIdx]) {
      // reset the debouncing timer
      lastDebounceTime = millis();
      last_button_state[pinIdx] = reading;
    }
  }
  
  if (reading == LOW) {
    return 0x1;
  }
  
  return 0x0;
}

uint8_t pollAll(){
  int i;
  int reading;
  for(i=0;i<BUTTON_NUM;i++){
    last_button_state[i]=digitalRead(button_pins[i]);
    if(last_button_state[i]==BUTION_PRESSED){
      if(debounce(i)){
        return button_pins[i];
      }
    }
  }
  while(1){
    for(i=0;i<BUTTON_NUM;i++){
      reading=digitalRead(button_pins[i]);
      if(reading!=last_button_state[i]){
        if(debounce(i)){
          return button_pins[i];
        }
      }
    }
    return 0;
  }
}

void loop() {
  uint8_t button;
  if(event){
    switch(event){
      case EVENT_BUTTON:
        button=pollAll();
        if(button){
          Serial.print("Result  ");
          Serial.println(button);
        }
        break;
    }
    event=0x0;
  }
}
