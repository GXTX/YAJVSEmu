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

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <csignal>
#include <memory>
#include <tuple>

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
auto delay{std::chrono::microseconds(25)};
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

	// TODO: make sense mode configuable, Sink for OpenJVS hat
	std::unique_ptr<GpIo> GPIOHandler (std::make_unique<GpIo>(GpIo::SenseType::Sink));
	if (!GPIOHandler->IsInitialized) {
		std::cerr << "Couldn't initalize GPIO controller.\n";
		return 1;
	}

	std::vector<std::unique_ptr<JvsIo>> boards{};
	while (boards.size() != NUMBER_OF_IO_BOARDS) {
		boards.emplace_back(std::make_unique<JvsIo>(JvsIo::SenseState::NotConnected));
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

	Setup.reset();

	std::tuple<JvsIo::Status, size_t> jvsStatus{};

	std::vector<uint8_t> readBuffer{};
	std::vector<uint8_t> writeBuffer{};
	readBuffer.reserve(512);
	writeBuffer.reserve(512);

	std::cout << "Running...\n";

	size_t manErase{};

	while (running) {
		if (SerialHandler->Read(readBuffer) != SerIo::Status::Okay) {
			std::this_thread::sleep_for(delay);
			continue;
		}

		if (manErase > 0) {
			readBuffer.erase(readBuffer.begin(), readBuffer.begin() + manErase);
		}

		for (const auto &board : boards) {
			// No sense running ReceivePacket if we don't have enough in the buffer
			if (readBuffer.size() < 4) {
				break;
			}

			jvsStatus = board->ReceivePacket(readBuffer);

			if (std::get<JvsIo::Status>(jvsStatus) == JvsIo::Status::Okay || std::get<JvsIo::Status>(jvsStatus) == JvsIo::Status::ChecksumError) {
				writeBuffer.clear();

				if (board->SendPacket(writeBuffer) != JvsIo::Status::EmptyResponse) {
					manErase = 0;
					SerialHandler->Write(writeBuffer);
					break;
				}
			} else if (std::get<JvsIo::Status>(jvsStatus) == JvsIo::Status::SyncError || std::get<JvsIo::Status>(jvsStatus) == JvsIo::Status::CountError) {
				manErase = 0;
				break;
			}

			manErase = std::get<size_t>(jvsStatus);
		}

		// Protect against malformed packets, or if we're in a real chain
		int currentTarget = boards.back()->currentTarget;
		if (currentTarget != JvsIo::TARGET_BROADCAST && std::none_of(boards.begin(), boards.end(), [currentTarget](const auto &board) { return currentTarget == board->DeviceID; })) {
			readBuffer.clear();
		}

		if (boards[0]->pSenseChange) { // If the first board wants a change then we should iterate through the rest, otherwise we're wasting cycles.
			if (std::all_of(boards.begin(), boards.end(), [](const auto &board) { return board->pSenseChange; })) {
				// We only care about the first boards setting.
				if (boards[0]->pSense == JvsIo::SenseState::NotConnected) {
					GPIOHandler->SetMode(GpIo::PinMode::In);
				} else {
					GPIOHandler->SetMode(GpIo::PinMode::Out);
					if (GPIOHandler->senseType == GpIo::SenseType::Float) {
						GPIOHandler->Write(GpIo::PinState::Low);
					} else { // Sink
						GPIOHandler->Write(GpIo::PinState::High);
					}
				}

				// Reset all boards.
				for (const auto &board : boards) {
					board->pSenseChange = false;
				}
			}
		}

		std::this_thread::sleep_for(delay);
	}

	return 0;
}
