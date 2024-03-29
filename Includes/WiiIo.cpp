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

#include "WiiIo.h"

//#define DEBUG_IR_POS

WiiIo::WiiIo(uint8_t players, jvs_input_states *jvsInputs)
{
	Inputs = jvsInputs;

	Player.resize(players);

	// Start scan looking for Wii Remotes
	xwii_monitor *mon = xwii_monitor_new(false, true);
	if (!mon)
		std::puts("WiiIo::WiiIo: Unable to create monitor.");

	// FIXME: If there's multiple 'players' but less controllers then we crash.
	for (uint8_t i = 0; i != players; i++) {
		Player[i].id = i;
		Player[i].controller = xwii_monitor_poll(mon);
		if (Player[i].controller.empty()) {
			std::puts("Not good! We need to kill the thread here instead of continuing.");
			break;
		}
		std::printf("WiiIo::WiiIo: Found device #%d: %s\n", i, Player[i].controller.c_str());
	}
	xwii_monitor_unref(mon);

	// NOTE: There is a bug with xwiimote where Nyko controllers wont always enable their IR interface,
	// so we need to make sure this is opened. It's possible to stall the thread and attempt to connect
	// forever but it's *obviously* imporant that we have access to IR.
	for (wii_remote &remote : Player) {
		while (true) {
			std::puts("WiiIo::WiiIo: Connecting Wii Remote...");
			xwii_iface_new(&remote.interface, remote.controller.c_str());
			xwii_iface_open(remote.interface, XWII_IFACE_ALL | XWII_IFACE_WRITABLE);
			if (xwii_iface_opened(remote.interface) & XWII_IFACE_CORE && xwii_iface_opened(remote.interface) & XWII_IFACE_IR) {
				remote.fd = xwii_iface_get_fd(remote.interface);
				std::puts("WiiIo::WiiIo: Successfully connected Wii Remote.");
				break;
			}
			xwii_iface_unref(remote.interface);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}

WiiIo::~WiiIo()
{
	for (wii_remote &remote : Player) {
		xwii_iface_unref(remote.interface);
	}
}

void WiiIo::Loop()
{
	std::vector<pollfd> fd;
	for (wii_remote remote : Player) {
		fd.push_back({remote.fd, POLLIN, 0});
	}

	while (true) {
		int fdret = poll(fd.data(), fd.size(), -1);
		if (fdret < 0) {
			std::puts("WiiIo::Loop: Can't poll the fd!");
			break;
		}
		for (wii_remote &remote : Player) {
			int wiiret = xwii_iface_dispatch(remote.interface, &event, sizeof(event));
			if (wiiret < 0 && wiiret != -EAGAIN) {
				std::puts("WiiIo::Loop: Failed to read fd queue.");
				continue;
			}
			switch (event.type) {
				case XWII_EVENT_KEY: ButtonPressHandler(remote.id, &event.v.key, remote.interface); break;
				case XWII_EVENT_IR: IRMovementHandler(remote.id, event.v.abs, MovementValueType::Analog); break;
				default: break;
			}
		}
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
}

void WiiIo::ButtonPressHandler(int player, xwii_event_key *button, xwii_iface *fd)
{
	switch (button->code) {
		case XWII_KEY_A: 
			Inputs->switches.player[player].button[1] = button->state;
			xwii_iface_rumble(fd, button->state);
		break;
		case XWII_KEY_B: 
			Inputs->switches.player[player].button[0] = button->state;
			xwii_iface_rumble(fd, button->state);
		break;
		case XWII_KEY_ONE: Inputs->switches.player[player].button[2] = button->state; break;
		case XWII_KEY_TWO: Inputs->switches.player[player].button[3] = button->state; break;
		case XWII_KEY_MINUS: Inputs->switches.system.test = button->state; break;
		case XWII_KEY_HOME: Inputs->switches.player[player].service = button->state; break;
		case XWII_KEY_PLUS: Inputs->switches.player[player].start = button->state; break;
		case XWII_KEY_UP: Inputs->switches.player[player].up = button->state; break;
		case XWII_KEY_DOWN: Inputs->switches.player[player].down = button->state; break;
		case XWII_KEY_LEFT: Inputs->coins[player].coins++; break;
		case XWII_KEY_RIGHT: Inputs->coins[player].coins--; break;
		default: break;
	}
}

void WiiIo::IRMovementHandler(int player, xwii_event_abs *ir, MovementValueType type)
{
	int middlex{};
	int middley{};

	// NOTE: Due to bugs with libxwiimote we can sometimes present as a valid 1 & 2
	// but report as X:0 & Y:0, where we would actually want to read from 1 & 3.
	// This generally only happens with 3rd party Nyko controllers.
	if (xwii_event_ir_is_valid(&ir[0]) && xwii_event_ir_is_valid(&ir[1])) {
		middlex = (ir[0].x + ir[1].x) / 2;
		middley = (ir[0].y + ir[1].y) / 2;
	} else if (xwii_event_ir_is_valid(&ir[0]) && xwii_event_ir_is_valid(&ir[2])) {
		middlex = (ir[0].x + ir[2].x) / 2;
		middley = (ir[0].y + ir[2].y) / 2;
	} else {
		return;
	}

	float valuex = middlex - 1023;
	float valuey = middley;

	uint16_t finalx = std::fabs(valuex / 1023) * 0xFFFF;
	uint16_t finaly = (valuey / 1023) * 0xFFFF;

#ifdef DEBUG_IR_POS
	std::printf("POS: %04X/%04X\n", finalx, finaly);
#endif

	if (type == MovementValueType::Analog) {
		switch (player) {
			case 0: Inputs->analog[0].value = finalx; Inputs->analog[1].value = finaly; break;
			case 1: Inputs->analog[2].value = finalx; Inputs->analog[3].value = finaly; break;
			case 2: Inputs->analog[4].value = finalx; Inputs->analog[5].value = finaly; break;
			case 3: Inputs->analog[6].value = finalx; Inputs->analog[7].value = finaly; break;
			default: break;
		}
	} else {
		Inputs->screen[player].position = static_cast<uint32_t>((finalx << 16) | finaly);
	}
}
