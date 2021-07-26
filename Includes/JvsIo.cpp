// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// ******************************************************************
// *  This file is part of the Cxbx project.
// *
// *  Cxbx and Cxbe are free software; you can redistribute them
// *  and/or modify them under the terms of the GNU General Public
// *  License as published by the Free Software Foundation; either
// *  version 2 of the license, or (at your option) any later version.
// *
// *  This program is distributed in the hope that it will be useful,
// *  but WITHOUT ANY WARRANTY; without even the implied warranty of
// *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// *  GNU General Public License for more details.
// *
// *  You should have recieved a copy of the GNU General Public License
// *  along with this program; see the file COPYING.
// *  If not, write to the Free Software Foundation, Inc.,
// *  59 Temple Place - Suite 330, Bostom, MA 02111-1307, USA.
// *
// *  (c) 2019 Luke Usher
// *  (c) 2020-2021 wutno (https://github.com/GXTX)
// *
// *  All rights reserved
// *
// ******************************************************************

#include "JvsIo.h"

//#define DEBUG_JVS_PACKETS

JvsIo::JvsIo(SenseState sense)
{
	pSense = sense;

	// Version info BCD Format: X.X
	CommandFormatRevision = 0x11;
	JvsVersion = 0x20;
	CommunicationVersion = 0x10;

	BoardID = "SEGA ENTERPRISES,LTD.;I/O BD JVS;837-13551;Ver1.00";

	ResponseBuffer.reserve(512);
}

uint8_t JvsIo::Jvs_Command_F0_Reset(uint8_t *data)
{
	uint8_t ensure_reset = data[1];

	if (ensure_reset == 0xD9) {
		pSense = SenseState::NotConnected; // Set sense 2.5v to instruct the baseboard we're ready.
		pSenseChange = true;
		//ResponseBuffer.emplace_back(JvsReportCode::Handled);
		DeviceID = 0;
	}
	return 1;
}

uint8_t JvsIo::Jvs_Command_F1_SetDeviceId(uint8_t *data)
{
	ResponseBuffer.emplace_back(JvsReportCode::Handled);

	DeviceID = data[1]; // Set address.
	pSense = SenseState::Connected; // Signal to set sense to 0v.
	pSenseChange = true;

	return 1;
}

uint8_t JvsIo::Jvs_Command_10_GetBoardId()
{
	ResponseBuffer.emplace_back(JvsReportCode::Handled);
	std::copy(BoardID.begin(), BoardID.end(), std::back_inserter(ResponseBuffer));

	return 0;
}

uint8_t JvsIo::Jvs_Command_11_GetCommandFormat()
{
	ResponseBuffer.emplace_back(JvsReportCode::Handled);
	ResponseBuffer.emplace_back(CommandFormatRevision);

	return 0;
}

uint8_t JvsIo::Jvs_Command_12_GetJvsRevision()
{
	ResponseBuffer.emplace_back(JvsReportCode::Handled);
	ResponseBuffer.emplace_back(JvsVersion);

	return 0;
}

uint8_t JvsIo::Jvs_Command_13_GetCommunicationVersion()
{
	ResponseBuffer.emplace_back(JvsReportCode::Handled);
	ResponseBuffer.emplace_back(CommunicationVersion);

	return 0;
}

