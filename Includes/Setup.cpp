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

#include "Setup.h"

SetupInfo::SetupInfo()
{
    if (UserInput() == SetupStatus::Okay) {
        IsFinished = true;
		std::cout << "Setup complete.\n" << "==================================================\n";
    }
}

SetupInfo::SetupStatus SetupInfo::UserInput()
{
	std::string user_input{};

	std::cout << "==================================================\n";

	std::cout << "Which serial port would you like to use? (Default: /dev/ttyUSB0): ";
	std::getline(std::cin, user_input);

	// TODO: Sanitize and verify answer.
	if (user_input.empty()) {
		info.serial_port = "/dev/ttyUSB0";
	}

	user_input.clear();

	std::cout << "Do you want to use an SDL controller? (Y/N): ";
	std::getline(std::cin, user_input);

	std::transform(user_input.begin(), user_input.end(), user_input.begin(), [](unsigned char c){ return std::tolower(c); });

	if (user_input.compare("y") == 0) {
		if (SDLAsk() == SetupStatus::Failed) {
			return SetupStatus::Failed;
		}
		return SetupStatus::Okay;
	}

	user_input.clear();

	std::cout << "How about a Wii Remote? (Y/N): ";
	std::getline(std::cin, user_input);

	std::transform(user_input.begin(), user_input.end(), user_input.begin(), [](unsigned char c){ return std::tolower(c); });

	if (user_input.compare("y") == 0) {
		if (XWIIAsk() == SetupStatus::Failed) {
			return SetupStatus::Failed;
		}
		return SetupStatus::Okay;
	} else if (user_input.compare("n") == 0){
		info.backend = SetupInfos::Backend::None;
		return SetupStatus::Okay;
	}

	return SetupStatus::Failed;
}

SetupInfo::SetupStatus SetupInfo::SDLAsk()
{
	std::string user_input{};
	SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

	std::cout << "Which joystick do you wish to use?\n"
		<< "--------------------------------------------------\n";

	int numberOfSDLDevices{0};
	for (int i = 0; i != SDL_NumJoysticks(); i++) {
		if (SDL_IsGameController(i)) {
			std::printf("%d: %s\n", i+1, SDL_JoystickNameForIndex(i));
		} else {
			std::printf("%d: %s (Requires external mapping)\n", i+1, SDL_JoystickNameForIndex(i));
		}
		numberOfSDLDevices++;
	}

	if (numberOfSDLDevices == 0) {
		std::cout << "Couldn't find any compatible devices to connect.\n";
		return SetupStatus::Failed;
	}

	std::cout << "--------------------------------------------------\n"
		<< "Choice: ";
	std::getline(std::cin, user_input);

	if (!std::strtol(user_input.data(), NULL, 10)) {
		SDL_Quit();
		return SetupStatus::Failed;
	}

	info.device_index = std::stoi(user_input);

	if (info.device_index > numberOfSDLDevices) {
		SDL_Quit();
		return SetupStatus::Failed;
	}

	info.backend = SetupInfos::Backend::SDL;
	info.device_index--; // We add an offset of +1, remove it.
	SDL_Quit();

	return SetupStatus::Okay;
}

SetupInfo::SetupStatus SetupInfo::XWIIAsk()
{
	std::string user_input{};
	char *ent;
	std::vector<std::string> wiiremotePaths{}; // TODO: Use this for remote initialization

	xwii_monitor *monitor = xwii_monitor_new(false, false);
	if (monitor) {
		while ((ent = xwii_monitor_poll(monitor))) {
			wiiremotePaths.emplace_back(ent);
		}
		free(ent);
		xwii_monitor_unref(monitor);

		if (wiiremotePaths.size() == 0) {
			std::cout << "Couldn't find any Wii Remotes.\n";
			return SetupStatus::Failed;
		}

		if (wiiremotePaths.size() == 1) {
			info.players = 1;
			info.backend = SetupInfos::Backend::XWII;
			return SetupStatus::Okay;
		} else {
			std::cout << "Would you like to attach more than one WiiMote? (Y/N): ";
			std::getline(std::cin, user_input);

			if (user_input.compare("y") == 0) {
				user_input.clear();
				std::cout << "How many? ";
				std::printf("(1 - %d", wiiremotePaths.size() > JVS_MAX_PLAYERS ? JVS_MAX_PLAYERS : wiiremotePaths.size());
				std::getline(std::cin, user_input);

				if (!std::strtol(user_input.data(), NULL, 10)) {
					return SetupStatus::Failed;
				}

				int numberOfPlayers = std::stoi(user_input);

				if (numberOfPlayers > static_cast<int>(wiiremotePaths.size()) || numberOfPlayers == 0 || numberOfPlayers > JVS_MAX_PLAYERS) {
					return SetupStatus::Failed;
				}

				info.players = numberOfPlayers;
				info.backend = SetupInfos::Backend::XWII;
				return SetupStatus::Okay;
			} else if (user_input.compare("n") != 0) { // If user_input wasn't Y/N fail.
				return SetupStatus::Failed;
			}

			std::cout << "==================================================\n";
			info.players = 1;
			info.backend = SetupInfos::Backend::XWII;

			return SetupStatus::Okay;
		}
	}

	std::cout << "Unable to initalize Wii Remote monitor.\n";
	return SetupStatus::Failed;
}
