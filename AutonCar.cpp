

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
motor backLeft = motor(PORT1, ratio18_1, false);

motor backRight = motor(PORT2, ratio18_1, false);

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
/*    Important Code Line #'s:                              */
/*        User-adjustable values -------- 87, 90, 93, 96    */
/*        DO NOT CHANGE values ----------------- 99, 102    */
/*        Turns programming ------------------- 120, 121    */
/*        Main code exec block --------------------- 123    */
/*        PID Controller --------------------------- 203    */
/*        Turn Executor ---------------------------- 282    */
/*        Final Leg -------------------------------- 318    */
/*        Safety Abort ----------------------------- 345    */
/*        Macro Functions -------------------------- 366    */
/*                                                          */
/*----------------------------------------------------------*/

#include "vex.h"

using namespace vex;

// decides which distance sensor to use
bool useRightSensor = true;

// decides distance between car and wall in mm
const int distanceFromWall = 100;

// decides the intensity of system keep straight adjustments
const double pidMultiplier = 1;

// decides speed of back motors
int leftMotor = 95, rightMotor = 95;

// DO NOT CHANGE: stores distance sensor data, default 0
int lastLeft, currentLeft, lastRight, currentRight;

// DO NOT CHANGE: stopwatch for race timing in ms, default 0
int raceTime;

// required to predefine functions??? vex version of cpp is dumb... declarations down below
void pidDistanceStraight();
void executeTurn(int i);
void finalLeg();
void abort();
void preparePrintBig(int y, int x, bool clear);

// auton turns programming
// timing is automatic
//
// make sure numberOfTurns is the same as the length of carTurns!
// memory allocation is required!!!
//
// key:
// 0 - left turn
// 1 - right turn
const int numberOfTurns = 4;
const int carTurns[numberOfTurns] = {1, 1, 1, 0};

int main() {
  
  preparePrintBig(2, 1, true);
  Brain.Screen.print("Initializing...");
  
  Controller1.ButtonX.pressed(abort);
  
  mpu.calibrate();
  mpu.setRotation(0, degrees);
  
  wait(1, seconds);
  preparePrintBig(2, 1, true);
  Brain.Screen.print("Hello!");
  wait(2, seconds);
  preparePrintBig(2, 1, true);
  Brain.Screen.print("Starting in...");
  wait(1, seconds);
  preparePrintBig(2, 1, true);
  Brain.Screen.print("5");
  wait(1, seconds);
  preparePrintBig(2, 1, true);
  Brain.Screen.print("4");
  wait(1, seconds);
  preparePrintBig(2, 1, true);
  Brain.Screen.print("3");
  wait(1, seconds);
  preparePrintBig(2, 1, true);
  Brain.Screen.print("2");
  wait(1, seconds);
  preparePrintBig(2, 1, true);
  Brain.Screen.print("1");
  wait(1, seconds);
  preparePrintBig(2, 1, true);
  Brain.Screen.print("Currently Running:");
  
  // record start time
  raceTime = Brain.Timer.time(msec);
  
  backLeft.spin(forward);
  backRight.spin(forward);
  for (int i = 0; i < numberOfTurns; i++) {
    
    Brain.Screen.clearLine(3);
    preparePrintBig(3, 1, false);
    Brain.Screen.print("Straightaway");
    preparePrintBig(3, 14, false);
    Brain.Screen.print(i + 1);
    
    // if car is detecting wall where it's supposed to be, always loop pid
    while (
           (useRightSensor && currentRight < (distanceFromWall + 50))
           ||
           (!useRightSensor && currentLeft < (distanceFromWall + 50))
           ) {
             pidDistanceStraight();
             wait(50, msec);
           }
    
    Brain.Screen.clearLine(3);
    preparePrintBig(3, 1, false);
    Brain.Screen.print("Turn");
    preparePrintBig(3, 6, false);
    Brain.Screen.print(i + 1);
    
    executeTurn(carTurns[i]);
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
    if (currentRight - 1 > distanceFromWall) {
      // too far from wall
      pidShiftRight();
    } else if (currentRight + 1 < distanceFromWall) {
      // too close to wall
      pidShiftLeft();
    } else {
      // keep straight
      if (currentRight - 1 > lastRight) {
        pidShiftRight();
      } else if (currentRight + 1 < lastRight) {
        pidShiftLeft();
      }
    }
  } else {
    // using left sensor
    if (currentLeft - 1 > distanceFromWall) {
      // too far from wall
      pidShiftLeft();
    } else if (currentLeft + 1 < distanceFromWall) {
      // too close to wall
      pidShiftRight();
    } else {
      // keep straight
      if (currentLeft - 1 > lastLeft) {
        pidShiftLeft();
      } else if (currentLeft + 1 < lastLeft) {
        pidShiftRight();
      }
    }
  }
  
  // debug printing
  //
  // line 1: left distance
  // line 2: right distance
  //
  // line 4: left motor velocity
  // line 5: right motor velocity
  //
  // Brain.Screen.clearScreen();
  // Brain.Screen.setCursor(1, 1);
  // Brain.Screen.print(currentLeft);
  // Brain.Screen.setCursor(2, 1);
  // Brain.Screen.print(currentRight);
  // Brain.Screen.setCursor(4, 1);
  // Brain.Screen.print(leftMotor);
  // Brain.Screen.setCursor(5, 1);
  // Brain.Screen.print(rightMotor);
  
  backLeft.setVelocity(leftMotor, percent);
  backRight.setVelocity(rightMotor, percent);
}

void pidShiftLeft() {
  leftMotor -= pidMultiplier;
  rightMotor += pidMultiplier;
}

void pidShiftRight() {
  leftMotor += pidMultiplier;
  rightMotor -= pidMultiplier;
}

// updates last & current distance
void updateDistance() {
  lastLeft = currentLeft;
  currentLeft = distanceLeft.objectDistance(mm);
  lastRight = currentRight;
  currentRight = distanceRight.objectDistance(mm);
}

// turn execution based on previous instructions from for each loop & carTurns array
void executeTurn(int i) {
  
  mpu.setRotation(0, degrees);
  
  if (i == 0) {
    
    // turn left
    while (mpu.rotation(degrees) > -90) {
      leftMotor = -20;
      rightMotor = 100;
    }
    
    leftMotor = 80;
    rightMotor = 80;
    
    // change to detect left wall, wait 1s for wall to spawn in
    useRightSensor = false;
    wait(1, seconds);
  } else {
    
    // turn right
    while (mpu.rotation(degrees) < 90) {
      leftMotor = 100;
      rightMotor = -20;
    }
    
    leftMotor = 80;
    rightMotor = 80;
    
    // change to detect right wall, wait 1s for wall to spawn in
    useRightSensor = true;
    wait(1, seconds);
  }
}

// simple stuff
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
  
  preparePrintBig(2, 1, true);
  Brain.Screen.print("SAFETY");
  preparePrintBig(3, 1, false);
  Brain.Screen.print("KILLSWITCH");
  
  // hard brake
  leftMotor = -100;
  rightMotor = -100;
  
  wait(100, msec);
  
  while (true) {
    leftMotor = 0;
    rightMotor = 0;
    backLeft.stop();
    backRight.stop();
  }
}

// MARK: Macro functions

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