uint8_t JvsIo::Jvs_Command_14_GetCapabilities()
{
	ResponseBuffer.emplace_back(JvsReportCode::Handled);

	// Capabilities list (4 bytes each)

	// Input capabilities
	ResponseBuffer.emplace_back(JvsCapabilityCode::PlayerSwitchButtonSets);
	ResponseBuffer.emplace_back(JVS_MAX_PLAYERS); // number of players
	ResponseBuffer.emplace_back(13); // 13 button switches per player
	ResponseBuffer.emplace_back(0);

	ResponseBuffer.emplace_back(JvsCapabilityCode::CoinSlots);
	ResponseBuffer.emplace_back(JVS_MAX_COINS); // number of coin slots
	ResponseBuffer.emplace_back(0);
	ResponseBuffer.emplace_back(0);

	ResponseBuffer.emplace_back(JvsCapabilityCode::AnalogInputs);
	ResponseBuffer.emplace_back(JVS_MAX_ANALOG); // number of analog input channels
	ResponseBuffer.emplace_back(16); // 16 bits per analog input channel
	ResponseBuffer.emplace_back(0);
/*
	// Input switches
	ResponseBuffer.emplace_back(JvsCapabilityCode::SwitchInputs);
	ResponseBuffer.emplace_back(0);
	ResponseBuffer.emplace_back(16);
	ResponseBuffer.emplace_back(0);

	// NOTE: SEGA hardware used/uses 12 bits, NAMCO is known to use 16 bits.
	ResponseBuffer.emplace_back(JvsCapabilityCode::ScreenPointerInputs);
	ResponseBuffer.emplace_back(16); // 16bits for X
	ResponseBuffer.emplace_back(16); // Y
	ResponseBuffer.emplace_back(JVS_MAX_SCREEN_CHANNELS);
*/
	// Output capabilities
	ResponseBuffer.emplace_back(JvsCapabilityCode::GeneralPurposeOutputs);
	ResponseBuffer.emplace_back(JVS_MAX_GPO); // number of outputs
	ResponseBuffer.emplace_back(0);
	ResponseBuffer.emplace_back(0);

	ResponseBuffer.emplace_back(JvsCapabilityCode::EndOfCapabilities);

	return 0;
}

// TODO: Verify with a test case...
uint8_t JvsIo::Jvs_Command_15_ConveyId(uint8_t *data)
{
	ResponseBuffer.emplace_back(JvsReportCode::Handled);

	std::string masterId;

	// Skip first 2 bytes, max size is 100, break on null.
	for (int i = 0; i != 100; i++) {
		if (data[i+2] == 0x00)
			break;
		masterId.push_back(data[i+2]);
	}

#ifdef DEBUG_CONVEY_ID
	std::cout << "JvsIo::Jvs_Command_15_ConveyId: " <<
		std::printf("%s", masterId.c_str()) <<
		std::endl;
#endif

	return 2 + masterId.size();
}

uint8_t JvsIo::Jvs_Command_20_ReadSwitchInputs(uint8_t *data)
{
	static jvs_switch_player_inputs_t default_switch_player_input;
	uint8_t nr_switch_players = data[1];
	uint8_t bytesPerSwitchPlayerInput = data[2];

	ResponseBuffer.emplace_back(JvsReportCode::Handled);

	ResponseBuffer.emplace_back(Inputs.switches.system.GetByte0());

	for (int i = 0; i != nr_switch_players; i++) {
		for (int j = 0; j != bytesPerSwitchPlayerInput; j++) {
			// If a title asks for more switch player inputs than we support, pad with dummy data
			jvs_switch_player_inputs_t &switch_player_input = (i >= JVS_MAX_PLAYERS) ? default_switch_player_input : Inputs.switches.player[i];
			uint8_t value
				= (j == 0) ? switch_player_input.GetByte0()
				: (j == 1) ? switch_player_input.GetByte1()
				: 0; // Pad any remaining bytes with 0, as we don't have that many inputs available
			ResponseBuffer.emplace_back(value);
		}
	}

	return 2;
}

uint8_t JvsIo::Jvs_Command_21_ReadCoinInputs(uint8_t *data)
{
	static jvs_coin_slots_t default_coin_slot;
	uint8_t nr_coin_slots = data[1];
	
	ResponseBuffer.emplace_back(JvsReportCode::Handled);

	for (int i = 0; i != nr_coin_slots; i++) {
		const uint8_t bytesPerCoinSlot = 2;
		for (int j = 0; j != bytesPerCoinSlot; j++) {
			// If a title asks for more coin slots than we support, pad with dummy data
			jvs_coin_slots_t &coin_slot = (i >= JVS_MAX_COINS) ? default_coin_slot : Inputs.coins[i];
			uint8_t value
				= (j == 0) ? coin_slot.GetByte0()
				: (j == 1) ? coin_slot.GetByte1()
				: 0; // Pad any remaining bytes with 0, as we don't have that many inputs available
			ResponseBuffer.emplace_back(value);
		}
	}

	return 1;
}

