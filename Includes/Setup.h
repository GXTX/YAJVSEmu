/*
    YAJVSEmu
    ----------------
    Copyright (C) 2020-2022 wutno (https://github.com/GXTX)


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

#ifndef SETUP_H
#define SETUP_H

#include <iostream>
#include <algorithm>
#include <vector>

#include "SdlIo.h"
#include "WiiIo.h"

class SetupInfo
{
public:
	struct SetupInfos{
		enum Backend {
			None,
			SDL,
			XWII,
		};
		int players{1};
		Backend backend;
		int device_index{0}; // SDL device
		std::string device_path{}; // XWII path
		std::string serial_port{};
	};

	enum class SetupStatus {
		Okay,
		Failed,
	};

	bool IsFinished{false};

	SetupInfos info;

	SetupInfo();
private:
	SetupStatus UserInput();
	SetupStatus SDLAsk();
	SetupStatus XWIIAsk();
};

#endif
