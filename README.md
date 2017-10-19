# WiFi-MBus

This project is ripoff from
[MBusMqttLogger](https://github.com/roarfred/MBusMqttLogger]).
I disabled everything except MBus although unused code is still
here.

Idea of my modification is wireless reading of a kWh meter with
minimum client requrements.

ESP8266 will read kWh count over MBus every minute and if the reading is
different than previous one, it will update its WiFi AP name 
(Access Point ESSID Name) with the reading. For example, an AP with 
this name will appear:

    ESSID: "1,2 00001.234 kWh"

And when some kWh pass by, it will change AP name into something like:

    ESSID: "1,2 00002.345 kWh"

So any wifi capable phone or tablet can be used as readout device,
just view a list of available AP's. No client applications, no browsers,
no web interface not even connecting to any AP is needed.

# BUGS

There must be plenty. I've seen it crashed occasionaly and watchdog
rebooted it correctly. So far it somehow avoids executing infinite loop,
but I'll update this README with further problems.

I had to rush to get this thing  up and installed, so it's done
in a quick and dirty way.
