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

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <csignal>

#include "JvsIo.h"
#include "SerIo.h"


//auto delay{std::chrono::microseconds(250)};
auto delay{std::chrono::milliseconds(5)};
std::atomic<bool> running{true};

void sig_handle(int sig) {
	if (sig == SIGINT || sig == SIGTERM) {
		running = false;
	}
}

// TODO: Replace with ini setup
static const std::string dev{"/dev/ttyUSB0"};

int main()
{
	// Handle quitting gracefully via signals
	std::signal(SIGINT, sig_handle);
	std::signal(SIGTERM, sig_handle);

	std::shared_ptr<JvsIo> JVSHandler (std::make_shared<JvsIo>(JvsIo::SenseState::Connected));

	std::unique_ptr<SerIo> SerialHandler (std::make_unique<SerIo>(dev.c_str()));
	if (!SerialHandler->IsInitialized) {
		//std::cerr << "Coudln't initiate the serial controller.\n";
		//return 1;
	}

	JvsIo::Status jvsStatus;
	SerIo::Status serialStatus;

	std::vector<uint8_t> SerialBuffer{};
	SerialBuffer.reserve(512);

	while (running) {
		for (int i = 0; i < 0xFFFF; i += 8) {
			std::this_thread::sleep_for(delay);
			if (!SerialBuffer.empty()) {
				SerialBuffer.clear();
			}
			JVSHandler->SendPacket(SerialBuffer, i);
			SerialHandler->Write(SerialBuffer);
			while (true) {
				serialStatus = SerialHandler->Read(SerialBuffer);
				if (serialStatus != SerIo::Status::Okay) {
					std::this_thread::sleep_for(delay);
					continue;
				}
				break;
			}
			jvsStatus = JVSHandler->ReceivePacket(SerialBuffer, i);
			if (jvsStatus != JvsIo::Status::Okay) {
				std::cerr << "Killing at dec: " << i << "\n";
				running = false;
				break;
			}
		}

		running = false;
	}

	return 0;
}
