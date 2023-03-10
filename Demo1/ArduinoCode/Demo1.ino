unsigned long Tc; //current time =
int N = 1; // loop iterations
#include <Encoder.h>   // encoder Arduino library

//distance and angle
float desiredDistance = 0; //in feet
float desiredAngle = 90; //in degrees

//encoder pins for localization (2 and 3 are interrupt pins)
Encoder Lwheel(3, 5);
Encoder Rwheel(2, 11);

//constants
float radPerCount = 0.001963;
float pi = 3.14159;
float diameter = 6; //inches
float vehicleRadius = 6; //distance from centerpoint of vehicle to the wheel (inches)

float radius = diameter/2; //radius of the wheels (inches)
float circumference = pi * diameter; //circumference of the wheels (inches)
float requiredRadiansFwd; //radians required for vehicle to move forward
float requiredRadiansTrn; //radians required for vehicle to turn in place
float desiredAngleRad = (pi/180)*desiredAngle; //desired angle converted to radians
float rotationDistance = vehicleRadius*desiredAngleRad; //distance for each wheel to travel to achieve desired angle
bool hasTurned = false; //has the vehicle completed turning


//SETUP PINS (don't change these, they are specified on the motor driver data sheet)
int D2 = 4; //motor board enable pin
int mLDirPin = 7; //Left motor direction pin
int mLSpeedPin = 9; //Left motor speed pin 
int mRDirPin = 8; //right motor direction pin
int mRSpeedPin = 10; //right motor speed pin

// Initializing controller gains
float Kp = 10; // Proportional gain
float Ki = 0; // Integral gain

// error variables
  //left
float IeL; // left running integral of error
float eL; // left current error
float IeR; // right running integral of error
float eR; //right current error

// Time variables
float Ts = 5; // sampling rate I think?(10 ms)
int CL; //left motor voltage
int CR; //right motor voltage


 void setup() {
  // put your setup code here, to run once:
  Serial.begin(250000);  //start serial communication w/ baud rate of 250000
  motorSetup();
  
  //distance calculations
  requiredRadiansFwd = ((desiredDistance*12)/(circumference/2))*pi; //radians required to travel desired distance
  
  //turn in place calculations
  requiredRadiansTrn = ((rotationDistance)/(circumference/2))*pi; //radians required to turn in place to desired angle  
  
  //if no need to turn
  if (desiredAngle == 0) {
    hasTurned = true;
  }
}

