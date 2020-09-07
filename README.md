# The Heisenbox

A demonstration of Heisenberg's Uncertainty Principle, over-engineered for an astrophysics project.

Controlled by a MSP430G2553, though most microcontrollers in that series should work; this isn't particularly demanding. Uses a TM1637 7-segment display and a H-bridge to demonstrate this principle, switched by a reed switch.
When activated, it will run the demonstration for around a minute before entering LPM4, that is, essentially shut down the microcontroller. The 'start' button is connected to the reset pin, so pressing that will reset the MSP430 and remove it from LPM4. 

The code ain't optimized nor fancy, but hey, it works and does what it needs to.

The TM1637 driver is based off of [lpodkalicki's ATtiny library](https://github.com/lpodkalicki/attiny-tm1637-library), with slight modifications to make it work on a MSP430 processor. 

Read more about this project [here](https://reidsoxharris.me/projects/heisenbox). 
