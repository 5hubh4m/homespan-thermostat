# HomeSpan Thermostat

HomeKit connected smart thermostat based on Arduino using [HomeSpan](https://github.com/HomeSpan/HomeSpan).

## Background and Motivation

I have a 24 V Heat-only system with a 2-wire system (No C wire, only R and W). It used a manual
Honeywell single-point thermostat. I wanted to get the functionalities of a HomeKit compatible
smart thermostat without the hassle of installing a C wire myself and/or having to buy an
expensive smart thermostat. So I created my own with a Wi-Fi capable microcontroller.

## Hardware

I used a [Adafruit ESP32-S2 Feather](https://www.adafruit.com/product/5000) board, along with
a [TMP36](https://www.adafruit.com/product/165) analog temperature sensor, and a
[simple non-latching relay](https://www.adafruit.com/product/4409).

I wired up the TMP36 to an analog input pin (`A5` or `ADC1_CHANNEL_7`) and the relay to a digital pin (`10`),
and connected the relay to the R and W wires in my heating panel in the `NO` mode i.e when the relay is
`off`, the heater is also `off`. The circuit diagram is given below.

![This image shows how the components are wired to the microcontroller board.](assets/thermostat.png "Wiring")

## Flashing the Code on the Microcontroller

Use the Arduino IDE to flash the code to the microcontroller.

## Configuring

There are a few parameters that can be configured.

The Wi-Fi credentials can be defined in the source code or can be set using
the `W` command of the
[HomeSpan CLI](https://github.com/HomeSpan/HomeSpan/blob/master/docs/CLI.md).

The HomeKit pairing code can be defined in the source code. Otherwise the
default code `466-37-726` is used or it can be set using the `S` command of the
[HomeSpan CLI](https://github.com/HomeSpan/HomeSpan/blob/master/docs/CLI.md).

The HomeKit QR Setup ID can be defined in the source code. Otherwise the
default QR Setup ID `HSPN` is used or it can be set using the `Q` command of the
[HomeSpan CLI](https://github.com/HomeSpan/HomeSpan/blob/master/docs/CLI.md).

## Using

Pair the accessory to HomeKit. The general instructions are given
[here](https://github.com/HomeSpan/HomeSpan/blob/master/docs/HomeSpanUserGuide.pdf).

That's pretty much it. If you're connected to HomeKit, you should be able to control your heater
using Siri.

![This image shows how the this thermostat looks in the Home app.](assets/home.png "Home")
