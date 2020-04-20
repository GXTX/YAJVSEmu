#include "GpIo.h"

GpIo* g_pGpIo;

GpIo::GpIo(SenseType sense_type, uint8_t gpio_pin = 12)
{
	switch (sense_type) {
		case SenseType::Float:
		//case SenseType::Switch:
			Init(sense_type);
			break;
		default:
			break;
	}
}

GpIo::~GpIo()
{
	bcm2835_close();
}

void GpIo::Init(SenseType sense_type)
{
	if (bcm2835_init()) {
		switch (sense_type) {
			case SenseType::Float:
				bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT);
				std::cout << "GpIo::Init: Setting pin to IN." << std::endl;
				IsInitialized = true;
				break;
			//case SenseType::Switch: // Untested
			//	bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);
			//	bcm2835_gpio_write(PIN, 0);
			//	break;
			default:
				break;
		}
	}
	else if (sense_type != SenseType::None) {
		IsInitialized = false;
		std::cout << "GpIo::Init: Failed to initalize the bcm2835 lib.\n" << std::endl;
	}
}

void GpIo::TogglePin(PinState state) 
{
	if (state == PinState::In) {
		std::cout << "GpIo::TogglePin: Toggling pin to IN.\n" << std::endl;
		bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT);
	}
	else {
		std::cout << "GpIo::TogglePin: Toggling pin to OUT.\n" << std::endl;
		bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);
	}
}

void GpIo::Write(OutputState state)
{
	bcm2835_gpio_write(PIN, state);
}
