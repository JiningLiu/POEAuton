# Changelog

This file contains the change in features of the codebase, sorted from newest to oldest.

The link in the main version headers will always be to the latest update of the version.

## [v2](https://github.com/JiningLiu/POEAuton/commit/main) - 05/06-08/2024

### [We Kinda Cooked Update (3)](https://github.com/JiningLiu/POEAuton/commit/main) - 05/07-08/2024
- updated distance from wall to 200mm
- updated pid multiplier to 2
- updated min & max motor values (slow down & overturn prevention)
- fixed distance sensor issue (override first 9999s)
- super rotation calibration
- fixed rotational axis
- changed wall detection values to 9999
- fixed brain print clear line locations
- commented out useless (for now) code
- fixed pid shifting not matching up
- fixed turn execution incorrect angles
- fixed debug print issues
- aborting no longer removes debug print
- fixed (probably) pause & resume (didn't test)
- added rotation to debug print
- formatting changes

### [Update 2](https://github.com/JiningLiu/POEAuton/commit/baa1e351fadf846500ed028510a2107c0c9f50e7) - 05/06/2024
- fixed debug printout font size
- updated pid multiplier to 3

### [Update 1](https://github.com/JiningLiu/POEAuton/commit/5a80527c4e6215378f155d03325cc5934c398d6d) - 05/06/2024
- re-added debug printouts
- added pause & resume

### [WE DID NOT COOK Update](https://github.com/JiningLiu/POEAuton/commit/3124846a58d50c54e08308b1250ff0690fb1b0d5) - 05/06/2024
- removed line #'s (too much work to maintain)
- distance from wall is now 1m
- added motors min & max values
- added skip turns
- added programmable pre & post turn delays
- removed start countdown
- increased loop interval delay to 200ms
- updated logic for keeping straight
- updated abort motors rollback
- updated abort to enter while true loops everywhere

## [v1](https://github.com/JiningLiu/POEAuton/commit/db1937d011ea49b6ce23679627ae04f3af062d49) - 05/02-03/2024

### [Update 3](https://github.com/JiningLiu/POEAuton/commit/db1937d011ea49b6ce23679627ae04f3af062d49) - 05/03/2024
- Added currently running printout

### [Update 2](https://github.com/JiningLiu/POEAuton/commit/901bbd5ba94785034ba136148fb59a09fca4c409) - 05/03/2024
- Added timer/stopwatch

### [Update 1](https://github.com/JiningLiu/POEAuton/commit/b3a288d5724b21327f699367a96826deec5e8f81) - 05/02/2024
- Fixed code line #'s

### [Basic Auton DT](https://github.com/JiningLiu/POEAuton/commit/04332562dae87584d8580f70a218971d385aaf95) - 05/02/2024
- keep straight (parallel with left or right wall)
- correct to specific distance away from wall
- easily adjustable pid multiplier
- easily programmable turns by just changing a single array
- auto turning sequence w/ correction delay
- manual safety killswitch via controller
