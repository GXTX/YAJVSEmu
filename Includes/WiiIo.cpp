#include "WiiIo.h"

WiiIo::WiiIo(jvs_input_states_t *jvs_inputs)
{
	Inputs = jvs_inputs;
	struct xwii_monitor *mon;
	char *ent;
	int num = 0;
	int ret;

	// Start scan looking for WiiMotes
	mon = xwii_monitor_new(false, false);
	if (!mon) {
		std::cout << "WiiIo::WiiIo: Unable to create monitor." << std::endl;
	}

	while ((ent = xwii_monitor_poll(mon))) {
		std::printf("WiiIo::WiiIo: Found device #%d: %s", ++num, ent);
		std::cout << std::endl;
		break;
	}
	xwii_monitor_unref(mon);

	// Attempt to connect the device.
	ret = xwii_iface_new(&iface, ent);
	if (ret) {
		std::cout << "WiiIo::WiiIo: Unable to connect controller." << std::endl;
	}
	else {
		ret = xwii_iface_open(iface, XWII_IFACE_CORE | XWII_IFACE_IR);
		if (ret) {
			std::printf("WiiIo::WiiIo: Cannot open interface: %d", ret);
			std::cout << std::endl;
		}
		std::printf("WiiIo::WiiIo: Successfully connected %s.", XWII__NAME);
		std::cout << std::endl;
	}
}

WiiIo::~WiiIo()
{
	xwii_iface_unref(iface);
}

void WiiIo::Loop()
{
	struct xwii_event event;
	int ret = 0, fds_num;
	struct pollfd fds[2];

	memset(fds, 0, sizeof(fds));
	fds[0].fd = 0;
	fds[0].events = POLLIN;
	fds[1].fd = xwii_iface_get_fd(iface);
	fds[1].events = POLLIN;
	fds_num = 2;

	while (true) {
		ret = poll(fds, fds_num, -1);
		if (ret < 0) {
			if (errno != EINTR) {
				ret = -errno;
				std::printf("WiiIo::Loop: Cannot poll fds: %d", ret);
				std::cout << std::endl;
				break;
			}
		}

		ret = xwii_iface_dispatch(iface, &event, sizeof(event));
		if (ret) {
			if (ret != -EAGAIN) {
				std::printf("WiiIo::Loop: Read failed with error : %d", ret);
				std::cout << std::endl;
				break;
			}
		} else {
			switch (event.type) {
				case XWII_EVENT_KEY: ButtonPressHandler(&event.v.key); break;
				case XWII_EVENT_IR: IRMovementHandler(event.v.abs); break;
				default:
					break;
			}
		}
	}
}

void WiiIo::ButtonPressHandler(xwii_event_key* button)
{
	switch (button->code) {
		case XWII_KEY_A: Inputs->switches.player[0].button[1] = button->state; break;
		case XWII_KEY_B: Inputs->switches.player[0].button[0] = button->state; break;
		case XWII_KEY_ONE: Inputs->switches.player[0].button[2] = button->state; break;
		case XWII_KEY_TWO: Inputs->switches.player[0].button[3] = button->state; break;
		case XWII_KEY_MINUS: Inputs->switches.system.test = button->state; break;
		case XWII_KEY_HOME: Inputs->switches.player[0].service = button->state; break;
		case XWII_KEY_PLUS: Inputs->switches.player[0].start = button->state; break;
		default: break;
	}
}

void WiiIo::IRMovementHandler(xwii_event_abs* ir)
{
	if (xwii_event_ir_is_valid(&ir[0]) && xwii_event_ir_is_valid(&ir[1]))  {
		int middlex = (ir[0].x + ir[1].x) / 2;
		int middley = (ir[0].y + ir[1].y) / 2;

		float valuex = middlex;
		float valuey = middley - 1023;

		uint16_t finalx = (valuex / 1023.0) * 0xFFFF;
		uint16_t finaly = std::fabs((valuey / 1023.0) * 0xFFFF);

		Inputs->analog[0].value = finalx;
		Inputs->analog[1].value = finaly;
	}
}
