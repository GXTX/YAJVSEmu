#ifndef GPIO_H
#define GPIO_H

#include <cstdint>
#include <iostream>
#include <bcm2835.h>

#define PIN RPI_BPLUS_GPIO_J8_32

class GpIo
{
public:
	enum SenseType{
		None,
		Float,
		//Switch = 2,
	};

	enum PinState{
		In,
		Out,
	};

	enum OutputState{
		Low,
		High,
	};

	bool IsInitialized;

	GpIo(SenseType sense_type);
	~GpIo();
	void TogglePin(PinState state);
	void Write(OutputState state);
private:
	void Init(SenseType sense_type);
	// TODO: Add privates 
};

extern GpIo* g_pGpIo;

#endif
