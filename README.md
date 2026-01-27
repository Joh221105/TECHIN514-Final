# Bloop
An interactive desk companion featuring a magnetically-controlled ferrofluid that responds to hand gestures (and potentially audio commands in the future). The creature exhibits lifelike behaviors through computer vision and wireless control. Core Behaviors:

Follow - Tracks and follows user's face position
Hide - Rotates away from user (shy behavior)
Spin - Runs around the track once

![Sketch of the overview of overall product](./assets/Overview.png)

### Physical Features
A circular acrylic platform (12" diameter) contains ferrofluid in carved tracks. Beneath the platform, a stepper motor controls an arm with an embedded magnet rotates to manipulate the fluid. An LED ring provides ambient lighting and behavioral feedback.

# Sensor Device

### ESP32-C3
The sensing device uses an ESP32C3 with a SEN0359-EN voice recognition module to detect command words and move the stepper motor accordingly to make the ferrofluid perform the "tricks".

The device is powered by a 150mAh 3.7V LiPo battery with USB charging, making it portable and easily repositionable. 

![Sketch of sensing device](./assets/Sensing.png)

# Display Device

### Magnetically-Actuated Ferrofluid Platform
The display device features a laser-cut circular acrylic platform with ferrofluid in carved tracks. A NEMA 17 stepper motor beneath the platform rotates a vertical arm assembly, positioning a neodymium magnet 1-2mm under the ferrofluid. The TMC2208 driver provides silent operation with microstepping for smooth, organic motion.

An ESP32-C3 microcontroller receives wireless commands and controls motor movements. A WS2812B LED ring (16 LEDs) provides visual feedback with color-coded patterns. A capacitive touch pad enables power control, and an 11.1V 2200mAh LiPo battery provides power and allows portability.

As the magnet rotates, the ferrofluid forms dramatic spikes that follow the magnetic field, creating lifelike tracking behaviors that respond to user movements detected by the camera.

![Sketch of the display device](./assets/Display.png)

# Communication Between Devices

![Systems Diagram](./assets/System.png)
![Flow chart](./assets/Flow.png)

![Electrical Diagram](./assets/Electrical%20Schematic.png)