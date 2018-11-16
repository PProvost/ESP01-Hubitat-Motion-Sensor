# ESP01 Hubitat Contact Sensor

This project is derived from and inspired by many excellent examples found on the web, including (but not limited to):

1. https://github.com/DanielOgorchock/ST_Anything
2. https://github.com/debsahu/ESP_External_Interrupt
3. http://www.forward.com.au/pfod/ESP8266/GPIOpins/ESP8266_01_pin_magic.html
4. https://randomnerdtutorials.com/modifying-cheap-pir-motion-sensor-to-work-at-3-3v/

## Getting started

* This project was developed using Platformio and therefore uses that project structure. If you want to use the
  Arduino IDE instead, you will have to move some files around, install some libraries manually, etc.
  
* This project will NOT build as-is! I store my local WiFi information in a separate file outside of the project
  directory. You will need to copy src/ap_setting_h.example to the directory above the project directory. Edit
  it to match your local WiFi settings and you should be good to go. Look at the top of main.cpp for more info.

* You will also need to edit the IPAddress variables at the top of main.cpp to match your configuration. Each
  device needs to be given its own unique static IP address (faster==lower power consumption).

* When setting up the device in Hubitat, you will need to provide the static IP address, device port (8090),
  and the device MAC address. There are properties for the first two, but the MAC address must be entered into
  the "Device Network Id" field in Hubitat. The easiest way to get the MAC address is from the serial output 
  after you flash the device and before you install it.

* I've included a folder called "Hubitat Driver" that contains the Device Driver code you will need to import into
  your home automation hub. There are plenty of online resources explaining how to do that, so I'll not get into
  that here.

## Building the circuit

Please refer to the schematic PDF and the other files in the schematic/ folder. I'm working on a board you will be
able to order from OSHPark, but until I have confirmed it works, you might be better off building it by hand on 
perf or strip board.

Once the board is confirmed working, I will also design a 3D printed enclosure for it.

## Implementation Notes

* The circuit is designed to be powered off of one lithium ion (LiPo) battery, and uses a low
  QI voltage regulator to provide the 3V3 expected by the ESP8266. This requires some special
  attention with your PIR sensor. Most of the cheap ones you will find expect 5V. See below for
  more information about modding your PIR sensor for 3V3.

* Because the ESP01 requires GPIO00 and GPIO02 to be HIGH when powering up, they're not appropriate
  for use as GPIO inputs. To get around this, I reused RXD as a GPIO pin as described in ESP8266-01 Pin Magic [3].
  This means you can't use serial commands to talk TO the device, but it can still send serial logging
  to the serial monitor.

* My circuit is essentally the same as the one described in [2] above, with the exception that I used RXD instead of 
  GPIO12 (which isn't broken out on the ESP-01). Also, I found that the pulldown resistor connected after the diode
  needed to be 100k ohm instead of 10k ohm. I think this is because I'm running the PIR at 3v3 instead of 5v, so the
  voltage coming out of the signal line is lower than in the original design. When I used 10k, I wasn't able to get
  the ESP to reliably wake up. YMMV.

* I wanted to reuse the VCC pin on the PIR sensor to make mounting easier, but also to power the PIR sensor
  with the same 3.3VDC the ESP takes. I made the following modifications to the PIR sensor:
  - Removed the 3V3 voltage regulator
  - Connected the cathode (output) side of the diode to the VOUT pad where the regulator had been connected
  - Removed the 1k resistor between the controller IC and the signal pin and shorted the two pads.
  - NOTE:This permanently changes the sensor to only take 3.3V, do not power it with 5V after removing the regulator.
    If you don't want to remove the regulator from your sensor, you can still power it with 3.3V following the guide 
    above [4], but you may still need to remove the 1k resistor.


