# Esp8266-Nexa-Transmitter
Arduino code for esp8266 to send Nexa remote controllable switches and dimmers.

If you want to use this program change ssid and password to match your wifi network
and use serial monitor to see what is the ip address of the device.

Url command order is Mode,Unit(0-15),Controller address,Dim level,Transmit repeat times
Example On,0,10646597,0,4 Or Off,0,10646597,0,4 When dimming Dim,1,10646597,7,4 Or DimOff,1,10646597,0,4 Dimming level is 0-1
  
If you want to teach new switch or dimmer use (Teach,"0-15","8 number code",0) = (Teach,0,12345678,0)

Its better to use random 8 digit number as code for controller address because if you use 12345678 there is a chance that someone else has the same code

Example url: http://192.168.11.10/On,0,10646597,0,4
