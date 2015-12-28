#include <Bounce2.h>

#define q0Sensor 2
#define q1Sensor 3

int state = 0;
// index by ssqq where ss=state and qq=q1q0
byte nextState[] = {
  0b00,  //0000 same
  0b01,  //0001 forw
  0b11,  //0010 back
  0b01,  //0011 error forw
  
  0b00,  //0100 back
  0b01,  //0101 same
  0b10,  //0110 error forw
  0b10,  //0111 forw
  
  0b11,  //1000 error forw
  0b01,  //1001 back
  0b11,  //1010 forw
  0b10,  //1011 same
  
  0b00,  //1100 forw
  0b00,  //1101 error forw
  0b11,  //1110 same
  0b10  //1111 back
};
  

Bounce q0SensorDebouncer = Bounce();
Bounce q1SensorDebouncer = Bounce();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(q0Sensor, INPUT_PULLUP);
  q0SensorDebouncer.attach(q0Sensor);
  q0SensorDebouncer.interval(10);
  
  pinMode(q1Sensor, INPUT_PULLUP);
  q1SensorDebouncer.attach(q1Sensor);
  q1SensorDebouncer.interval(10);
}

void loop() {
  if (q0SensorDebouncer.update() || q1SensorDebouncer.update()) {
    int q0 = 1 & !q0SensorDebouncer.read();
    int q1 = 1 & !q1SensorDebouncer.read();
    int i = (state & 3) << 2 | (q1 << 1) | q0;
    state = nextState[i];
    Serial.println(state);
  }
}
