# iSpindel-Sim

An ESP8266-based [iSpindel](https://iSpindel.de/) Hydrometer Simulator for HTTP endpoints.

This project is based on the excellent project https://github.com/spouliot/tilt-sim. 

## Description

Useful for developing/debugging iSpindel clients and tools. A single ESP8266 device can simulate multiple (16) iSpindel devices.

The device will also show a simple HTML page when accessed. For example:

```bash
# use Bonjour (mDNS) to open the web page and have it refresh every 5 seconds
$ open http://ispindel-sim.local./?refresh=5
```

Using the simple REST API you can script, e.g. several `curl` calls, a (condensed) fermentation session to test your software.

```bash
# turn all tilts off - they are all active by default
$ curl "http://ispindel-sim.local/set?name=*&active=off"
# acticate the purple tilt
$ curl "http://ispindel-sim.local/set?name=iSpindel4&active=on&angle=25.1&sg=1.2001&temp=65.1"
```

## Build

You can build this project using [VSCode](https://code.visualstudio.com) and [PlatformIO](https://platformio.org).

You'll need to define `SSID`, `PASSWORD` and `HTTP_ENDPOINT` to match your environment.
