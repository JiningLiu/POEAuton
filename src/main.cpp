
#pragma region VEXcode Generated Robot Configuration

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "vex.h"

using namespace vex;

brain Brain;

// robot devices onfiguration
motor backLeft = motor(PORT1, ratio36_1, false);
motor backRight = motor(PORT2, ratio36_1, true);

inertial mpu = inertial(PORT6);

distance distanceLeft = distance(PORT11);
distance distanceRight = distance(PORT12);
distance distanceFront = distance(PORT13);

optical colorLeft = optical(PORT16);
optical colorRight = optical(PORT17);

controller Controller1 = controller(primary);

bool RemoteControlCodeEnabled = true;

#pragma endregion VEXcode Generated Robot Configuration

/*----------------------------------------------------------*/
/*                                                          */
/*    Module:       main.cpp                                */
/*    Author:       Jining Liu                              */
/*    Created:      04/26/2024                              */
/*    Description:  Autonomous Race Car for POE             */
/*                                                          */
/*----------------------------------------------------------*/

#include "vex.h"

using namespace vex;

// decides which distance sensor to use
bool useRightSensor = true;

// decides distance between car and wall in mm, this is a range
const int minDistanceFromWall = 250, maxDistanceFromWall = 350;

// decides the intensity of system keep straight adjustments
const double leftPidMultiplier = 1, rightPidMultiplier = 2;

// decides initial speed of back motors
const int initLeftMotor = 34, initRightMotor = 50;
const int leftMotorMin = 31, rightMotorMin = 44;
const int leftMotorMax = 37, rightMotorMax = 56;
int leftMotor = initLeftMotor, rightMotor = initRightMotor;

// DO NOT CHANGE: stores distance sensor data, default 0
int lastLeft, currentLeft, lastRight, currentRight, currentFront;

// DO NOT CHANGE: stopwatch for race timing in ms, default 0
int raceTime;

// required to predefine functions??? vex version of cpp is dumb... declarations down below
void updateDistance();
void pidDistanceStraight();
void executeTurn(int i, int delay);
void finalLeg();
void abort();
void pauseResume();
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
const int preTurnDelay[numberOfTurns] = {3, 1, 2, 1, 1, 1};
// this sets the seconds of delay AFTER executing the turn or continuing on
const int postTurnDelay[numberOfTurns] = {0, 2, 0, 2, 2, 2};

// stupid vex don't ask
bool overrideFirst9999 = true;

bool paused = false;
bool aborted = false;

int main()
{

  preparePrintBig(2, 1, true);
  Brain.Screen.print("Initializing...");

  Controller1.ButtonX.pressed(abort);
  Controller1.ButtonR1.pressed(pauseResume);

  // super inertial sensor calibration
  for (int i = 0; i < 100; i++)
  {
    mpu.calibrate();
    mpu.setHeading(0, degrees);
    wait(15, msec);
  }

  wait(1, seconds);

  preparePrintBig(2, 1, true);
  Brain.Screen.print("Currently:");

  // record start time
  raceTime = Brain.Timer.time(msec);

  for (int i = 0; i < numberOfTurns; i++)
  {

    stop();

    mpu.setHeading(0, degrees);

    preparePrintBig(3, 1, false);
    Brain.Screen.clearLine(3);
    Brain.Screen.print("Straightaway");
    preparePrintBig(3, 14, false);
    Brain.Screen.print(i + 1);
    Brain.Screen.clearLine(4);

    // if car is detecting wall where it's supposed to be, always loop pid
    while ((useRightSensor && !(currentRight >= 9999 && lastRight < 9999)) ||
           (!useRightSensor && !(currentRight >= 9999 && lastLeft < 9999)))
    {
      stop();
      pidDistanceStraight();
      wait(100, msec);
    }

    preparePrintBig(3, 1, false);
    Brain.Screen.clearLine(3);
    Brain.Screen.print("Intersection");
    preparePrintBig(3, 14, false);
    Brain.Screen.print(i + 1);
    preparePrintBig(4, 1, false);
    switch (carTurns[i])
    {
    case -1:
      Brain.Screen.print("Left Turn");
      break;
    case 0:
      Brain.Screen.print("Keep Straight");
      break;
    case 1:
      Brain.Screen.print("Right Turn");
      break;
    }

    // keep going delay before turn
    wait(preTurnDelay[i], seconds);

    executeTurn(carTurns[i], postTurnDelay[i]);
  }

  preparePrintBig(3, 1, false);
  Brain.Screen.clearLine(3);
  Brain.Screen.print("Final Leg!");

  finalLeg();
}

// predefine for all below, this is so dumb
void setVelocity();

// predefine for pid, this is so dumb
void pidShiftLeft();
void pidShiftRight();

// pid for back motors keeping straight and correct distance from wall
void pidDistanceStraight()
{

  updateDistance();

  if (useRightSensor)
  {
    // using right sensor
    if (currentRight > maxDistanceFromWall)
    {
      // too far from wall
      pidShiftRight();
    }
    else if (currentRight < minDistanceFromWall)
    {
      // too close to wall
      pidShiftLeft();
    }
    else
    {
      // keep straight
      if (currentRight > lastRight)
      {
        pidShiftRight();
      }
      else if (currentRight < lastRight)
      {
        pidShiftLeft();
      }
    }
  }
  else
  {
    // using left sensor
    if (currentLeft > maxDistanceFromWall)
    {
      // too far from wall
      pidShiftLeft();
    }
    else if (currentLeft < minDistanceFromWall)
    {
      // too close to wall
      pidShiftRight();
    }
    else
    {
      // keep straight
      if (currentLeft > lastLeft)
      {
        pidShiftLeft();
      }
      else if (currentLeft < lastLeft)
      {
        pidShiftRight();
      }
    }
  }

  debugPrint();

  setVelocity();
}