uint8_t JvsIo::Jvs_Command_22_ReadAnalogInputs(uint8_t *data)
{
	static jvs_analog_input_t default_analog;
	uint8_t nr_analog_inputs = data[1];

	ResponseBuffer.emplace_back(JvsReportCode::Handled);

	for (int i = 0; i != nr_analog_inputs; i++) {
		const uint8_t bytesPerAnalogInput = 2;
		for (int j = 0; j != bytesPerAnalogInput; j++) {
			// If a title asks for more analog input than we support, pad with dummy data
			jvs_analog_input_t &analog_input = (i >= JVS_MAX_ANALOG) ? default_analog : Inputs.analog[i];
			uint8_t value
				= (j == 0) ? analog_input.GetByte0()
				: (j == 1) ? analog_input.GetByte1()
				: 0; // Pad any remaining bytes with 0, as we don't have that many inputs available
			ResponseBuffer.emplace_back(value);
		}
	}

	return 1;
}

// TODO: Verify with a test case...
uint8_t JvsIo::Jvs_Command_25_ReadScreenPosition(uint8_t *data)
{
	static jvs_screen_pos_input_t default_screen_pos;
	uint8_t nr_screen_inputs = data[1];

	ResponseBuffer.emplace_back(JvsReportCode::Handled);

	for (int i = 0; i != nr_screen_inputs; i++) {
		const uint8_t bytesPerScreenInput = 4;
		for (int j = 0; j != bytesPerScreenInput; j++) {
			// If a title asks for more screen channels than we support, pad with dummy data
			jvs_screen_pos_input_t &screen_pos_input = (i >= JVS_MAX_SCREEN_CHANNELS) ? default_screen_pos : Inputs.screen[i];
			uint8_t value
				= (j == 0) ? screen_pos_input.GetByte0()
				: (j == 1) ? screen_pos_input.GetByte1()
				: (j == 2) ? screen_pos_input.GetByte2()
				: (j == 3) ? screen_pos_input.GetByte3()
				: 0;
			ResponseBuffer.emplace_back(value);
		}
	}

	return 1;
}

// TODO: Verify with a test case...
uint8_t JvsIo::Jvs_Command_26_ReadGeneralSwitchInputs(uint8_t *data)
{
	uint8_t bytesPerSwitchGeneralInput = data[1]; //?
	jvs_switch_general_inputs_t &switch_general_input = Inputs.switches.general;

	ResponseBuffer.emplace_back(JvsReportCode::Handled);

	for (int j = 0; j != bytesPerSwitchGeneralInput; j++) {
		// If a title asks for more switch player inputs than we support, pad with dummy data
		uint8_t value
			= (j == 0) ? switch_general_input.GetByte0()
			: (j == 1) ? switch_general_input.GetByte1()
			: 0; // Pad any remaining bytes with 0, as we don't have that many inputs available
		ResponseBuffer.emplace_back(value);
	}

	return 2;
}

uint8_t JvsIo::Jvs_Command_30_CoinSubtractionOutput(uint8_t *data)
{
	ResponseBuffer.emplace_back(JvsReportCode::Handled);

	uint8_t slot = data[1] - 1;
	uint16_t decrement = (data[2] << 8) | data[3];

	if (Inputs.coins[slot].coins >= decrement) {
		Inputs.coins[slot].coins -= decrement;
	} else {
		Inputs.coins[slot].coins = 0;
	}

	return 3;
}

