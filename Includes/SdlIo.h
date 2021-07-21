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

#ifndef SDLIO_H
#define SDLIO_H

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

#include <SDL.h>

#include "JvsIo.h"

class SdlIo
{
public:
	SdlIo(int deviceIndex, jvs_input_states_t *jvsInputs);
	~SdlIo();

	void Loop();
private:
	enum class AxisType {
		Stick,
		Trigger,
	};

	jvs_input_states_t *Inputs{nullptr};
	SDL_GameController *controller{nullptr};
	SDL_Event event;

	void ButtonPressHandler(SDL_ControllerButtonEvent *button);
	void AxisMovementHandler(SDL_ControllerAxisEvent *axis);
	uint16_t ScaledAxisMovement(int16_t value, AxisType type);
};

#endif