void loop() {
  // put your main code here, to run repeatedly:


  //TURNING MOVEMENT
  if (true == true) {

    //TESTING
    Serial.print(getCurrentAngle());
    Serial.print(" ");
    Serial.print(requiredRadiansTrn);
    Serial.print(" ");
    Serial.print(getCurrentPosL());
    Serial.print(" ");
    Serial.print(getCurrentPosR());
    Serial.print(" eL: ");
    Serial.print(eL);
    Serial.print(" eR: ");
    Serial.print(eR);

    Serial.println();

    //left motor calculations
    eL = abs(requiredRadiansTrn) - getCurrentPosL();
    IeL = IeL + eL * Ts*0.001; // calculating the integral
    CL = Kp* eL + IeL* Ki; // This will be in volts

    //right motor calculations
    eR = abs(requiredRadiansTrn) - getCurrentPosR();
    IeR = IeR + eR * Ts*0.001; // calculating the integral
    CR = Kp* eR + IeR* Ki; // This will be in volts
  
    if (desiredAngle > 0) { //clockwise
      while (eL > 0.05) {
        //left motor
        digitalWrite(mLDirPin, HIGH);
        analogWrite(mLSpeedPin, abs(int(51*CL))); //255/CL
        break;
      }

      while (eR > 0.05) {
        //right motor
        digitalWrite(mRDirPin, LOW);
        analogWrite(mRSpeedPin, abs(int(51*CR))); //~ had negatives
        break;
      }
    }

    if (desiredAngle < 0) { //counterclockwise
      while (eL > 0.05) {
        //left motor
        digitalWrite(mLDirPin, LOW);
        analogWrite(mLSpeedPin, abs(int(51*CL))); // ~
        break;
      }
      
      while (eR > 0.05) {
        //right motor
        digitalWrite(mRDirPin, HIGH);
        analogWrite(mRSpeedPin, abs(int(51*CR)));
       break;
      }
      
    }
  }


  //ANGLE CHECK
  if ((getCurrentAngle() - desiredAngle) == 0) { //once both motors have reached the desired angle
    hasTurned = true;
  }
  
  //FORWARD MOVEMENT
  if (hasTurned == true) {
    //TESTING
    //Serial.print(requiredRadiansFwd);

    //left calculations
    Tc = millis(); //get current time ms
    eL = requiredRadiansFwd - getCurrentPosL();
    
    while (eL > 0.05) {
      IeL = IeL + eL * Ts*0.001; // calculating the integral
      CL = Kp* eL + IeL* Ki; // This will be in volts
      //left motor
      if (CL >= 0) { //forward
        digitalWrite(mLDirPin, HIGH);
        analogWrite(mLSpeedPin, int(51*CL));
      } else { //backward
        digitalWrite(mLDirPin, LOW);
        analogWrite(mLSpeedPin, int(-51*CL));
      }
      break;
    }

    //TESTING
    /*
    Serial.print(" ");
    Serial.print(getCurrentPosL());
    Serial.print(" ");
    Serial.print(CL);
    */

    //right calculations
    eR = requiredRadiansFwd - getCurrentPosR();

    while (eR > 0.05) {
      IeR = IeR + eR * Ts*0.001; // calculating the integral
      CR = Kp* eR + IeR* Ki; // This will be in volts

      //right motor
      if (CR >= 0) { //forward
        digitalWrite(mRDirPin, HIGH);
        analogWrite(mRSpeedPin, int(51*CR));
      } else { //backward
        digitalWrite(mRDirPin, LOW);
        analogWrite(mRSpeedPin, int(-51*CR));
      } 
      break;
    }

    /*
    //TESTING
    Serial.print(" ");
    Serial.print(getCurrentPosR());
    Serial.print(" ");
    Serial.print(CR);

    Serial.println();
    */    
  }

  Tc = millis();
  while(millis() < Tc + Ts){}
}

//function that gets the current position of the wheel
float getCurrentPosR() {
  long newCountR = Rwheel.read();
  float thetaR = newCountR * radPerCount;
  return thetaR;
}

//function that gets the current position of the left wheel
float getCurrentPosL() {
  long newCountL = Lwheel.read();
  float thetaL = newCountL * radPerCount;
  return thetaL;
}

//function that will check the current angle of the vehicle after turning in place
float getCurrentAngle() {
  float leftAngle = (((getCurrentPosL()*circumference)/pi)/vehicleRadius)*(180/pi);
  float rightAngle = (((getCurrentPosR()*circumference)/pi)/vehicleRadius)*(180/pi);


  float vehicleAngle = (abs(leftAngle) + abs(rightAngle))/2;

  if (rightAngle > 0) { //if the vehicle travels counterclockwise then the angle is negative
    vehicleAngle = vehicleAngle*-1;
  }

  return vehicleAngle;
}

// This function initilizes the motor, direction, and enable pins
void motorSetup() {
  pinMode(mRDirPin, OUTPUT);
  pinMode(mRSpeedPin, OUTPUT);
  pinMode(mLDirPin, OUTPUT);
  pinMode(mLSpeedPin, OUTPUT);
  pinMode(D2, OUTPUT);
  digitalWrite(D2,HIGH); //enable motor driver board
  analogWrite(mRSpeedPin, 0); //set motor voltage to 0
  analogWrite(mLSpeedPin, 0); //set motor voltage to 0
}

