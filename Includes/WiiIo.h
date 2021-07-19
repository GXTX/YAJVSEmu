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

#ifndef WIIIO_H
#define wIIIO_H

#include <iostream>
#include <cmath>
#include <cstring>
#include <chrono>
#include <thread>

#include <unistd.h>
#include <poll.h>

#include <xwiimote.h>

#include "JvsIo.h"

typedef struct {
	int id;
	int fd;
	std::string controller;
	xwii_iface *interface;
} wiiremote;

class WiiIo
{
public:
	WiiIo(int players, jvs_input_states_t *jvsInputs);
	~WiiIo();

	void Loop();
private:
	enum MovementValueType {
		Analog,
		ScreenPos,
	};

	jvs_input_states_t *Inputs;
	std::vector<wiiremote> Player;
	xwii_event event;

	void ButtonPressHandler(int player, xwii_event_key *button, xwii_iface *fd);
	void IRMovementHandler(int player, xwii_event_abs *ir, MovementValueType type);
};

#endif
