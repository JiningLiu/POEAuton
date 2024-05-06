

#pragma region VEXcode Generated Robot Configuration
// Make sure all required headers are included.
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>


#include "vex.h"

using namespace vex;

// Brain should be defined by default
brain Brain;


// START V5 MACROS
#define waitUntil(condition)                                                   \
do {                                                                         \
wait(5, msec);                                                             \
} while (!(condition))

#define repeat(iterations)                                                     \
for (int iterator = 0; iterator < iterations; iterator++)
// END V5 MACROS


// Robot configuration code.
motor backLeft = motor(PORT1, ratio36_1, false);

motor backRight = motor(PORT2, ratio36_1, true);

motor front = motor(PORT3, ratio18_1, false);

inertial mpu = inertial(PORT6);

distance distanceLeft = distance(PORT11);
distance distanceRight = distance(PORT12);
optical colorLeft = optical(PORT16);
optical colorRight = optical(PORT17);
optical colorFront = optical(PORT18);
controller Controller1 = controller(primary);



// Helper to make playing sounds from the V5 in VEXcode easier and
// keeps the code cleaner by making it clear what is happening.
void playVexcodeSound(const char *soundName) {
  printf("VEXPlaySound:%s\n", soundName);
  wait(5, msec);
}



// define variable for remote controller enable/disable
bool RemoteControlCodeEnabled = true;

#pragma endregion VEXcode Generated Robot Configuration
/*----------------------------------------------------------*/
/*                                                          */
/*    Module:       AutonCar.cpp                            */
/*    Author:       Jining Liu                              */
/*    Created:      04/26/2024                              */
/*    Description:  Autonomous Race Car for POE             */
/*                                                          */
/*----------------------------------------------------------*/

#include "vex.h"

using namespace vex;

// decides which distance sensor to use
bool useRightSensor = true;

// decides distance between car and wall in mm
const int distanceFromWall = 1000;

// decides the intensity of system keep straight adjustments
const double pidMultiplier = 3;

// decides initial speed of back motors
const int initLeftMotor = 50, initRightMotor = 80;
const int leftMotorMin = 5, rightMotorMin = 10;
const int leftMotorMax = 90, rightMotorMax = 100;
int leftMotor = initLeftMotor, rightMotor = initRightMotor;

// DO NOT CHANGE: stores distance sensor data, default 0
int lastLeft, currentLeft, lastRight, currentRight;

// DO NOT CHANGE: stopwatch for race timing in ms, default 0
int raceTime;

// required to predefine functions??? vex version of cpp is dumb... declarations down below
void pidDistanceStraight();
void executeTurn(int i, int delay);
void finalLeg();
void abort();
void pasueResume();
void stop();
void debugPrint();
void preparePrintBig(int y, int x, bool clear);

// auton turns programming
// timing is automatic
//
// make sure numberOfTurns is the same as the length of carTurns & preTurnDelay!
// memory allocation is required!!!
//
// key:
// -1 - left turn
// 0 - keep straight
// 1 - right turn
const int numberOfTurns = 6;
const int carTurns[numberOfTurns] = {0, 1, 0, 1, 1, -1};
// this sets the seconds of delay BEFORE executing the turn or continuing on
const int preTurnDelay[numberOfTurns] = {4, 1, 4, 1, 1, 1};
// this sets the seconds of delay AFTER executing the turn or continuing on
const int postTurnDelay[numberOfTurns] = {0, 2, 0, 2, 2, 2};

bool paused = false;
bool aborted = false;

int main() {
  
  preparePrintBig(2, 1, true);
  Brain.Screen.print("Initializing...");
  
  Controller1.ButtonX.pressed(abort);
  Controller1.ButtonR1.pressed(pasueResume);
  
  mpu.calibrate();
  mpu.setRotation(0, degrees);
  
  // wait(1, seconds);
  // preparePrintBig(2, 1, true);
  // Brain.Screen.print("Hello!");
  // wait(2, seconds);
  // preparePrintBig(2, 1, true);
  // Brain.Screen.print("Starting in...");
  // wait(1, seconds);
  // preparePrintBig(2, 1, true);
  // Brain.Screen.print("5");
  // wait(1, seconds);
  // preparePrintBig(2, 1, true);
  // Brain.Screen.print("4");
  // wait(1, seconds);
  // preparePrintBig(2, 1, true);
  // Brain.Screen.print("3");
  // wait(1, seconds);
  // preparePrintBig(2, 1, true);
  // Brain.Screen.print("2");
  // wait(1, seconds);
  // preparePrintBig(2, 1, true);
  // Brain.Screen.print("1");
  // wait(1, seconds);
  preparePrintBig(2, 1, true);
  Brain.Screen.print("Currently:");
  
  // record start time
  raceTime = Brain.Timer.time(msec);
  
  backLeft.spin(forward);
  backRight.spin(forward);
  
  for (int i = 0; i < numberOfTurns; i++) {
    
    stop();
    
    Brain.Screen.clearLine(3);
    preparePrintBig(3, 1, false);
    Brain.Screen.print("Straightaway");
    preparePrintBig(3, 14, false);
    Brain.Screen.print(i + 1);
    
    // if car is detecting wall where it's supposed to be, always loop pid
    while ((useRightSensor && currentRight < 2000) ||
           (!useRightSensor && currentLeft < 2000)) {
      stop();
      pidDistanceStraight();
      wait(200, msec);
    }
    
    Brain.Screen.clearLine(3);
    preparePrintBig(3, 1, false);
    Brain.Screen.print("Turn");
    preparePrintBig(3, 6, false);
    Brain.Screen.print(i + 1);
    
    // keep going delay before turn
    wait(preTurnDelay[i], seconds);
    
    executeTurn(carTurns[i], postTurnDelay[i]);
  }
  
  Brain.Screen.clearLine(3);
  preparePrintBig(3, 1, false);
  Brain.Screen.print("Final Leg!");
  
  finalLeg();
}

