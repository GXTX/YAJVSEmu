/*
    YAJVSEmu
    ----------------
    Copyright (C) 2020-2021 wutno (https://github.com/GXTX)


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "GpIo.h"

GpIo::GpIo(SenseType sense_type)
{
	if (bcm2835_init()) {
		IsInitialized = true;
		bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT);
		std::cout << "GpIo::Init: Initialized sense line." << std::endl;
	}
	else if (sense_type != SenseType::None) {
		IsInitialized = false;
		std::cerr << "GpIo::Init: Failed to initalize the bcm2835 lib." << std::endl;
	}
}

GpIo::~GpIo()
{
	bcm2835_close();
}

void GpIo::SetMode(PinMode state) 
{
	if (state == PinMode::In) {
		bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT);
#ifdef DEBUG_GPIO
		std::cout << "GpIo::TogglePin: Toggled pin to IN." << std::endl;
#endif
	}
	else if (state == PinMode::Out) {
		bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);
#ifdef DEBUG_GPIO
		std::cout << "GpIo::TogglePin: Toggled pin to OUT." << std::endl;
#endif
	}
}

void GpIo::Write(PinState state)
{
	if (state == PinState::Low) {
		bcm2835_gpio_write(PIN, LOW);
#ifdef DEBUG_GPIO
		std::cout << "GpIo::Write: Grounded sense line." << std::endl;
#endif
	}
	else if (state == PinState::High) {
		bcm2835_gpio_write(PIN, HIGH);
#ifdef DEBUG_GPIO
		std::cout << "GpIo::Write: Pullingup the sense line." << std::endl;
#endif
	}
}
