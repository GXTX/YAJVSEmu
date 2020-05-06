YetAnotherJVSEmu
============

YetAnotherJVSEmu is a program which aims to emulate the SEGA JVS protocol using mostly off-the-shelf solutions, and a way for me to play around with new things. Supports *many many* different types of controllers and wheels thanks to using SDL2 as the backend for Joystick and Gamepad support, have a controller or device that isn't mapped? Checkout [this project for more information](https://github.com/gabomdq/SDL_GameControllerDB "this project for more information"). Just plop the file in the same directory as the executable and it'll load the configuration from it. Also supports using a Wii IR controller as a X/Y gun.

Building
---------
##### Prerequisites

```sh
sudo apt install build-essential cmake libsdl2-dev libxwiimote-dev
```

*Note: You are also required to have BCM2835 installed, you can find this [here](https://www.airspayce.com/mikem/bcm2835/ "here").*

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

Prior to use you must have a USB to RS485 adapter installed on your Pi, and JVS cable wired up like this.

[![wiring diagram](https://gist.githubusercontent.com/GXTX/d771608fb2dd0944c6d944dbf041acaf/raw/a1c453d78f1f51953c67901f4135050ef18d9d31/wiring_diagram.png "wiring diagram")](https://gist.githubusercontent.com/GXTX/d771608fb2dd0944c6d944dbf041acaf/raw/a1c453d78f1f51953c67901f4135050ef18d9d31/wiring_diagram.png "wiring diagram")

The program *must* be ran as root. There is also **zero** prior configuration required, just run and the program will ask you information about your environment.
```
sudo ./YetAnotherJVSEmu
```

