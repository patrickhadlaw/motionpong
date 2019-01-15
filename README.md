# motionpong

An motion activated pong game running on the Onion Omega micro-controller built for the final project of my introduction programming course.
Features:
* Motion control for pong game, simply move hands in front of ultrasonic sensors to control paddles
* Draws game on Onion Omega 2 display as seen in screenshots below

![Concept](/docs/concept.png?raw=true "Concept")

## Liscensing

### This project is licensed under the provided license - see the [LICENSE](LICENSE) file for details

## Authors

* **Patrick Hadlaw** - [patrickhadlaw](https://github.com/patrickhadlaw)

## Build information

### To build motion-pong you need to:
* Setup cross-compiling to Onion Omega 2
* Build using the makefile provided
* Setup ultrasonic sensors using voltage divider circuit shown in report with R1 = 1k Ohm, R2 = 2k Ohm to GPIO pins defined in ultrasonic.h
* Connect Onion Omega OLED to correct pins using cables
* rsync the executable file over to the Omega and then you can run it

## Report

### [Click here](/docs/Project Report.pdf) for a report on how motion-pong works and was created

## Screenshots

![Screenshot1](/docs/screenshot1.jpg?raw=true "Screenshot1")
![Screenshot2](/docs/screenshot2.jpg?raw=true "Screenshot2")
![Screenshot3](/docs/screenshot3.jpg?raw=true "Screenshot3")
