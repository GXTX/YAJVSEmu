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

#ifndef SERIO_H
#define SERIO_H

#include <iostream>
#include <vector>

#include <libserialport.h>

class SerIo
{
public:
	enum StatusCode {
		Okay,
		Timeout,
		ReadError,
		WriteError,
	};

	bool IsInitialized;

	SerIo(char *devicePath);
	~SerIo();

	int Read(std::vector<uint8_t> &buffer);
	int Write(std::vector<uint8_t> &buffer);
private:
	int SerialHandler;

	struct sp_port *Port;
	struct sp_port_config *PortConfig;
};

#endif
