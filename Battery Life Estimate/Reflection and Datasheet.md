# Sensing Device

### How did you determine your "days of use" metric?
The days of use are based on an estimate of a realistic daily usage scenario for an interactive pet toy. I estimated the device spends 20 hours in off mode/deep sleep, 2.5 hours in sensing mode (listening for wake word and voice commands spread throughout the day), and 1.5 hours in interactive mode (actively sending BLE commands to the display device after recognizing a command). This models someone interacting with the pet a few times throughout the day in short sessions. 

### What do you think is the optimum battery size?
A 500mAh 3.7V LiPo is a good fit, it is enough to push the usage to over 24 hours and some spare just in case usage is higher than expected. It is also compact and light enough to make sense for the sensing device. 

### Hardware/software/cost/effort tradeoffs to improve UX?
One tradeoff would be adding a simple analog audio sensor that detects sound energy before powering on the voice module, this way the module only draws power when someone is actually speaking nearby, rather than continuously listening, which then means we can decrease the size of the battery and weight. On the software side, implementing smarter I2C polling intervals could reduce the ESP32-C3's active duty cycle during sensing mode, which would cut processor active time significantly with only a slight delay in responsiveness. For BLE, using longer connection intervals would reduce radio power during interactive mode, but can slow down command time.
<hr>

# Display Device

### How did you determine your "days of use" metric?
The display device follows a 19.5 hours off, 2.5 hours in sensing (awake and listening for BLE commands), and 2 hours in interactive mode (motor movements, LED effects, and capacitive touch sensor). The interactive time is longer because the motor movements and LED animations take time to execute. For instance a single "Run a Lap" command might run the stepper motor for 10-15 seconds. 

### What do you think is the optimum battery size?
The OVONIC 3S 2200mAh 11.1V LiPo is good enough because the TMC2208 stepper driver and NEMA 17 motor perform best at higher voltages for torque and speed, which would be required to move the magnet on top of the gauge needle. The bulkiness and weight of the battery doesn’t matter as much since the display device is meant to be stationary. 

### Hardware/software/cost/effort tradeoffs to improve UX?
The biggest tradeoff is the stepper motor power consumption. TMC2208 gives silent operation which is great for a pet toy, however other drivers might be more efficient at the cost of louder motor movements.The LED brightness could also be decreased with minimal impact on user experience, however it can also decrease power consumption. A smaller stepper motor would use less power but might not have enough torque to reliably move the aluminum arm through the ferrofluid.

# Data Sheets

Processor: https://files.seeedstudio.com/wiki/XIAO_WiFi/Resources/esp32-c3_datasheet.pdf

Sensor
https://wiki.dfrobot.com/sku_sen0539-en_gravity_voice_recognition_module_i2c_uart

Motor: 
https://www.omc-stepperonline.com/download/17HS19-2004S1.pdf
