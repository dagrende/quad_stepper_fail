// quadrature stepper

#define dirOut 0  // stepper direction
#define stepOut 1 // stepper step
#define q0In 2    // quadrature sensor phase 0
#define q1In 3    // quadrature sensor phase 1
#define ms0Out 7  // microstepping mode 0 (to pololu M0 input)
#define ms1Out 8  // microstepping mode 1 (to pololu M1 input)
#define ms2Out 9  // microstepping mode 2 (to pololu M2 input)
#define loopOut 10  // inverted on each main loop turn, if enabled - used to check performance limit

// index by ssqq where ss=state and qq=q1q0
// value is bfss where d=1 if backward, f=1 if forward, ss=new state
byte nextState[] = {
  0b0000,  //0000 same
  0b0101,  //0001 forw
  0b1011,  //0010 back
  0b0101,  //0011 error forw
  
  0b1000,  //0100 back
  0b0001,  //0101 same
  0b0110,  //0110 error forw
  0b0110,  //0111 forw
  
  0b0111,  //1000 error forw
  0b1001,  //1001 back
  0b0111,  //1010 forw
  0b0010,  //1011 same
  
  0b0100,  //1100 forw
  0b0100,  //1101 error forw
  0b0011,  //1110 same
  0b1010  //1111 back
};

byte loopCounter = 0;
byte state = 0;
long m = 1;
long d = 1;
long sum = 0;
int ms = 0;  // single step (no microstepping) (0, 1, 2, 3, 4, 5 for 1, 2, 4, 8, 16, 32)

void processIncomingByte (const byte inByte);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(q0In, INPUT_PULLUP);
  pinMode(q1In, INPUT_PULLUP);
  
  pinMode(dirOut, OUTPUT);
  pinMode(stepOut, OUTPUT);
  
  pinMode(ms0Out, OUTPUT);
  pinMode(ms1Out, OUTPUT);
  pinMode(ms2Out, OUTPUT);
  
//  pinMode(loopOut, OUTPUT);

  outputMicrostep();
  digitalWrite(dirOut, 0);
}


void loop() {
  // digitalWrite(loopOut, (loopCounter++) & 1);

  while (Serial.available() > 0)
    processIncomingByte(Serial.read());

  byte newState = nextState[(state & 3) << 2 | (~PIND & 3)];
  state = newState & 0b11;
  byte forward = (newState & 0b0100) != 0;
  byte backward = (newState & 0b1000) != 0;
  if (forward) {
    // quad forward
    digitalWrite(dirOut, 1);
    sum += m;
    if (sum >= d) {
      digitalWrite(stepOut, 1);
      sum -= d;
      digitalWrite(stepOut, 0);
    }
  } else if (backward) {
    // quad back
    digitalWrite(dirOut, 0);
    sum -= m;
    if (sum < 0) {
      digitalWrite(stepOut, 1);
      sum += d;
      digitalWrite(stepOut, 0);
    }
  }
}

void outputMicrostep() {
   digitalWrite(ms0Out, ms & 1);
   digitalWrite(ms1Out, ms & 2);
   digitalWrite(ms2Out, ms & 4);
}

// set m n - sets multiplier to n decimal
// set d n - sets dividend to n decimal
// set ms n - sets microstepping to 1, 2, 4, 8, 16, 32
// terminate with LF
void process_data (char * data) {
  if (sscanf(data, "set m %ld", &m) == 1
      || sscanf(data, "set d %ld", &d) == 1
      || sscanf(data, "set ms %d", &ms) == 1) {
    Serial.println("OK");
    outputMicrostep();
  } else if (strcmp(data, "get") == 0) {
    sprintf(data, "m=%ld", m); Serial.println(data);
    sprintf(data, "d=%ld", d); Serial.println(data);
    sprintf(data, "ms=%d", ms); Serial.println(data);
  } else {
    Serial.println("ERROR");
  }
}

#define MAX_INPUT 50
void processIncomingByte (const byte inByte) {
  static char input_line [MAX_INPUT];
  static unsigned int input_pos = 0;
  
  switch (inByte) {
  case '\n':   // end of text
    input_line [input_pos] = 0;  // terminating null byte
    process_data (input_line);
    input_pos = 0;  
    break;
  case '\r':   // discard carriage return
    break;
  default:
    // keep adding if not full ... allow for terminating null byte
    if (input_pos < (MAX_INPUT - 1))
      input_line [input_pos++] = inByte;
    break;
  }  // end of switch
 
} // end of processIncomingByte  

