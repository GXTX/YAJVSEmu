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

#ifndef JVSIO_H
#define JVSIO_H

#include <vector>
#include <iostream>

#define JVS_MAX_PLAYERS (2)
#define JVS_MAX_ANALOG (8)
#define JVS_MAX_COINS (JVS_MAX_PLAYERS)
#define JVS_MAX_SCREEN_CHANNELS (1)
#define JVS_MAX_GPO (6)

struct jvs_switch_system_input {
	bool test{};
	bool tilt_1{};
	bool tilt_2{};
	bool tilt_3{};

	uint8_t GetByte0() {
		uint8_t value{};
		value |= test   ? 1 << 7 : 0;
		value |= tilt_1 ? 1 << 6 : 0;
		value |= tilt_2 ? 1 << 5 : 0;
		value |= tilt_3 ? 1 << 4 : 0;
		return value;
	}
};

struct jvs_switch_player_input{
	bool start{};
	bool service{};
	bool up{};
	bool down{};
	bool left{};
	bool right{};
	bool button[10]{};

	uint8_t GetByte0() {
		uint8_t value{};
		value |= start     ? 1 << 7 : 0;
		value |= service   ? 1 << 6 : 0;
		value |= up        ? 1 << 5 : 0;
		value |= down      ? 1 << 4 : 0;
		value |= left      ? 1 << 3 : 0;
		value |= right     ? 1 << 2 : 0;
		value |= button[0] ? 1 << 1 : 0;
		value |= button[1] ? 1 << 0 : 0;
		return value;
	}

	uint8_t GetByte1() {
		uint8_t value{};
		value |= button[2] ? 1 << 7 : 0;
		value |= button[3] ? 1 << 6 : 0;
		value |= button[4] ? 1 << 5 : 0;
		value |= button[5] ? 1 << 4 : 0;
		value |= button[6] ? 1 << 3 : 0;
		value |= button[7] ? 1 << 2 : 0; //
		value |= button[8] ? 1 << 1 : 0; // Unused buttons with Type-1 IO
		value |= button[9] ? 1 << 0 : 0; //
		return value;
	}
};


struct jvs_switch_general_input {
	bool button[16]{};

	uint8_t GetByte0() {
		uint8_t value{};
		value |= button[0] ? 1 << 7 : 0;
		value |= button[1] ? 1 << 6 : 0;
		value |= button[2] ? 1 << 5 : 0;
		value |= button[3] ? 1 << 4 : 0;
		value |= button[4] ? 1 << 3 : 0;
		value |= button[5] ? 1 << 2 : 0;
		value |= button[6] ? 1 << 1 : 0;
		value |= button[7] ? 1 << 0 : 0;
		return value;
	}

	uint8_t GetByte1() {
		uint8_t value{};
		value |= button[8]  ? 1 << 7 : 0;
		value |= button[9]  ? 1 << 6 : 0;
		value |= button[10] ? 1 << 5 : 0;
		value |= button[11] ? 1 << 4 : 0;
		value |= button[12] ? 1 << 3 : 0;
		value |= button[13] ? 1 << 2 : 0;
		value |= button[14] ? 1 << 1 : 0;
		value |= button[15] ? 1 << 0 : 0;
		return value;
	}
};

struct jvs_switch_inputs {
	jvs_switch_system_input system;
	jvs_switch_player_input player[JVS_MAX_PLAYERS];
	jvs_switch_general_input general;
};

struct jvs_analog_input {
	uint16_t value{};

	uint8_t GetByte0() {
		return (value >> 8) & 0xFF;
	}

	uint8_t GetByte1() {
		return value & 0xFF;
	}
};

struct jvs_coin_slot {
	enum CoinStatus{
		Normal = 0,
		Jammed = 1,
		Disconnected = 2,
		Busy = 3,
	};

	uint16_t coins{};
	uint8_t status{CoinStatus::Normal};

	uint8_t GetByte0() {
		return ((coins >> 8) & 0x3F) | ((status << 6) & 0xC0);
	}

	uint8_t GetByte1() {
		return coins & 0xFF;
	}
};

struct jvs_screen_pos_input {
	uint32_t position{};

	uint8_t GetByte0() {
		return (position >> 24) & 0xFF;
	}

	uint8_t GetByte1() {
		return (position >> 16) & 0xFF;
	}

	uint8_t GetByte2() {
		return (position >> 8) & 0xFF;
	}

	uint8_t GetByte3() {
		return position & 0xFF;
	}
};

