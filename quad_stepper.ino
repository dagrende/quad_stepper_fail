// quadrature stepper

#define dirOut 0
#define stepOut 1
#define q0Sensor 2
#define q1Sensor 3

int state = 0;
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
  
long int m = 1;
long int d = 8;
long int sum = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(q0Sensor, INPUT_PULLUP);
  pinMode(q1Sensor, INPUT_PULLUP);
  pinMode(dirOut, OUTPUT);
  pinMode(stepOut, OUTPUT);
}

void loop() {
    // handle m,d input here
  
    byte newState = nextState[(state & 3) << 2 | (~PIND & 3)];
    state = newState & 0b11;
    byte forward = (newState & 0b0100) != 0;
    byte backward = (newState & 0b1000) != 0;
    digitalWrite(dirOut, forward);
    if (forward) {
      // quad back
      sum += m;
      if (sum >= d) {
        digitalWrite(stepOut, 1);
        sum -= d;
        digitalWrite(stepOut, 0);
      }
      Serial.println(sum);
    } else if (backward) {
      // quad back
      sum -= m;
      if (sum < 0) {
        digitalWrite(stepOut, 1);
        sum += d;
        digitalWrite(stepOut, 0);
      }
      Serial.println(sum);
    }
}