void pidShiftLeft()
{
  if (leftMotor >= leftMotorMin + leftPidMultiplier && rightMotor <= rightMotorMax - rightPidMultiplier)
  {
    leftMotor -= leftPidMultiplier;
    rightMotor += rightPidMultiplier;
  }
}

void pidShiftRight()
{
  if (leftMotor <= leftMotorMax - leftPidMultiplier && rightMotor >= rightMotorMin + rightPidMultiplier)
  {
    leftMotor += leftPidMultiplier;
    rightMotor -= rightPidMultiplier;
  }
}

// updates last & current distance
void updateDistance()
{

  if (overrideFirst9999)
  {
    do
    {
      debugPrint();
      currentLeft = distanceLeft.objectDistance(mm);
      currentRight = distanceRight.objectDistance(mm);
      wait(15, msec);
    } while ((currentLeft >= 9999 && lastLeft >= 9999) || (currentRight >= 9999 && lastRight >= 9999));
    overrideFirst9999 = false;

    backLeft.spin(forward);
    backRight.spin(forward);
  }

  lastLeft = currentLeft;
  currentLeft = distanceLeft.objectDistance(mm);
  lastRight = currentRight;
  currentRight = distanceRight.objectDistance(mm);
}

// turn execution based on previous instructions from for each loop & carTurns array
void executeTurn(int i, int delay)
{

  mpu.setHeading(0, degrees);

  if (i == -1)
  {

    // turn left
    while (mpu.heading(degrees) > 270 || mpu.heading(degrees) < 180)
    {
      stop();
      debugPrint();
      leftMotor = 10;
      rightMotor = 100;
      setVelocity();
      wait(15, msec);
    }

    leftMotor = initLeftMotor;
    rightMotor = initRightMotor;

    // change to detect left wall, wait 1s for wall to spawn in
    useRightSensor = false;
    wait(delay, seconds);
  }
  else if (i == 1)
  {

    // turn right
    while (mpu.heading(degrees) < 90 || mpu.heading(degrees) > 180)
    {
      stop();
      debugPrint();
      leftMotor = 100;
      rightMotor = 10;
      setVelocity();
      wait(15, msec);
    }

    leftMotor = initLeftMotor;
    rightMotor = initRightMotor;
    setVelocity();

    // change to detect right wall, wait 1s for wall to spawn in
    useRightSensor = true;
    wait(delay, seconds);
  } else {
    leftMotor = initLeftMotor;
    rightMotor = initRightMotor;
    setVelocity();
  }
}

// simple stuff
// change to detect tape soon
void finalLeg()
{
  leftMotor = 80;
  rightMotor = 100;
  setVelocity();
  wait(1, seconds);
  leftMotor = 100;
  rightMotor = 80;
  setVelocity();
  wait(1, seconds);
  rightMotor = 100;
  setVelocity();
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

// set motor velocity to variable values
void setVelocity()
{
  backLeft.setVelocity(leftMotor, percent);
  backRight.setVelocity(rightMotor, percent);
}

void frontImpale()
{
  currentFront = distanceFront.objectDistance(mm);
  if (currentFront < 200)
  {
    abort();
  }
}

// safety abort
void abort()
{
  aborted = true;
  preparePrintBig(2, 1, false);
  Brain.Screen.clearLine(2);
  Brain.Screen.print("SAFETY");
  preparePrintBig(3, 1, false);
  Brain.Screen.clearLine(3);
  Brain.Screen.print("KILLSWITCH");

  // hard brake
  backLeft.setVelocity(-100, percent);
  backRight.setVelocity(-100, percent);

  wait((leftMotor + rightMotor) * 4, msec);

  while (true)
  {
    leftMotor = 0;
    rightMotor = 0;
    backLeft.stop();
    backRight.stop();
  }
}

// self explanatory
void pauseResume()
{
  paused = !paused;
}

// stop loop
void stop()
{
  frontImpale();
  if (aborted || paused)
  {
    backLeft.stop(hold);
    // backLeft.setVelocity(-100, percent);
    // backRight.setVelocity(-100, percent);

    // wait((leftMotor + rightMotor) * 4, msec);

    const int lastLeftMotor = leftMotor;
    const int lastRightMotor = rightMotor;

    while (aborted || paused)
    {
      backLeft.setVelocity(0, percent);
      backRight.setVelocity(0, percent);
      backLeft.stop();
      backRight.stop();
      wait(15, msec);
    }

    leftMotor = lastLeftMotor;
    rightMotor = lastRightMotor;
    setVelocity();
    backLeft.spin(forward);
    backRight.spin(forward);
  }
}

// debug printing
//
// val 1: left distance
// val 2: right distance
// val 3: left motor velocity
// val 4: right motor velocity
// val 5: front distance
// val 6: gyro
//
void debugPrint()
{
  Brain.Screen.setFont(prop20);
  Brain.Screen.clearLine(1);
  Brain.Screen.setCursor(1, 1);
  Brain.Screen.print(currentLeft);
  Brain.Screen.setCursor(1, 7);
  Brain.Screen.print(currentRight);
  Brain.Screen.setCursor(1, 13);
  Brain.Screen.print(leftMotor);
  Brain.Screen.setCursor(1, 19);
  Brain.Screen.print(rightMotor);
  Brain.Screen.setCursor(1, 25);
  Brain.Screen.print(currentFront);
  Brain.Screen.setCursor(1, 31);
  Brain.Screen.print(mpu.heading(degrees));
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
void preparePrintBig(int y, int x, bool clear)
{
  if (clear)
  {
    Brain.Screen.clearScreen();
  }
  Brain.Screen.setCursor(y, x);
  Brain.Screen.setFont(prop60);
}