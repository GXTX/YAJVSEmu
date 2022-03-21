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

//#define DEBUG_GPIO

GpIo::GpIo(SenseType sense_type)
{
	this->sense_type = sense_type;

	if (bcm2835_init()) {
		IsInitialized = true;
		bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT);
		std::puts("GpIo::Init: Initialized sense line.");
	} else if (sense_type != SenseType::None) {
		IsInitialized = false;
		std::cerr << "GpIo::Init: Failed to initalize the bcm2835 lib.\n";
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
		std::puts("GpIo::SetMode: Toggled pin to IN.");
#endif
	} else if (state == PinMode::Out) {
		bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);
#ifdef DEBUG_GPIO
		std::puts("GpIo::SetMode: Toggled pin to OUT.");
#endif
	}
}

void GpIo::Write(PinState state)
{
	if ((state == PinState::Low && sense_type == SenseType::Float) || (state == PinState::High && sense_type == SenseType::Sink)) {
		bcm2835_gpio_write(PIN, LOW);
#ifdef DEBUG_GPIO
		std::puts("GpIo::Write: Grounded sense line.");
#endif
	} else if ((state == PinState::High && sense_type == SenseType::Float) || (state == PinState::Low && sense_type == SenseType::Sink)) {
		bcm2835_gpio_write(PIN, HIGH);
#ifdef DEBUG_GPIO
		std::puts("GpIo::Write: Pulling up the sense line.");
#endif
	}
}
