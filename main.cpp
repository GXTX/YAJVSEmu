#include "Includes/JvsIo.h"
#include "Includes/SerIo.h"
#include "Includes/GpIo.h"

#include <iostream>

int main()
{
	// TODO: Just use a std::string or something..
	char serialName[13];
	std::sprintf(serialName, "/dev/ttyUSB0");

	std::vector<uint8_t> ReadBuffer;
	std::vector<uint8_t> WriteBuffer;

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

	while (1) {
		ReadBuffer.resize(512);
		g_pSerIo->Read(ReadBuffer.data());

		if (ReadBuffer.size() > 0) {
			int temp = g_pJvsIo->ReceivePacket(ReadBuffer.data());
			// TODO: Why without this does it fault?
			WriteBuffer.resize(ReadBuffer.size());
			ReadBuffer.clear();
			// TODO: Check if receivepacket successfully processed then pass off
			// TODO: if a checksum failure has occured, do we need to send a message to mainboard? yes?
			int ret = g_pJvsIo->SendPacket(WriteBuffer.data());

			if (ret > 0) {
				if(g_pJvsIo->pSenseChange){
					if(g_pJvsIo->pSense == JvsIo::SenseStates::NotConnected) {
						g_pGpIo->TogglePin(GpIo::PinState::In);
					}
					else {
						g_pGpIo->TogglePin(GpIo::PinState::Out);
						g_pGpIo->Write(GpIo::OutputState::Low);
					}
					g_pJvsIo->pSenseChange = false;
				}
				g_pSerIo->Write(WriteBuffer.data(), ret);
				WriteBuffer.clear();
			}
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