uint8_t JvsIo::Jvs_Command_32_GeneralPurposeOutput(uint8_t *data)
{
	uint8_t banks = data[1];

	ResponseBuffer.emplace_back(JvsReportCode::Handled);

#ifdef DEBUG_GENERAL_OUT
	std::cout << "JvsIo::Jvs_Command_32_GeneralPurposeOutput:";
	for (int i = 0; i != banks; i++) {
		if (i <= JVS_MAX_GPO) {
			std::string gpo_pin = std::bitset<8>(data[i+2]).to_string();
			std::printf(" %s", gpo_pin.c_str());
		}
	}
	std::cout << std::endl;
#endif

	return 1 + banks;
}

uint8_t JvsIo::Jvs_Command_35_CoinAdditionOutput(uint8_t *data)
{
	ResponseBuffer.emplace_back(JvsReportCode::Handled);

	uint8_t slot = data[1] - 1;
	uint16_t increment = (data[2] << 8) | data[3];
	uint32_t total = Inputs.coins[data[1]].coins + increment;

	if (total <= 0x3FFF) {
		Inputs.coins[slot].coins += increment;
	} else {
		Inputs.coins[slot].coins = 0x3FFF;
	}

	return 3;
}

void JvsIo::HandlePacket(std::vector<uint8_t>& packet)
{
	// It's possible for a JVS packet to contain multiple commands, so we must iterate through it
	ResponseBuffer.emplace_back(JvsStatusCode::StatusOkay); // Assume we'll handle the command just fine

	for (size_t i = 0; i != packet.size(); i++) {
		uint8_t *command_data = &packet.at(i);
		switch (packet.at(i)) {
			// Broadcast Commands
			case 0xF0: i += Jvs_Command_F0_Reset(command_data); break;
			case 0xF1:
				if (DeviceID != 0) {
					// TODO: Refactor so we can just ignore things like this, for now
					// clear out the buffer so we cause a check in JvsIo::SendPacket to
					// fail so we don't send out a broken packet.
					ResponseBuffer.clear();
					return;
				}
				i += Jvs_Command_F1_SetDeviceId(command_data);
			break;
			// Init Commands
			case 0x10: i += Jvs_Command_10_GetBoardId(); break;
			case 0x11: i += Jvs_Command_11_GetCommandFormat(); break;
			case 0x12: i += Jvs_Command_12_GetJvsRevision(); break;
			case 0x13: i += Jvs_Command_13_GetCommunicationVersion(); break;
			case 0x14: i += Jvs_Command_14_GetCapabilities(); break;
			case 0x15: i += Jvs_Command_15_ConveyId(command_data); break;
			case 0x20: i += Jvs_Command_20_ReadSwitchInputs(command_data); break;
			case 0x21: i += Jvs_Command_21_ReadCoinInputs(command_data); break;
			case 0x22: i += Jvs_Command_22_ReadAnalogInputs(command_data); break;
			case 0x25: i += Jvs_Command_25_ReadScreenPosition(command_data); break;
			case 0x26: i += Jvs_Command_26_ReadGeneralSwitchInputs(command_data); break;
			case 0x30: i += Jvs_Command_30_CoinSubtractionOutput(command_data); break;
			case 0x32: i += Jvs_Command_32_GeneralPurposeOutput(command_data); break;
			case 0x35: i += Jvs_Command_35_CoinAdditionOutput(command_data); break;
			default:
				// Overwrite the verly-optimistic JvsStatusCode::StatusOkay with Status::Unsupported command
				// Don't process any further commands. Existing processed commands must still return their responses.
				ResponseBuffer[0] = JvsStatusCode::UnsupportedCommand;
				std::printf("JvsIo::HandlePacket: Unhandled Command %02X", packet[i]);
				std::cout << std::endl;
				return;
		}
	}
}

// TODO: Slow
uint8_t JvsIo::GetByte(std::vector<uint8_t> &buffer)
{
	uint8_t value = buffer.at(0);
	buffer.erase(buffer.begin());

	return value;
}

