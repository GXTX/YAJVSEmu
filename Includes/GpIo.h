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

#ifndef GPIO_H
#define GPIO_H

#include <iostream>

#include <bcm2835.h>

#define PIN RPI_BPLUS_GPIO_J8_32

class GpIo
{
public:
	enum class SenseType{
		None,
		Float,
	};

	enum class PinMode{
		In,
		Out,
	};

	enum class PinState{
		Low,
		High,
	};

	bool IsInitialized;

	GpIo(SenseType sense_type);
	~GpIo();

	void SetMode(PinMode state);
	void Write(PinState state);
private:
};

#endif
