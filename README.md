YetAnotherJVSEmu
============

YetAnotherJVSEmu is a program which aims to emulate the SEGA JVS protocol using mostly off-the-shelf solutions, and a way for me to play around with new things. Supports *many many* different types of controllers and wheels thanks to using SDL2 as the backend for Joystick and Gamepad support, have a controller or device that isn't mapped? Checkout [this project for more information](https://github.com/gabomdq/SDL_GameControllerDB "this project for more information"). Just plop the file in the same directory as the executable and it'll load the configuration from it. Also supports using a Wii IR controller as a X/Y gun.

Building
---------
##### Prerequisites

Ubuntu 20.04

```sh
sudo apt install build-essential cmake pkg-config libsdl2-dev libxwiimote-dev libserialport-dev
```

*Important: You are also required to have BCM2835 installed, you can find this [here](https://www.airspayce.com/mikem/bcm2835/ "here").*

*Note: It is also assumed you've properly created the gpio group and others as required. Checkout [this](https://wiki.ubuntu.com/ARM/RaspberryPi/ "this") for more information.*

##### Build
```
git clone https://github.com/GXTX/YetAnotherJVSEmu
cd YetAnotherJVSEmu
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Running
---------

Prior to use you must have a USB to RS485 adapter installed on your Pi, and JVS cable wired up like this. The diodes used in this example are 4 1N4148's. The voltage across ground and the GPIO pin should be 2.5V with your arcade system running and YetAnother not started.

[![wiring diagram](https://gist.githubusercontent.com/GXTX/d771608fb2dd0944c6d944dbf041acaf/raw/a1c453d78f1f51953c67901f4135050ef18d9d31/wiring_diagram.png "wiring diagram")](https://gist.githubusercontent.com/GXTX/d771608fb2dd0944c6d944dbf041acaf/raw/a1c453d78f1f51953c67901f4135050ef18d9d31/wiring_diagram.png "wiring diagram")

The program *must* be ran as root, and must be running before turning on your system. There is also **zero** prior configuration required, just run and the program will ask you information about your environment.
```
sudo ./YetAnotherJVSEmu
```

