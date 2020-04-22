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

	g_pJvsIo = new JvsIo(JvsIo::SenseStates::NotConnected);

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
		// TODO: SendPacket checksum is 0x01 too much causing checksum failures on send.

		int temp = g_pJvsIo->ReceivePacket(&read_buffer);
		std::memset(&read_buffer, 0x00, sizeof(read_buffer));
		int ret = g_pJvsIo->SendPacket(&write_buffer);

		if (ret > 0) {
			if(g_pJvsIo->pSense == JvsIo::SenseStates::NotConnected) {
				g_pGpIo->TogglePin(GpIo::PinState::In);
			}
			else {
				g_pGpIo->TogglePin(GpIo::PinState::Out);
				g_pGpIo->Write(GpIo::OutputState::Low);
			}
			g_pSerIo->Write(&write_buffer, ret);
			std::memset(&write_buffer, 0x00, sizeof(write_buffer));
			// TODO: Only set the pin on a state change..
		}
	}

	return 0;
}

/**
HEADER | NODE | NUMOFBYTES INC COMMAND + CHECKSUM | COMMAND | ARGS | CHKSUM
**/
/*
sample data from chihiro
0xE0 0xFF 0x03 0xF0 0xD9 0xCB <-- valid reset command
0xE0 0xFF 0x03 0xF1 0x01 0xF4 <-- valid setid
*/

/*
openjvs3 logs
Reading..
 E0 00 03 01 01 05 00 00 <-- chihiro sends reset packet and we respond with
 E0 00 1A 01 01 11 01 20 <-- unknown
 E0 00 37 01 01 4F 70 65 <-- unknwon
 E0 00 1C 01 01 00 00 00 <-- looks like a status okay reply but not using 0x03???
*/