// predefine for pid, this is so dumb
void updateDistance();
void pidShiftLeft();
void pidShiftRight();

// pid for back motors keeping straight and correct distance from wall
void pidDistanceStraight() {
  
  updateDistance();
  
  if (useRightSensor) {
    // using right sensor
    if (currentRight > distanceFromWall) {
      // too far from wall
      pidShiftRight();
    } else if (currentRight < distanceFromWall) {
      // too close to wall
      pidShiftLeft();
    } else {
      // keep straight
      if (currentRight > lastRight) {
        pidShiftRight();
      } else if (currentRight < lastRight) {
        pidShiftLeft();
      }
    }
  } else {
    // using left sensor
    if (currentLeft > distanceFromWall) {
      // too far from wall
      pidShiftLeft();
    } else if (currentLeft < distanceFromWall) {
      // too close to wall
      pidShiftRight();
    } else {
      // keep straight
      if (currentLeft > lastLeft) {
        pidShiftLeft();
      } else if (currentLeft < lastLeft) {
        pidShiftRight();
      }
    }
  }
  
  debugPrint();
  
  backLeft.setVelocity(leftMotor, percent);
  backRight.setVelocity(rightMotor, percent);
}

void pidShiftLeft() {
  if (leftMotor > leftMotorMin) {
    leftMotor -= pidMultiplier;
  }
  if (rightMotor < rightMotorMax) {
    rightMotor += pidMultiplier;
  }
}

void pidShiftRight() {
  if (leftMotor < leftMotorMax) {
    leftMotor += pidMultiplier;
  }
  if (rightMotor > rightMotorMin) {
    rightMotor -= pidMultiplier;
  }
}

// updates last & current distance
void updateDistance() {
  lastLeft = currentLeft;
  currentLeft = distanceLeft.objectDistance(mm);
  lastRight = currentRight;
  currentRight = distanceRight.objectDistance(mm);
}

// turn execution based on previous instructions from for each loop & carTurns array
void executeTurn(int i, int delay) {
  
  mpu.setRotation(0, degrees);
  
  if (i == -1) {
    
    // turn left
    while (mpu.rotation(degrees) > -90) {
      stop();
      leftMotor = 10;
      rightMotor = 100;
    }
    
    leftMotor = initLeftMotor;
    rightMotor = initRightMotor;
    
    // change to detect left wall, wait 1s for wall to spawn in
    useRightSensor = false;
    wait(delay, seconds);
  } else if (i == 1) {
    
    // turn right
    while (mpu.rotation(degrees) < 90) {
      stop();
      leftMotor = 100;
      rightMotor = 10;
    }
    
    leftMotor = initLeftMotor;
    rightMotor = initRightMotor;
    
    // change to detect right wall, wait 1s for wall to spawn in
    useRightSensor = true;
    wait(delay, seconds);
  }
}

// simple stuff
// change to detect tape soon
void finalLeg() {
  leftMotor = 80;
  rightMotor = 100;
  wait(1, seconds);
  leftMotor = 100;
  rightMotor = 80;
  wait(1, seconds);
  rightMotor = 100;
  wait(2, seconds);
  backLeft.stop();
  backRight.stop();
  
  // calculate & display race time
  raceTime = Brain.Timer.time(msec) - raceTime;
  
  preparePrintBig(2, 1, true);
  Brain.Screen.print("Finished!");
  preparePrintBig(4, 1, false);
  Brain.Screen.print("Final Time:");
  
  preparePrintBig(5, 1, false);
  Brain.Screen.print(raceTime / 1000 / 60);
  Brain.Screen.print(":");
  Brain.Screen.print(raceTime / 1000 % 60);
}

// safety abort
void abort() {
  
  aborted = true;
  
  preparePrintBig(2, 1, true);
  Brain.Screen.print("SAFETY");
  preparePrintBig(3, 1, false);
  Brain.Screen.print("KILLSWITCH");
  
  // hard brake
  backLeft.setVelocity(-100, percent);
  backRight.setVelocity(-100, percent);
  
  wait((leftMotor + rightMotor) * 4, msec);
  
  while (true) {
    leftMotor = 0;
    rightMotor = 0;
    backLeft.stop();
    backRight.stop();
  }
}

// self explanatory
void pasueResume() {
  paused = !paused;
}

// stop loop
void stop() {
  while (aborted || paused) {
    wait(15, msec);
  }
}

// debug printing
//
// val 1: left distance
// val 2: right distance
// val 3: left motor velocity
// val 4: right motor velocity
//
void debugPrint() {
  Brain.Screen.setFont(prop20);
  Brain.Screen.clearScreen();
  Brain.Screen.setCursor(1, 1);
  Brain.Screen.print(currentLeft);
  Brain.Screen.setCursor(1, 7);
  Brain.Screen.print(currentRight);
  Brain.Screen.setCursor(1, 13);
  Brain.Screen.print(leftMotor);
  Brain.Screen.setCursor(1, 19);
  Brain.Screen.print(rightMotor);
}

// makes preparing for printing in big font easy
//
// how to use:
// preparePrintBig(3, 1, true);
//          ^  ^     ^
//         /    \     L --> clear screen before print?
//        /      \
//   row #        column #
//
void preparePrintBig(int y, int x, bool clear) {
  if (clear) {
    Brain.Screen.clearScreen();
  }
  Brain.Screen.setCursor(y, x);
  Brain.Screen.setFont(prop60);
}