struct jvs_input_states {
	jvs_switch_inputs switches;
	jvs_analog_input analog[JVS_MAX_ANALOG];
	jvs_coin_slot coins[JVS_MAX_COINS];
	jvs_screen_pos_input screen[JVS_MAX_SCREEN_CHANNELS];
};

class JvsIo
{
public:
	enum class Status {
		Okay,
		SyncError,
		CountError,
		WrongTarget,
		ChecksumError,
		EmptyResponse,
		ServerWaitingReply,
	};

	enum class SenseState {
		Connected,
		NotConnected,
	};

	SenseState pSense{SenseState::NotConnected};
	bool pSenseChange{false};
	jvs_input_states Inputs;

	JvsIo(SenseState sense);

	JvsIo::Status SendPacket(std::vector<uint8_t> &buffer);
	JvsIo::Status ReceivePacket(std::vector<uint8_t> &buffer);
private:
	static const uint8_t SYNC_BYTE{0xE0};
	static const uint8_t ESCAPE_BYTE{0xD0};

	static const uint8_t TARGET_MASTER{0x00};
	static const uint8_t TARGET_BROADCAST{0xFF};

	uint8_t GetByte(uint8_t **buffer);
	uint8_t GetEscapedByte(uint8_t **buffer);

	void HandlePacket(std::vector<uint8_t> &packet);

	void SendByte(std::vector<uint8_t> &buffer, uint8_t value);
	void SendEscapedByte(std::vector<uint8_t> &buffer, uint8_t value);

	enum JvsStatusCode {
		StatusOkay = 1,
		UnsupportedCommand = 2,
		ChecksumError = 3,
		AcknowledgeOverflow = 4,
	};

	enum JvsReportCode {
		Handled = 1,
		NotEnoughParameters = 2,
		InvalidParameter = 3,
		Busy = 4,
	};

	enum JvsCapabilityCode {
		EndOfCapabilities = 0x00,
		// Input capabilities :
		PlayerSwitchButtonSets = 0x01,
		CoinSlots = 0x02,
		AnalogInputs = 0x03,
		RotaryInputs = 0x04, // Params : JVS_MAX_ROTARY, 0, 0
		KeycodeInputs = 0x05,
		ScreenPointerInputs = 0x06, // Params : Xbits, Ybits, JVS_MAX_POINTERS
		SwitchInputs = 0x07,
		// Output capabilities :
		CardSystem = 0x10, // Params : JVS_MAX_CARDS, 0, 0
		MedalHopper = 0x11, // Params : max?, 0, 0
		GeneralPurposeOutputs = 0x12, // Params : number of outputs, 0, 0
		AnalogOutput = 0x13, // Params : channels, 0, 0
		CharacterOutput = 0x14, // Params : width, height, type
		BackupData = 0x15,
	};

	// Commands
	// These return the additional param bytes used
#if 0
	uint8_t Jvs_Command_F0_Reset(uint8_t *data);
#endif
	uint8_t Jvs_Command_F1_SetDeviceId(uint8_t *data);
	uint8_t Jvs_Command_10_GetBoardId();
	uint8_t Jvs_Command_11_GetCommandFormat();
	uint8_t Jvs_Command_12_GetJvsRevision();
	uint8_t Jvs_Command_13_GetCommunicationVersion();
	uint8_t Jvs_Command_14_GetCapabilities();
	uint8_t Jvs_Command_15_ConveyId(uint8_t *data);
	uint8_t Jvs_Command_20_ReadSwitchInputs(uint8_t *data);
	uint8_t Jvs_Command_21_ReadCoinInputs(uint8_t *data);
	uint8_t Jvs_Command_22_ReadAnalogInputs(uint8_t *data);
	uint8_t Jvs_Command_25_ReadScreenPosition(uint8_t *data);
	uint8_t Jvs_Command_26_ReadGeneralSwitchInputs(uint8_t *data);
	uint8_t Jvs_Command_30_CoinSubtractionOutput(uint8_t *data);
	uint8_t Jvs_Command_32_GeneralPurposeOutput(uint8_t *data);
	uint8_t Jvs_Command_35_CoinAdditionOutput(uint8_t *data);
	uint8_t Jvs_Command_70_NamcoSpecific(uint8_t *data);

	std::vector<uint8_t> ResponseBuffer{}; // Command Response
	std::vector<uint8_t> ProcessedPacket{}; // Hold the incoming packet after we've processed

	// Device info
	uint8_t DeviceID{}; // Device ID assigned by running title
	uint8_t CommandFormatRevision{};
	uint8_t JvsVersion{};
	uint8_t CommunicationVersion{};
	std::string BoardID{};
};

#endif
