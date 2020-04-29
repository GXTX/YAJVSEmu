#include <iostream>
#include <thread>

#include "Includes/JvsIo.h"
#include "Includes/SerIo.h"
#include "Includes/GpIo.h"
#include "Includes/SdlIo.h"
#include "Includes/WiiIo.h"

#include <SDL.h>
#include <xwiimote.h>
#include <sched.h>

typedef struct {
	char *serialport;
	bool sdlbackend;
	int sdldeviceindex;
} setup_information;

int setup_questions(setup_information *info);

int main()
{
	setup_information setup;

	int ret = setup_questions(&setup);
	if (ret == 0) {
		std::cout << "You entered something unexpected." << std::endl;
		return 0;
	}

	// Set thread priority to RT. We don't care if this
	// fails but may be required for some systems.
	struct sched_param params;
	params.sched_priority = sched_get_priority_max(SCHED_FIFO);
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &params);

	// TODO: maybe put set these as shared in serio?
	std::vector<uint8_t> ReadBuffer;
	std::vector<uint8_t> WriteBuffer;

	std::unique_ptr<GpIo> GPIOHandler (std::make_unique<GpIo>(GpIo::SenseType::Float));
	if (!GPIOHandler->IsInitialized) {
		std::cerr << "Couldn't initiate GPIO and \"NONE\" wasn't explicitly set." << std::endl;
		return 1;
	}

	// TODO: probably doesn't need to be shared? we only need Inputs to be a shared ptr
	std::shared_ptr<JvsIo> JVSHandler (std::make_shared<JvsIo>(JvsIo::SenseStates::NotConnected));

	std::unique_ptr<SerIo> SerialHandler (std::make_unique<SerIo>(setup.serialport));
	if (!SerialHandler->IsInitialized) {
		std::cerr << "Coudln't initiate the serial controller." << std::endl;
		return 1;
	}

	// Spawn lone SDL2 or WiiIo input thread.
	// NOTE: There probably is no reason we can't have both of these running
	if (setup.sdlbackend) {
		std::thread(&SdlIo::Loop, std::make_unique<SdlIo>(&JVSHandler->Inputs, setup.sdldeviceindex)).detach();
	}
	else {
		std::thread(&WiiIo::Loop, std::make_unique<WiiIo>(&JVSHandler->Inputs)).detach();
	}

	while (true) {
		ReadBuffer.resize(512);
		SerialHandler->Read(ReadBuffer.data());

		if (ReadBuffer.size() > 0) {
			int temp = JVSHandler->ReceivePacket(ReadBuffer.data());
			// TODO: Why without this does it fault?
			WriteBuffer.resize(ReadBuffer.size());
			ReadBuffer.clear();

			int ret = JVSHandler->SendPacket(WriteBuffer.data());

			if (ret > 0) {
				if(JVSHandler->pSenseChange){
					if(JVSHandler->pSense == JvsIo::SenseStates::NotConnected) {
						GPIOHandler->SetMode(GpIo::PinMode::In);
					}
					else {
						GPIOHandler->SetMode(GpIo::PinMode::Out);
						GPIOHandler->Write(GpIo::PinState::Low);
					}
					JVSHandler->pSenseChange = false;
				}
				SerialHandler->Write(WriteBuffer.data(), ret);
				WriteBuffer.clear();
			}
		}
	}

	return 0;
}

int setup_questions(setup_information *info)
{
	struct xwii_monitor *mon;
	char *ent;
	int num = 0;
	std::string com_choice;
	std::string wii_choice;
	std::string sdl_choice;
	int sdl_controller;

	std::cout << "==================================================" << std::endl;

	std::cout << "Which serial port would you like to use? (/dev/ttyX): ";
	std::getline(std::cin, com_choice);

	info->serialport = (char*)std::malloc(com_choice.size()+1);
	std::strcpy(info->serialport, com_choice.c_str());

	mon = xwii_monitor_new(false, false);
	if (mon) {
		while ((ent = xwii_monitor_poll(mon))) {
			num++;
			free(ent);
			break;
		}
		xwii_monitor_unref(mon);
		if (num > 0) {
			std::cout << "Do you want to use your WiiMote? (Y/N): ";
			std::getline(std::cin, wii_choice);
			if (wii_choice.compare("Y") != 0 && 
				wii_choice.compare("N") != 0 &&
				wii_choice.compare("y") != 0 &&
				wii_choice.compare("n") != 0) {
				return 0;
			}
			else if (wii_choice.compare("y") == 0) {
				std::cout << "==================================================" << std::endl;
				info->sdlbackend = false;
				return 1;
			}
		}
		num = 0;
	}

	SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

	std::cout << "Which joystick do you wish to use?" << std::endl
		<< "--------------------------------------------------" << std::endl;
	for (int i = 0; i < SDL_NumJoysticks(); ++i) {
		if (SDL_IsGameController(i)) {
			std::printf("%d: %s", i+1, SDL_JoystickNameForIndex(i));
		}
		else {
			std::printf("%d: %s (Requires external mapping)", i+1, SDL_JoystickNameForIndex(i));
		}
		std::cout << std::endl;
		num++;
	}

	std::cout << "--------------------------------------------------" 
		<< std::endl << "Choice: ";
	std::getline(std::cin, sdl_choice);
	if (!std::strtol(sdl_choice.data(), NULL, 10)) {
		return 0;
	}
	sdl_controller = std::stoi(sdl_choice);
	if (sdl_controller > num+1) {
		return 0;
	}

	info->sdlbackend = true;
	info->sdldeviceindex = std::stoi(sdl_choice) - 1;

	std::cout << "==================================================" << std::endl;

	return 1;
}
