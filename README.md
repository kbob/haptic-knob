# haptic-knob

A knob with configurable haptic feedback using a brushless DC motor.

I have the idea that a brushless DC motor (BLDC) with a high resolution
absolute position sensor can be made into a knob with haptic feedback.

# Hardware

I have a brushless BLDC controller eval board from ST Microelectronics:
STEVAL-IHM043V1.  It has an STM32F051 micro controller and an L6234
H bridge.

I've got a BlackMagic Probe to debug and reflash the STM32.

I've got breakout boards for AMS AM5047 and AM5048 magnetic position
sensors.  I don't know which one I want to use.

And I've got a motor.  It's a chonky fellow from an old e-bike project.
Total overkill for this project, but it was in the parts bin.

I printed a little test fixture to hold all the pieces.  I've got
everything except the position sensor wired up and minimally tested.

# Future Hardware

There are many kinds of camera gimbal motors on Aliexpress.
The smallest are around 15mm diameter.  Those would be ideal,
but I don't know

# Firmware plans

I'll be using libopencm3, so I'll be reinventing a lot of wheel.
First goal is to get a basic app loaded that can blink the LED
and/or print something to the serial port.

Then I can start adding features like reading the analog measurement
points on the motor driver.

Next, make the motor move by applying slow PWM sine waves to the
motor coils.

If I can make it rotate, then I can start trying to read the position
using the AM504x chips.
