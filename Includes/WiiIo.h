#ifndef WIIIO_H
#define wIIIO_H

#include <iostream>
#include <cmath>
#include <cstring>

#include <unistd.h>
#include <poll.h>

#include <xwiimote.h>

#include "JvsIo.h"

class WiiIo
{
public:
	WiiIo(jvs_input_states_t *jvs_inputs);
	~WiiIo();
	void Loop();
private:
	enum MovementValueType {
		Analog,
		ScreenPos,
	};

	jvs_input_states_t *Inputs;
	struct xwii_iface *iface;
	void ButtonPressHandler(xwii_event_key* button);
	void IRMovementHandler(xwii_event_abs* ir, MovementValueType type);
};

#endif
