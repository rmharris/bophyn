These are some of the source files for a temperature sensing application based on the ATmega168 (Arduino).

A base station connects to up to six remote sensors using RFM70 radio transceivers driven by a [custom library](https://github.com/rmharris/rfm70).  On reset, each sensor signals its identity by blinking a green LED.  Every five minutes thereafter, it reads the temperature from a TI TMP102 and transmits it to the base;  every two hours the reading is accompanied by the battery voltage.  Power consumption is reduced to a minimum by shutting down all components and using the ATmega168's watchdog timer to wake as needed.  Battery life is between six and twelve months indoors.

![alt text](https://github.com/rmharris/bophyn/blob/master/transmitter.jpg)
