#include "WiiIo.h"

WiiIo::WiiIo(int players, jvs_input_states_t *jvs_inputs)
{
	Inputs = jvs_inputs;
	struct xwii_monitor *mon;
	int numberOfControllers = 0;
	int ret;
	int i = 0;

	numberOfPlayers = players;

	// Start scan looking for WiiMotes
	mon = xwii_monitor_new(false, false);
	if (!mon) {
		std::cout << "WiiIo::WiiIo: Unable to create monitor." << std::endl;
	}

	while (true) {
		if(i == numberOfPlayers) {
			break;
		}
		controllers.controller[i] = xwii_monitor_poll(mon);
		std::printf("WiiIo::WiiIo: Found device #%d: %s", ++numberOfControllers, controllers.controller[i].c_str());
		std::cout << std::endl;
		i++;
	}

	xwii_monitor_unref(mon);

	for (i = 0; i < numberOfControllers; i++) {
		ret = xwii_iface_new(&controllers.interface[i], controllers.controller[i].c_str());
		if (ret) {
			std::cout << "WiiIo::WiiIo: Unable to connect controller: " <<
				std::printf("%d", ret) << std::endl;
		}
		else {
			ret = xwii_iface_open(controllers.interface[i], XWII_IFACE_CORE | XWII_IFACE_IR);
			if (ret) {
				std::printf("WiiIo::WiiIo: Cannot open interface: %d", ret);
				std::cout << std::endl;
			}
			std::printf("WiiIo::WiiIo: Successfully connected %s.", XWII__NAME);
			controllers.fd[i] = xwii_iface_get_fd(controllers.interface[i]);
			std::cout << std::endl;
		}
	}
}

WiiIo::~WiiIo()
{
	for (int i = 0; i < numberOfPlayers; i++) {
		xwii_iface_unref(controllers.interface[i]);
	}
}

void WiiIo::Loop()
{
	int ret = 0;
	std::vector<struct pollfd> fd;

	for (int i = 0; i < numberOfPlayers; i++) {
		fd.push_back({controllers.fd[i], POLLIN, 0});
	}

	while (true) {
		ret = poll(fd.data(), fd.size(), -1);
		if (ret < 0) {
			if (errno != EINTR) {
				ret = -errno;
				std::printf("WiiIo::Loop: Cannot poll fds: %d", ret);
				std::cout << std::endl;
				break;
			}
		}

		for (size_t i = 0; i < fd.size(); i++) {
			ret = xwii_iface_dispatch(controllers.interface[i], &event, sizeof(event));
			if (ret) {
				if (ret != -EAGAIN) {
					std::printf("WiiIo::Loop: Read failed with error : %d", ret);
					std::cout << std::endl;
					break;
				}
			} else {
				switch (event.type) {
					case XWII_EVENT_KEY: ButtonPressHandler(i, &event.v.key); break;
					case XWII_EVENT_IR: IRMovementHandler(i, event.v.abs, MovementValueType::Analog); break;
					default: break;
				}
			}
		}
	}
}

void WiiIo::ButtonPressHandler(int player, xwii_event_key* button)
{
	switch (button->code) {
		case XWII_KEY_A: Inputs->switches.player[player].button[1] = button->state; break;
		case XWII_KEY_B: Inputs->switches.player[player].button[0] = button->state; break;
		case XWII_KEY_ONE: Inputs->switches.player[player].button[2] = button->state; break;
		case XWII_KEY_TWO: Inputs->switches.player[player].button[3] = button->state; break;
		case XWII_KEY_MINUS: Inputs->switches.system.test = button->state; break;
		case XWII_KEY_HOME: Inputs->switches.player[player].service = button->state; break;
		case XWII_KEY_PLUS: Inputs->switches.player[player].start = button->state; break;
		/*case XWII_KEY_UP: Inputs->switches.player[player].up = button->state; break;
		case XWII_KEY_DOWN: Inputs->switches.player[player].down = button->state; break;
		case XWII_KEY_LEFT: Inputs->switches.player[player].left = button->state; break;
		case XWII_KEY_RIGHT: Inputs->switches.player[player].right = button->state; break;*/
		default: break;
	}
}

void WiiIo::IRMovementHandler(int player, xwii_event_abs* ir, MovementValueType type)
{
	if (xwii_event_ir_is_valid(&ir[0]) && xwii_event_ir_is_valid(&ir[1]))  {
		int middlex = (ir[0].x + ir[1].x) / 2;
		int middley = (ir[0].y + ir[1].y) / 2;

		float valuex = middlex;
		float valuey = middley - 1023;

		uint16_t finalx = (valuex / 1023.0) * 0xFFFF;
		uint16_t finaly = std::fabs((valuey / 1023.0) * 0xFFFF);

		if (type == MovementValueType::ScreenPos) {
			Inputs->screen[0].position = (finalx << 16);
			Inputs->screen[0].position |= finaly;
		}
		else {
			switch (player) {
				case 0: Inputs->analog[0].value = finalx; Inputs->analog[1].value = finaly; break;
				case 1: Inputs->analog[2].value = finalx; Inputs->analog[3].value = finaly; break;
				case 2: Inputs->analog[4].value = finalx; Inputs->analog[5].value = finaly; break;
				case 3: Inputs->analog[6].value = finalx; Inputs->analog[7].value = finaly; break;
				default: break;
			}
		}
	}
}
