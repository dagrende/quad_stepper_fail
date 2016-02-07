# quad_stepper
Arduino program to synchronize a stepper motor to a quadrature rotation sensor, multiplied by a fraction m/d &lt; 1

## Developed with
- Arduino development environment 1.6.7 om Mac OS X
- SparkFun Pro Micro 5V/16MHz
- Pololu DRV8825 stepper driver

##Connections

Connect Arduino pin function - to
- RXI dirOut - Pololu DIR
- TXO stepOut - Pololu STEP
- 2 q0In - Quadrature sensor phase 0
- 3 q1In - Quadrature sensor phase 1
- 7 ms0Out - Pololu M0
- 8 ms0Out - Pololu M1
- 9 ms0Out - Pololu M2

## Serial commands

The arduino USB Serial inteface is used to send command from a computer to the arduino, for setting frequency multipliter and divider, and microstepping mode.
The USB serial interface appears under vaious names depending on OS etc.
- In the arduino environment it can be accessed by the serial console
- On Linux it may be /dev/ttyUSB0 or /dev/ttyACM0
- On OS X it may be /dev/cu.usbmodem1411, number varies

Commands
- set m x - set the multiplier to x
- set d x - sets the divider to x
- set ms x - sets the microstepping mode to x

All commands are terminated newline. 
Program responds with OK.
Parameters are not stored, so they have to be set at every power-on.

### Microstepping modes
- 0 - 1/1 (no microstepping)
- 1 - 1/2
- 2 - 1/4
- 3 - 1/8
- 4 - 1/16
- 5 - 1/32
- 6 - 1/32
- 7 - 1/32

