# emg-processing
Work in progress about EMG processing

## What this is 

A collection of modules for retrieving the signals from our EMG sensors (Delsys), do basic processing, streaming the data on a Yarp network (or ROS network), than use the processed signals for HRI experiments.

If you are going to use these modules, please cite us:
```
Ivaldi, S.; Fritzsche, L.; Babic, J.; Stulp, F.; Damsgaard, M.; Graimann, B.; Bellusci, G.; Nori, F. (2017) Anticipatory models of human movements and dynamics: the roadmap of the AnDy project. Proc. International Conf. on Digital Human Models (DHM).
```

## Hardware Requirements

* Delsys Wireless EMG Sensors, model Trigno
* iCub robot

## Software Requirements

* Yarp
* icub-main (only if you want to compile also the modules for controlling iCub)
* icub-contrib 

## Installation

```
git clone https://github.com/inria-larsen/emg-processing
cd emg-processing
mkdir build
cd build
ccmake ..
make
make install
```

## How to run the modules

### Concept

There is a server module, `EMGserver` that is connected to the TCP server of the Delsys. This module makes a first online filtering of the signals.
The module `EMGhuman` is used to retrieve the signals (raw or filtered), selecting those that are actually used by the human, and computing some quantities (stiffnes, ICC, effort..).
The module `EMGhuman2robot` is used to retrieve the computed quantities and update the robot's motion/compliance level according to some pre-defined policies.

TODO IMAGE

### Setup and launch of the Delsys server

TODO.

### From terminal

On terminal 1: `EMGserver`

On terminal 2: `EMGhuman —from human_operator.ini`

On terminal 3: `EMGhuman2robot —from emg_operator2robot.ini`

If you want the modules to automatically connect, use the `--autoconnect option` or set it inside the configuration files. There are numerous ports that are created by the modules. For connecting the ports one by one, please follow the XML template.

### From the yarpmanager

Open a `yarpmanager`, import the XML file, click on "run" then "connect". 


## Calibration

The module `EMGhuman` needs a calibration phase. 

TODO

## Acknowledgments

The development of this software is partially supported by [the European Project H2020 An.Dy](http://andy-project.eu/).
We thank [Trinoma](http://trinoma.fr/ "Trinoma webpage") for their support with the Delsys sensors.


