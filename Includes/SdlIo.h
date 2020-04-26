#ifndef SDLIO_H
#define SDLIO_H

#include <iostream>

#include <SDL.h>
#include <SDL_stdinc.h>

#include "JvsIo.h"

class SdlIo
{
public:
	jvs_input_states_t *Inputs;

	SdlIo(jvs_input_states_t *jvs_inputs);
	~SdlIo();
	void Loop();
private:
	enum AxisType {
		Stick,
		Trigger,
	};

	SDL_GameController *sgc;
	SDL_Event event;

	void ButtonPressHandler(SDL_ControllerButtonEvent *button);
	void AxisMovementHandler(SDL_ControllerAxisEvent *axis);
	uint16_t ScaledAxisMovement(int16_t value, AxisType type);
};

#endif
