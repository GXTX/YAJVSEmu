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

volatile std::atomic<bool> running = true;

void sig_handle(int sig) {
	switch (sig) {
		case SIGINT:
		case SIGTERM:
			running = false;
			break;
		default: break;
	}
}

// TODO: Replace with ini setup
static char *dev = "/dev/ttyS0";

int main()
{
	std::cout << PROJECT_NAME << ": ";
#ifdef NDEBUG
std::cout << "Release - ";
#else
std::cout << "Debug - ";
#endif
	std::cout << GIT_VERSION << " (" << __DATE__ << " " << __TIME__ << ")" << std::endl;

	// Set thread priority to RT. We don't care if this
	// fails but may be required for some systems.
	struct sched_param params;
	params.sched_priority = sched_get_priority_max(SCHED_FIFO);
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &params);

	// Handle quitting via signals
	std::signal(SIGINT, sig_handle);

	std::unique_ptr<GpIo> GPIOHandler (std::make_unique<GpIo>(GpIo::SenseType::Float));
	if (!GPIOHandler->IsInitialized) {
		std::cerr << "Couldn't initiate GPIO and \"NONE\" wasn't explicitly set." << std::endl;
		return 1;
	}

	// TODO: probably doesn't need to be shared? we only need Inputs to be a shared ptr
	std::shared_ptr<JvsIo> JVSHandler (std::make_shared<JvsIo>(JvsIo::SenseStates::NotConnected));

	std::unique_ptr<SerIo> SerialHandler (std::make_unique<SerIo>(dev));
	if (!SerialHandler->IsInitialized) {
		std::cerr << "Coudln't initiate the serial controller." << std::endl;
		return 1;
	}

	// Spawn lone SDL2 or WiiIo input thread.
	// NOTE: There probably is no reason we can't have both of these running
	//if (setup.sdlbackend) {
	//	std::thread(&SdlIo::Loop, std::make_unique<SdlIo>(&JVSHandler->Inputs, setup.sdldeviceindex)).detach();
	//}
	//else {
	//	std::thread(&WiiIo::Loop, std::make_unique<WiiIo>(setup.players, &JVSHandler->Inputs)).detach();
	//}

	JvsIo::Status jvsStatus;
	SerIo::Status serialStatus;

	std::vector<uint8_t> SerialBuffer;

	while (running) {
		if (!SerialBuffer.empty()) {
			SerialBuffer.clear();
		}

		serialStatus = SerialHandler->Read(&SerialBuffer);
		if (serialStatus != SerIo::Okay) {
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			continue;
		}

		jvsStatus = JVSHandler->ReceivePacket(SerialBuffer);
		if (jvsStatus == JvsIo::Okay) {
			jvsStatus = JVSHandler->SendPacket(SerialBuffer);
			SerialHandler->Write(&SerialBuffer);
		}

		if(JVSHandler->pSenseChange){
			if(JVSHandler->pSense == JvsIo::NotConnected) {
					GPIOHandler->SetMode(GpIo::In);
			}
			else {
				GPIOHandler->SetMode(GpIo::Out);
				GPIOHandler->Write(GpIo::Low);
			}
			JVSHandler->pSenseChange = false;
		}

		// NOTE: This is a workaround for Crazy Taxi - High Roller on Chihiro
		// Without this the Chihiro will crash (likely) or stop sending packets to us (less likely).
		std::this_thread::sleep_for(std::chrono::microseconds(250));
	}

	std::cout << std::endl;
	return 0;
}
