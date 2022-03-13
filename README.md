# Humming Bird Pulse Sensor

Hi.
This project is also on [Hackaday.io](https://hackaday.io/project/184261-hummingbird-heartbeats)

Adding files as they become publishable (read: workish)

## Basic Shape
What you have here is a Teensy 3.6 that reads 5 analog inputs at a very high rate and stores the data to SD card. 

-	Sensors
	- Pulse Sensor 2x
	- Analog Accelerometer

The signal of interest is likely in the hundreds of hertz. Here's a ball park:

	Hummingbird Heart Rate 800 Beats Per Minute (BPM)
	BPM = 60 Seconds / Inter-beat Interval (IBI)

the IBI value is the time between beats. the IBI associated with a heart rate of 800 BPM is 75 milliseconds (0.075 Seconds). That 800 BPM number I just pulled out of the air. It could be faster, based on the liturature. So we need a high sample rate. 

I'm starting with a Teensy 3.6 and the [ExFatLogger.ino](https://github.com/greiman/SdFat/tree/master/examples/ExFatLogger) example to get high speed SD card writing. The basic Arduino code is [there](https://github.com/biomurph/DINACON_Hummingbird_Heartbeats/tree/2022/2022/Arduino/humm_beat_sensor_01).

## Hardware
Will not work if SD card is not installed. Unplug device, install SD card, plug in device.

## User Guide

Outline of use. 
The interface allows you to access and manipulate binary files. 
Hardware is powered via USB, and requires a connected Terminal or Data Visualizer to begin function. When you plug in the USB *without Terminal or Data Visualizer* the BLUE LED will be steady on. Once the port is connected, the BLUE LED will pulse/fade nicely saying it is ready.

- Record sensor data to a binary file
- List all the files in memory
- Open an existing binary file
- Convert a binary file and save it as a .csv file
- Convert a binary file and print it to the serial port
- Read sensor data without logging

It is recommended to follow a specific series of steps to use the interface efficiently. Though it is not possible to screw it up really bad, it can be confusing without this guide. Here are the steps for success:

1. Insert a micro SC card that is formatted as ExFat (super important)
2. Open up a serial port visualizer window
3. Plug into USB computer
4. other stuff
5. After copying the data you want off the micro SD, make sure to empty your trash on the computer before unmounting the disk.

	

# Below is readme from 2019

Helping [Jay Falk](https://www.birdmorph.com/) to measure Humming Bird Heartbeats with [Pulse Sensor](www.pulsesensor.com)
Jay is a scientist who studies hummingbirds in Gamboa, Panama. It was recently discovered that hummingbirds (some, not all?) can vocalize, that is make sounds, up to 15KHz. At the same time, it is generally believed that birds can only hear sounds up to 8KHz. Jay wants to know if hummingbirds can hear in the higher registers that they make sounds in. His thought is that maybe the birds heart rate changes when they hear another bird call, and that if he can measure heart rate this might be a way to prove that they can hear in the higher registers.

I have been making and selling Pulse Sensor, an optical heart rate monitor, since 2012. There happen to be a couple of Pulse Sensors at the Dinalab, and so very soon after I arrived at Dinacon, I got together with Jay to try and see if we could find a hummingbird heart beat with Pulse Sensor. 

The Pulse Sensor uses the principal of [Photoplethysmography](https://en.wikipedia.org/wiki/Photoplethysmogram) to measure heart beats. Here is a page that discribes the [features](https://www.researchgate.net/figure/A-typical-waveform-of-the-PPG-and-its-characteristic-parameters-whereas-the-amplitude-of_fig9_230587653) of the waveform that we're looking for.

Hummigbirds have a couple of 'bald spots' where hair doen't grow. Typically, most birds don't have feathers on their lower abdomen in order to make a good connection to their eggs when incubating them. Also, Hummingbirds have no feathers right behind their crop on the back of their heads. We decided we would try both of those spots.

I used two Pulse Sensors. One was just a normal Pulse Sensor, and the other I modified by taking off the LED. I did this in order to test a reflective Pulse Sensor (the way they normally work) and a transmissive Pulse Sensor, where I used the light from the normal Pulse Sensor to send a signal through the body of the bird. It became clear very soon that the reflective Pulse Sensor was the best option.

We did se a signal when we placed the Pulse Sensor on the belly bald spot, however it was difficult to tell if we were reading the heartbeat or the breathing. Placing the Pulse Sensor on the bald spot behind the crop was much more successful, and we think we did see a heartbeat in that location.

Here is a [LINK](https://www.dropbox.com/sh/dgatqx7pgcjl1rp/AABGw35ZrMt65FjiA4dyHX-6a?dl=0) to a dropbox that has videos from the experiment.
