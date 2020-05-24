/*
    YetAnotherJVSEmu
    ----------------
    Copyright (C) 2020 wutno (https://github.com/GXTX)


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

#include <SDL.h>
#include <SDL_stdinc.h>

#include "JvsIo.h"

class SdlIo
{
public:
	jvs_input_states_t *Inputs;

	SdlIo(jvs_input_states_t *jvs_inputs, int device_index);
	~SdlIo();
	void Loop();
private:
	enum AxisType {
		Stick,
		Trigger,
	};

	SDL_GameController *sgc;
	SDL_Event event;

	void ButtonPressHandler(SDL_ControllerButtonEvent *button);
	void AxisMovementHandler(SDL_ControllerAxisEvent *axis);
	uint16_t ScaledAxisMovement(int16_t value, AxisType type);
};

#endif