uint8_t JvsIo::GetEscapedByte(std::vector<uint8_t> &buffer)
{
	uint8_t value = GetByte(buffer);

	// Special case: 0xD0 is an exception byte that actually returns the next byte + 1
	if (value == ESCAPE_BYTE) {
		value = GetByte(buffer) + 1;
	}

	return value;
}

JvsIo::Status JvsIo::ReceivePacket(std::vector<uint8_t> &buffer)
{
	// First, read the sync byte
	if (GetByte(buffer) != SYNC_BYTE) { // Do not unescape the sync-byte!
#ifdef DEBUG_JVS_PACKETS
		std::cerr << "JvsIo::ReceivePacket: Missing sync byte!\n";
#endif
		return Status::SyncError;
	}

	uint8_t target = GetEscapedByte(buffer);
	if (target != TARGET_BROADCAST && target != DeviceID) {
		return Status::WrongTarget;
	}

	// Miscount can happen if we read too fast or we start *after* master already has a slave.
	uint8_t count = GetEscapedByte(buffer);
	if (count != buffer.size()) {
#ifdef DEBUG_JVS_PACKETS
		std::cerr << "JvsIo::ReceivePacket: Count was incorrect, ignoring.\n";
#endif
		return Status::CountError;
	}

	// Calculate the checksum
	uint8_t actual_checksum = target + count;

	// Decode the payload data
	// TODO: don't put in another vector just to send off
	std::vector<uint8_t> packet;
	packet.reserve(512);
	for (int i = 0; i != count - 1; i++) { // NOTE: -1 to avoid adding the checksum byte to the packet
		uint8_t value = GetEscapedByte(buffer);
		packet.emplace_back(value);
		actual_checksum += value;
	}

	// Read the checksum from the last byte
	uint8_t packet_checksum = GetEscapedByte(buffer);

	// Verify checksum - skip packet if invalid
	ResponseBuffer.clear();
	if (packet_checksum != actual_checksum) {
		ResponseBuffer.emplace_back(JvsStatusCode::ChecksumError);
		return Status::SumError;
	}

	HandlePacket(packet);

	return Status::Okay;
}

void JvsIo::SendByte(std::vector<uint8_t> &buffer, uint8_t value)
{
	buffer.emplace_back(value);
}

void JvsIo::SendEscapedByte(std::vector<uint8_t> &buffer, uint8_t value)
{
	// Special case: Send an exception byte followed by value - 1
	if (value == SYNC_BYTE || value == ESCAPE_BYTE) {
		SendByte(buffer, ESCAPE_BYTE);
		value--;
	}

	SendByte(buffer, value);
}

JvsIo::Status JvsIo::SendPacket(std::vector<uint8_t> &buffer)
{
	// This shouldn't happen...
	if (ResponseBuffer.empty()) {
		return Status::EmptyResponse;
	}

	// TODO: What if count overflows (meaning : responses are bigger than 255 bytes); Should we split it over multiple packets?
	// Send the header bytes
	SendByte(buffer, SYNC_BYTE); // Do not escape the sync byte!
	SendEscapedByte(buffer, TARGET_MASTER);
	SendEscapedByte(buffer, (uint8_t)ResponseBuffer.size() + 1);

	// Calculate the checksum, normally you would add the target, but we only talk to TARGET_MASTER
	uint8_t packet_checksum = (uint8_t)ResponseBuffer.size() + 1;

	// Encode the payload data
	for (uint8_t n : ResponseBuffer) {
		SendEscapedByte(buffer, n);
		packet_checksum += n;
	}

	// Write the checksum to the last byte
	SendEscapedByte(buffer, packet_checksum);

	ResponseBuffer.clear();

#ifdef DEBUG_JVS_PACKETS
	std::cout << "JvsIo::SendPacket:";
	for (uint8_t n : buffer) {
		std::printf(" %02X", n);
	}
	std::cout << std::endl;
#endif

	return Status::Okay;
}
