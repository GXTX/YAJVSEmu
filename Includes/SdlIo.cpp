#include <iostream>

#include <SDL.h>
#include <SDL_stdinc.h>

#include "JvsIo.h"

jvs_input_states_t states;

enum AxisType {
	Stick,
	Trigger,
};

void handle_button_press(SDL_ControllerButtonEvent *button);
void handle_axis_movement(SDL_ControllerAxisEvent *axis);
int scaled(int16_t value, AxisType type);

int main()
{
	// TODO: selectable via argv or config
	SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS);
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
	SDL_GameController *sgc = SDL_GameControllerOpen(0);
	SDL_Event event;
	if (sgc == nullptr) {
		std::cerr << "Joystick Error: " << SDL_GetError() << std::endl;
		return 1;
	}
	else {
		std::cout << "Joystick Connected: " << SDL_GameControllerName(sgc);
	}

	while (1) {
		//for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
		//	int state = SDL_GameControllerGetButton(sgc, (SDL_GameControllerButton)i);
		//	std::cout << "State: " << state << " : Name: " << SDL_GameControllerGetStringForButton((SDL_GameControllerButton)i) << std::endl;
		//}

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_CONTROLLERBUTTONDOWN:
				case SDL_CONTROLLERBUTTONUP:
					handle_button_press(&event.cbutton);
					break;
				case SDL_CONTROLLERAXISMOTION:
					handle_axis_movement(&event.caxis);
				default:
					break;
			}
		}
		// TODO: required to stop using 100% of CPU, find lowest reasonable value
		SDL_Delay(10);
	}

	return 0;
}

void handle_button_press(SDL_ControllerButtonEvent *button)
{
	switch (button->button) {
		case SDL_CONTROLLER_BUTTON_A: states.switches.player[0].button[0] = button->state; break;
		case SDL_CONTROLLER_BUTTON_B: states.switches.player[0].button[1] = button->state; break;
		case SDL_CONTROLLER_BUTTON_X: states.switches.player[0].button[2] = button->state; break;
		case SDL_CONTROLLER_BUTTON_Y: states.switches.player[0].button[3] = button->state; break;
		case SDL_CONTROLLER_BUTTON_BACK: states.switches.system = button->state; break;
		case SDL_CONTROLLER_BUTTON_GUIDE: states.switches.player[0].service = button->state; break;
		case SDL_CONTROLLER_BUTTON_START: states.switches.player[0].start = button->state; break;
		case SDL_CONTROLLER_BUTTON_LEFTSTICK: states.switches.player[0].button[4] = button->state; break;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK: states.switches.player[0].button[5] = button->state; break;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: states.switches.player[0].button[6] = button->state; break;
		/*case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: break;*/
		case SDL_CONTROLLER_BUTTON_DPAD_UP: states.switches.player[0].up = button->state; break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN: states.switches.player[0].down = button->state; break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT: states.switches.player[0].left = button->state; break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: states.switches.player[0].right = button->state; break;
		default: break;
	}
}

void handle_axis_movement(SDL_ControllerAxisEvent *axis)
{
	switch (axis->axis) {
		case SDL_CONTROLLER_AXIS_LEFTX: states.analog[0].value = scaled(axis->value, AxisType::Stick); break;
		/*case SDL_CONTROLLER_AXIS_RIGHTX: break;*/
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT: states.analog[2].value = scaled(axis->value, AxisType::Trigger); break;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: states.analog[1].value = scaled(axis->value, AxisType::Trigger); break;
		default: break;
	}
}

int scaled(int16_t value, AxisType type)
{
	float max = SDL_MAX_SINT16;
	// NOTE: SDL2 limits trigger lower range to 0
	float min = (type == AxisType::Trigger ? 0 : SDL_MIN_SINT16);

	double scaled_value = (value - min) / (max - min);
	int scaled_return = scaled_value * 0xFFFF; //0xFF

	return scaled_return;
}
