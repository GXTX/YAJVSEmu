#include "Includes/JvsIo.h"
#include "Includes/SerIo.h"
#include "Includes/GpIo.h"

#include <iostream>

int main()
{
	// TODO: Just use a std::string or something..
	char serialName[13];
	std::sprintf(serialName, "/dev/ttyUSB4");

	g_pGpIo = new GpIo(GpIo::SenseType::Float); // Some systems require Sense pin support so run this before anything else.
	if (!g_pGpIo->IsInitialized) {
		std::cout << "Dying.\n";
		return 1;
	}

	g_pJvsIo = new JvsIo(0);

	g_pSerIo = new SerIo(serialName);
	if (!g_pSerIo->IsInitialized) {
		std::cout << "Dying.\n";
		return 1;
	}

	uint8_t read_buffer;
	uint8_t write_buffer;

	while(1) {
		g_pSerIo->Read(&read_buffer);

		// TODO: If there's nothing in the buffer, don't bother sending it to receive and send...

		int temp = g_pJvsIo->ReceivePacket(&read_buffer);
		std::memset(&read_buffer, 0x00, sizeof(read_buffer));
		int ret = g_pJvsIo->SendPacket(&write_buffer);

		if (ret > 0) {
			g_pSerIo->Write(&write_buffer, ret);
			//pSense
			//0 not connected / no address
			//1 connected with an address
			// TODO: Only set the pin on a state change..
			if(g_pJvsIo->pSense == 0) {
				g_pGpIo->TogglePin(g_pGpIo->PinState::In);
			}
			else {
				g_pGpIo->TogglePin(g_pGpIo->PinState::Out);
				g_pGpIo->Write(g_pGpIo->OutputState::Low);
			}
		}
	}

	return 0;
}
