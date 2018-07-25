// quadrature stepper

#include <EEPROM.h>

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
struct {
  long m;
  long d;
  int ms;  // single step (no microstepping) (0, 1, 2, 3, 4, 5 for 1, 2, 4, 8, 16, 32)
} prefs = {1, 1, 0};
long sum = 0;

char encoderChange(char a, char b);
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

  digitalWrite(dirOut, 0);

  EEPROM.get(0, prefs);
  if (prefs.m < 1) {
    prefs.m = 1;
    prefs.d = 1;
    prefs.ms = 0;
  }
  outputMicrostep();
}

long pos = 0;
long qpn = 0;

int maxRec = 10000;

void oneChange(char change) {
  if (change == 1) {
    // quad forward
    digitalWrite(dirOut, 1);
    sum += prefs.m;
    if (sum >= prefs.d) {
      digitalWrite(stepOut, 1);
      sum -= prefs.d;
      digitalWrite(stepOut, 0);
      ++pos;
    }
  } else if (change == -1) {
    // quad back
    digitalWrite(dirOut, 0);
    sum -= prefs.m;
    if (sum < 0) {
      digitalWrite(stepOut, 1);
      sum += prefs.d;
      digitalWrite(stepOut, 0);
      --pos;
    }
  }
  qpn += change;
}

void loop() {
  // digitalWrite(loopOut, (loopCounter++) & 1);

  while (Serial.available() > 0 && maxRec > 0) {
    processIncomingByte(Serial.read());
    maxRec--;
  }

  // char change = encoderChange(digitalRead(q0In), digitalRead(q1In));
  // if (change != 0) {
  //   oneChange(change);
  // }
  digitalWrite(dirOut, 0);
  digitalWrite(stepOut, 1);
  delay(1);
  digitalWrite(stepOut, 0);
  delay(5);
}

// call continously with the two phases A, B of the quadrature encoder
// returns the change to the position based on any quadrature switch change
char encoderChange(char a, char b) {
  // position change based on [fromSwitches][toSwitches]
  // where switches is two bits AB
  static char changeTable[4][4] = {
   //00  01  10  11 - prevSwitches
    { 0,  1, -1,  0}, //00 - switches
    {-1,  0,  0,  1}, //01
    { 1,  0,  0, -1}, //10
    { 0, -1,  1,  0}  //11
  };
  static char prevSwitches = -1;

  char switches = (a << 1) + b;
  if (prevSwitches == -1) {
    prevSwitches = switches;
  }
  char change = changeTable[prevSwitches][switches];
  prevSwitches = switches;
  return change;
}


void outputMicrostep() {
   digitalWrite(ms0Out, prefs.ms & 1);
   digitalWrite(ms1Out, prefs.ms & 2);
   digitalWrite(ms2Out, prefs.ms & 4);
}

// simulate 10000
void test() {
  pos = 0;
  sum = 0;
  int n = 5600;
  for (int i = 0; i < n; ++i) {
    oneChange(1);
  }
  Serial.print("n="); Serial.println(n);
  Serial.print("pos="); Serial.println(pos);
  Serial.print("sum="); Serial.println(sum);

  Serial.print("correct pos="); Serial.println(n * prefs.m / prefs.d);
}

void usDelay(unsigned long d) {
  unsigned long start = micros();
  while (micros() - start < d) ;
}

void nForward(int n) {
  reset();
  for (int i = 0; i < n; ++i) {
    oneChange(n > 0 ? 1 : -1);
    usDelay(5000);
  }
}

void reset() {
  pos = 0;
  sum = 0;
  qpn = 0;
}

void savePrefs() {
  EEPROM.put(0, prefs);
}

// set m n - sets multiplier to n decimal
// set d n - sets dividend to n decimal
// set ms n - sets microstepping to 1, 2, 4, 8, 16, 32
// terminate with LF
void process_data (char * data) {
  if (sscanf(data, "set m %ld", &prefs.m) == 1
      || sscanf(data, "set d %ld", &prefs.d) == 1
          // || sscanf(data, "set ms %d", &prefs.ms) == 1
        ) {
    Serial.println(data);
    outputMicrostep();
    sum = 0;
    savePrefs();
  } else if (strncmp(data, "set ms ", 7) == 0) {
    prefs.ms = atoi(data + 7);
    outputMicrostep();
    savePrefs();
  } else if (strcmp(data, "get") == 0) {
    Serial.print("m="); Serial.println(prefs.m);
    Serial.print("d="); Serial.println(prefs.d);
    Serial.print("ms="); Serial.println(prefs.ms);
    Serial.print("sum="); Serial.println(sum);
    Serial.print("pos="); Serial.println(pos);
    Serial.print("qpn="); Serial.println(qpn);
  } else if (strcmp(data, "reset") == 0) {
    reset();
  } else if (strcmp(data, "test") == 0) {
    test();
  } else if (strncmp(data, "forw ", 5) == 0) {
    nForward(atoi(data + 5));
  } else {
    Serial.println("ERROR");
    Serial.print("data length="); Serial.println(strlen(data));
    Serial.print("data="); Serial.println(data);

  }
}

#define MAX_INPUT 50
void processIncomingByte (const byte inByte) {
  static char input_line [MAX_INPUT];
  static unsigned int input_pos = 0;

  switch (inByte) {
  case '\n':   // end of text
  // Serial.print("input_pos="); Serial.println(input_pos);
  // Serial.print("input_line[input_pos]="); Serial.println(input_line[input_pos]);
    input_line[input_pos] = 0;  // terminating null byte
    input_pos = 0;
    process_data (input_line);
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
