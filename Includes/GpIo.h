#ifndef GPIO_H
#define GPIO_H

#include <iostream>
#include <bcm2835.h>

#define PIN RPI_BPLUS_GPIO_J8_32

class GpIo
{
public:
	enum SenseType{
		None,
		Float,
		//Switch,
	};

	enum PinMode{
		In,
		Out,
	};

	enum PinState{
		Low,
		High,
	};

	bool IsInitialized;

	GpIo(SenseType sense_type);
	~GpIo();
	void SetMode(PinMode state);
	void Write(PinState state);
private:
};

#endif
