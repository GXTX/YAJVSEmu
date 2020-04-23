#include "GpIo.h"

#define DEBUG_GPIO

GpIo* g_pGpIo;

GpIo::GpIo(SenseType sense_type)
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
				std::cout << "GpIo::Init: Initialized GPIO pin." << std::endl;
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
		std::cout << "GpIo::Init: Failed to initalize the bcm2835 lib." << std::endl;
	}
}

void GpIo::TogglePin(PinState state) 
{
	if (state == PinState::In) {
		bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT);
#ifdef DEBUG_GPIO
		std::cout << "GpIo::TogglePin: Toggled pin to IN." << std::endl;
#endif
	}
	else {
		bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);
#ifdef DEBUG_GPIO
		std::cout << "GpIo::TogglePin: Toggled pin to OUT." << std::endl;
#endif
	}
}

void GpIo::Write(OutputState state)
{
	if (state == OutputState::Low) {
		bcm2835_gpio_write(PIN, LOW);
#ifdef DEBUG_GPIO
		std::cout << "GpIo::Write: Set pin to pulldown." << std::endl;
#endif
	}
	else {
		bcm2835_gpio_write(PIN, HIGH);
#ifdef DEBUG_GPIO
		std::cout << "GpIo::Write: Set pin to pullup." << std::endl;
#endif
	}
}
