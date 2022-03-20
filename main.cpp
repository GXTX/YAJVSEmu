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
#include "Setup.h"
#include "version.h"

#define NUMBER_OF_IO_BOARDS 1 // Max: 31

//#define REAL_TIME

#ifdef REAL_TIME
// If we don't delay longer we'll starve the other processes of CPU time.
auto delay{std::chrono::microseconds(20)};
#else
auto delay{std::chrono::microseconds(5)};
#endif

std::atomic<bool> running{true};

void sig_handle(int sig)
{
	if (sig == SIGINT || sig == SIGTERM) {
		running = false;
	}
}

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
		std::cerr << "Couldn't initalize GPIO controller.\n";
		return 1;
	}

	// TODO: probably doesn't need to be shared? we only need Inputs to be a shared ptr
	std::vector<std::shared_ptr<JvsIo>> boards;
	while (boards.size() != NUMBER_OF_IO_BOARDS) {
		boards.emplace_back(std::make_shared<JvsIo>(JvsIo::SenseState::NotConnected));
	}

	std::unique_ptr<SetupInfo> Setup (std::make_unique<SetupInfo>());
	if (!Setup->IsFinished) {
		std::cerr << "Unknown problem setting up input devices.\n";
		return 1;
	}

	std::unique_ptr<SerIo> SerialHandler (std::make_unique<SerIo>(Setup->info.serial_port.c_str()));
	if (!SerialHandler->IsInitialized) {
		std::cerr << "Coudln't initiate the serial controller.\n";
		return 1;
	}

	// Only hookup inputs for the first IO board, we can support all 31 but no reason.
	if (Setup->info.backend == Setup->info.SDL) {
		std::thread(&SdlIo::Loop, std::make_unique<SdlIo>(Setup->info.device_index, &boards[0]->Inputs)).detach();
	} else if (Setup->info.backend == Setup->info.XWII) {
		std::thread(&WiiIo::Loop, std::make_unique<WiiIo>(Setup->info.players, &boards[0]->Inputs)).detach();
	}

	// Free
	Setup.reset();

	JvsIo::Status jvsStatus;
	SerIo::Status serialStatus;

	std::vector<uint8_t> ReadBuffer{};
	std::vector<uint8_t> WriteBuffer{};
	ReadBuffer.reserve(256);
	WriteBuffer.reserve(256);

	std::cout << "Running...\n";

	std::bool emptyBuffer{false};

	while (running) {
		if (emptyBuffer && !ReadBuffer.empty()) {
			ReadBuffer.clear();
		}

		if (SerialHandler->Read(ReadBuffer) != SerIo::Status::Okay) {
			std::this_thread::sleep_for(delay);
			continue;
		}

		if (ReadBuffer.size() < 5) { // smallest packet size is 5 bytes
			std::this_thread::sleep_for(delay);
			continue;
		}

		for (auto &board : boards) {
			jvsStatus = board->ReceivePacket(ReadBuffer);
			if (jvsStatus == JvsIo::Status::Okay || jvsStatus == JvsIo::Status::ChecksumError) {
				if (!WriteBuffer.empty()) {
					WriteBuffer.clear();
				}
				board->SendPacket(WriteBuffer);
				serialStatus = SerialHandler->Write(WriteBuffer);
				if (serialStatus == SerIo::Status::Okay) {
					// Avoid checking the rest of the boards if we have more than one & we've generated a valid response.
					emptyBuffer = true;
					break;
				}
			} else if (jvsStatus == JvsIo::Status::CountError) {
				if ((ReadBuffer.size() - 3) < Readbuffer[2]) {
					emptyBuffer = false;
				} else {
					emptyBuffer = true;
				}
				break;
			}
			emptyBuffer = true;
		}

		if (boards[0]->pSenseChange) { // If the first board wants a change then we should iterate through the rest, otherwise we're wasting cycles.
			if (std::all_of(boards.begin(), boards.end(), [](const auto &board) { return board->pSenseChange; })) {
				// We only care about the first boards setting.
				if (boards[0]->pSense == JvsIo::SenseState::NotConnected) {
					GPIOHandler->SetMode(GpIo::PinMode::In);
				} else {
					GPIOHandler->SetMode(GpIo::PinMode::Out);
					GPIOHandler->Write(GpIo::PinState::Low);
				}

				// Reset all boards.
				for (auto &board : boards) {
					board->pSenseChange = false;
				}
			}
		}

		std::this_thread::sleep_for(delay);
	}

	return 0;
}
