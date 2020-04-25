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

	std::shared_ptr<SdlIo> InputHandler (std::make_shared<SdlIo>(&g_pJvsIo->Inputs));
	std::thread InputThread(&SdlIo::Loop, InputHandler);

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
/**
Rationale behind GPIO PinModes:
JVS spec has 3 modes, 5v (nothing attached) / 2.5v (waiting for ID) / 0v (connected).
With 4 diodes in parallel (to GND) we bring down the 5v to ~2.5v to signal the JVS controller we're ready
for an ID. Setting PinMode::In allows us to not touch the 2.5v and we then switch to
PinMode::Out / PinState::Low to bring down the voltage to 0v signaling we have an ID and we're ready.
If we were to set PinMode::Out / PinState::High it raises the voltage to ~2.9v with the diodes.
**/
					if(g_pJvsIo->pSense == JvsIo::SenseStates::NotConnected) {
						g_pGpIo->SetMode(GpIo::PinMode::In);
					}
					else {
						g_pGpIo->SetMode(GpIo::PinMode::Out);
						g_pGpIo->Write(GpIo::PinState::Low);
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
