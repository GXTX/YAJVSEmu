#ifndef WIIIO_H
#define wIIIO_H

#include <iostream>
#include <cmath>
#include <cstring>

#include <unistd.h>
#include <poll.h>

#include <xwiimote.h>

#include "JvsIo.h"

// TODO: vector?
typedef struct {
	std::string controller[2];
	struct xwii_iface *interface[2];
	int fd[2];
} wiimotes;

class WiiIo
{
public:
	WiiIo(int players, jvs_input_states_t *jvs_inputs);
	~WiiIo();
	void Loop();
private:
	enum MovementValueType {
		Analog,
		ScreenPos,
	};

	jvs_input_states_t *Inputs;
	wiimotes controllers;
	int numberOfPlayers;
	struct xwii_event event;

	void ButtonPressHandler(int player, xwii_event_key* button);
	void IRMovementHandler(int player, xwii_event_abs* ir, MovementValueType type);
};

#endif
