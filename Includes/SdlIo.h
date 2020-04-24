#ifndef SDLIO_H
#define SDLIO_H

#include <iostream>

#include <SDL.h>
#include <SDL_stdinc.h>

#include "JvsIo.h"

class SdlIo
{
public:
	enum AxisType {
		Stick,
		Trigger,
	};

	jvs_input_states_t *Inputs;

	SdlIo(jvs_input_states_t *jvs_inputs);
	//~SdlIo();
	void Loop();
private:
	SDL_GameController *sgc;
	SDL_Event event;

	void ButtonPressHandler(SDL_ControllerButtonEvent *button);
	void AxisMovementHandler(SDL_ControllerAxisEvent *axis);
	int ScaledAxisMovement(int16_t value, AxisType type);
};

extern SdlIo* g_pSdlIo;

#endif