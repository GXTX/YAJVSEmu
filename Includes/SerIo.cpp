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

#include "SerIo.h"

//#define DEBUG_SERIAL

SerIo::SerIo(const char *devicePath)
{
	sp_new_config(&PortConfig);
	sp_set_config_baudrate(PortConfig, 115200);
	sp_set_config_bits(PortConfig, 8);
	sp_set_config_parity(PortConfig, SP_PARITY_NONE);
	sp_set_config_stopbits(PortConfig, 1);
	sp_set_config_flowcontrol(PortConfig, SP_FLOWCONTROL_NONE);

	sp_get_port_by_name(devicePath, &Port);

	sp_return ret = sp_open(Port, SP_MODE_READ_WRITE);

	if (ret != SP_OK) {
		std::cerr << "SerIo::Init: Failed to open " << devicePath << "\n";
		IsInitialized = false;
	} else {
		sp_set_config(Port, PortConfig);
		IsInitialized = true;
	}
}

SerIo::~SerIo()
{
	sp_close(Port);
}

SerIo::Status SerIo::Write(std::vector<uint8_t> &buffer)
{
	// This shouldn't happen...
	if (buffer.empty()) {
		return Status::ZeroSizeError;
	}

#ifdef DEBUG_SERIAL
	std::cout << "SerIo::Write:";
	for (uint8_t c : buffer) {
		std::printf(" %02X", c);
	}
	std::cout << "\n";
#endif

	int ret = sp_nonblocking_write(Port, &buffer[0], buffer.size());

	if (ret <= 0) {
		return Status::WriteError;
	} else if (ret != static_cast<int>(buffer.size())) {
#ifdef DEBUG_SERIAL
		std::cerr << "SerIo::Write: Only wrote " << std::hex << ret << " of " << std::hex << static_cast<int>(buffer.size()) << " to the port!\n";
#endif
		return Status::WriteError;
	}

	return Status::Okay;
}

SerIo::Status SerIo::Read(std::vector<uint8_t> &buffer)
{
	int bytes = sp_input_waiting(Port);

	if (bytes == 0) {
		return Status::ZeroSizeError;
	} else if (bytes < 0) {
		return Status::ReadError;
	} else if (bytes < 5) {
		return Status::ReadError; // TODO: Dirty hack
	}

	buffer.resize(static_cast<size_t>(bytes));

	int ret = sp_nonblocking_read(Port, &buffer[0], buffer.size());

	if (ret <= 0) {
		return Status::ReadError;
	}

#ifdef DEBUG_SERIAL
	std::cout << "SerIo::Read:";
	for (uint8_t c : buffer) {
		std::printf(" %02X", c);
	}
	std::cout << "\n";
#endif

	return Status::Okay;
}
