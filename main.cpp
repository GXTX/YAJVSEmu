#include "Includes/JvsIo.h"
#include "Includes/SerIo.h"
#include "Includes/GpIo.h"
#include "Includes/SdlIo.h"

#include <iostream>
#include <thread>

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

	// TODO: is there a better way to do this?
	g_pSdlIo = new SdlIo(&g_pJvsIo->Inputs);
	std::thread InputThread(&SdlIo::Loop, std::ref(g_pSdlIo));

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
