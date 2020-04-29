#include "SdlIo.h"

SdlIo::SdlIo(jvs_input_states_t *jvs_inputs, int device_index)
{
	Inputs = jvs_inputs;
	SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS);
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
	// For additional mappings use https://github.com/gabomdq/SDL_GameControllerDB
	// Or use custom ones you generate yourself using SDL2's test app or
	// 3rd party applications such as http://www.generalarcade.com/gamepadtool/
	if (std::filesystem::exists("gamecontrollerdb.txt")) {
		SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
	}
	sgc = SDL_GameControllerOpen(device_index);
	//SDL_Event event;
	if (sgc == nullptr) {
		std::cerr << "SdlIo::SdlIo: " << SDL_GetError() << std::endl;
	}
	else {
		std::cout << "SdlIo::SdlIo: Connected " << SDL_GameControllerName(sgc) << std::endl;
	}
}

SdlIo::~SdlIo()
{
	SDL_GameControllerClose(sgc);
}

void SdlIo::Loop()
{
	while (true) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_CONTROLLERBUTTONDOWN:
				case SDL_CONTROLLERBUTTONUP:
					ButtonPressHandler(&event.cbutton);
					break;
				case SDL_CONTROLLERAXISMOTION:
					AxisMovementHandler(&event.caxis);
				default:
					break;
			}
		}
		// TODO: required to stop using 100% of CPU, find lowest reasonable value
		SDL_Delay(5);
	}
}

void SdlIo::ButtonPressHandler(SDL_ControllerButtonEvent *button)
{
	switch (button->button) {
		case SDL_CONTROLLER_BUTTON_A: Inputs->switches.player[0].button[0] = button->state; break;
		case SDL_CONTROLLER_BUTTON_B: Inputs->switches.player[0].button[1] = button->state; break;
		case SDL_CONTROLLER_BUTTON_X: Inputs->switches.player[0].button[2] = button->state; break;
		case SDL_CONTROLLER_BUTTON_Y: Inputs->switches.player[0].button[3] = button->state; break;
		case SDL_CONTROLLER_BUTTON_BACK: Inputs->switches.system.test = button->state; break;
		case SDL_CONTROLLER_BUTTON_GUIDE: Inputs->switches.player[0].service = button->state; break;
		case SDL_CONTROLLER_BUTTON_START: Inputs->switches.player[0].start = button->state; break;
		case SDL_CONTROLLER_BUTTON_LEFTSTICK: Inputs->switches.player[0].button[4] = button->state; break;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK: Inputs->switches.player[0].button[5] = button->state; break;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: Inputs->switches.player[0].button[6] = button->state; break;
		/*case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: break;*/
		case SDL_CONTROLLER_BUTTON_DPAD_UP: Inputs->switches.player[0].up = button->state; break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN: Inputs->switches.player[0].down = button->state; break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT: Inputs->switches.player[0].left = button->state; break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: Inputs->switches.player[0].right = button->state; break;
		default: break;
	}
}

void SdlIo::AxisMovementHandler(SDL_ControllerAxisEvent *axis)
{
	switch (axis->axis) {
		case SDL_CONTROLLER_AXIS_LEFTX: Inputs->analog[0].value = ScaledAxisMovement(axis->value, AxisType::Stick); break;
		/*case SDL_CONTROLLER_AXIS_RIGHTX: break;*/
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT: Inputs->analog[2].value = ScaledAxisMovement(axis->value, AxisType::Trigger); break;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: Inputs->analog[1].value = ScaledAxisMovement(axis->value, AxisType::Trigger); break;
		default: break;
	}
}

uint16_t SdlIo::ScaledAxisMovement(int16_t value, AxisType type)
{
	float max = SDL_MAX_SINT16;
	// NOTE: SDL2 limits trigger lower range to 0
	float min = (type == AxisType::Trigger ? 0 : SDL_MIN_SINT16);

	float scaled_value = (value - min) / (max - min);
	uint16_t scaled_return = scaled_value * 0xFFFF;

	return scaled_return;
}
