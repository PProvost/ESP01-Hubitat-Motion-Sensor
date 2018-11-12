# ESP01 Hubitat Contact Sensor

This project is derived from and inspired by many excellent examples found on the web, including (but not limited to):

1. https://github.com/DanielOgorchock/ST_Anything
2. https://github.com/debsahu/ESP_External_Interrupt
3. http://www.forward.com.au/pfod/ESP8266/GPIOpins/ESP8266_01_pin_magic.html
4. https://randomnerdtutorials.com/modifying-cheap-pir-motion-sensor-to-work-at-3-3v/

## Implementation Notes

* This project was developed using Platformio and therefore uses that project structure. If you want to use the
  Arduino IDE instead, you will have to move some files around, install some libraries manually, etc.
  
* Because the ESP01 requires GPIO00 and GPIO02 to be HIGH when powering up, they're not appropriate
  for use in this scenario. To get around this, I reused RXD as a GPIO pin as described in ESP8266-01 Pin Magic [3].

* My circuit is essentally the same as the one described in [2] above, with the exception that I used RXD instead of 
  GPIO12 (which isn't broken out on the ESP-01). Also, I found that the pulldown resistor connected after the diode
  needed to be 100k ohm instead of 10k ohm. I think this is because I'm running the PIR at 3v3 instead of 5v, so the
  voltage coming out of the signal line is lower than in the original design. When I used 10k, I wasn't able to get
  the ESP to reliably wake up. YMMV.

* I wanted to reuse the VCC pin on the PIR sensor to make mounting easier, but also to power the PIR sensor
  with the same 3.3VDC the ESP takes. I made the following modifications to the PIR sensor:
  - Removed the 3V3 voltage regulator
  - Connected the cathode (output) side of the diode to the VOUT pad where the regulator had been connected
  - Removed the 1k resistor between the controller IC and the signal pin.
  - NOTE:This permanently changes the sensor to only take 3.3V, do not power it with 5V after removing the regulator.
    See below for instructions on making this change. If you don't want to make this mod to your sensor, you
    can still power it with 3.3V following the guide above [4].

* I've included a folder called "Hubitat Driver" that contains the Device Driver code you will need to import into
  your home automation hub. There are plenty of online resources explaining how to do that, so I'll not get into
  that here.

