#ifndef WIIIO_H
#define wIIIO_H

#include <iostream>
#include <vector>
#include <functional>

#include <unistd.h>
#include <poll.h>
#include <cmath>
#include <cstring>
#include <xwiimote.h>

#include "JvsIo.h"

class WiiIo
{
public:
	jvs_input_states_t *Inputs;	
	struct xwii_iface *iface;
	WiiIo(jvs_input_states_t *jvs_inputs);
	//~WiiIo();
	void Loop();
private:
	void ButtonPressHandler(xwii_event_key* button);
	void IRMovementHandler(xwii_event_abs* ir);
};

#endif
