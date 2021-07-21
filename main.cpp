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
#include "GpIo.h"
#include "SdlIo.h"
#include "WiiIo.h"
#include "version.h"

#ifdef REAL_TIME
// If we don't delay longer we'll starve the other processes of CPU time.
auto delay{std::chrono::microseconds(25)};
#else
auto delay{std::chrono::microseconds(5)};
#endif

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
	std::printf("%s: %s - %s (Build Date: %s %s)\n", PROJECT_NAME,
#ifdef NDEBUG
"Release",
#else
"Debug",
#endif
	GIT_VERSION, __DATE__, __TIME__);

#ifdef REAL_TIME
	// Set thread priority to RT. We don't care if this
	// fails but may be required for some systems.
	sched_param params{sched_get_priority_max(SCHED_FIFO)};
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &params);
#endif

	// Handle quitting gracefully via signals
	std::signal(SIGINT, sig_handle);
	std::signal(SIGTERM, sig_handle);

	std::unique_ptr<GpIo> GPIOHandler (std::make_unique<GpIo>(GpIo::SenseType::Float));
	if (!GPIOHandler->IsInitialized) {
		std::cerr << "Couldn't initalize GPIO controller and 'NONE' wasn't explicitly set.\n";
		return 1;
	}

	// TODO: probably doesn't need to be shared? we only need Inputs to be a shared ptr
	std::shared_ptr<JvsIo> JVSHandler (std::make_shared<JvsIo>(JvsIo::SenseState::NotConnected));

	std::unique_ptr<SerIo> SerialHandler (std::make_unique<SerIo>(dev.c_str()));
	if (!SerialHandler->IsInitialized) {
		std::cerr << "Coudln't initiate the serial controller.\n";
		return 1;
	}

	// Spawn lone SDL2 or WiiIo input thread.
	// NOTE: There probably is no reason we can't have both of these running
	//if (setup.sdlbackend) {
		//std::thread(&SdlIo::Loop, std::make_unique<SdlIo>(0, &JVSHandler->Inputs)).detach();
	//}
	//else {
		//std::thread(&WiiIo::Loop, std::make_unique<WiiIo>(1, &JVSHandler->Inputs)).detach();
	//}

	JvsIo::Status jvsStatus;
	SerIo::Status serialStatus;

	std::vector<uint8_t> SerialBuffer{};

	while (running) {
		if (!SerialBuffer.empty()) {
			SerialBuffer.clear();
		}

		serialStatus = SerialHandler->Read(SerialBuffer);
		if (serialStatus != SerIo::Status::Okay) {
			std::this_thread::sleep_for(delay);
			continue;
		}

		jvsStatus = JVSHandler->ReceivePacket(SerialBuffer);

		if(JVSHandler->pSenseChange){
			if(JVSHandler->pSense == JvsIo::SenseState::NotConnected) {
				GPIOHandler->SetMode(GpIo::PinMode::In);
			} else {
				GPIOHandler->SetMode(GpIo::PinMode::Out);
				GPIOHandler->Write(GpIo::PinState::Low);
			}
			JVSHandler->pSenseChange = false;
		}

		if (jvsStatus == JvsIo::Status::Okay || jvsStatus == JvsIo::Status::SumError) {
			jvsStatus = JVSHandler->SendPacket(SerialBuffer);
			SerialHandler->Write(SerialBuffer);
		}

		std::this_thread::sleep_for(delay);
	}

	return 0;
}
