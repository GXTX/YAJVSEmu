
#include "SerIo.h"

#include <iostream>
#include <cstdint>
#include <cstring>

SerIo* g_pSerIo;

SerIo::SerIo(char *devicePath)
{
	Init(devicePath);
}

SerIo::~SerIo()
{
	close(SerialHandler);
}

// TODO: Move this back in the constructor, we don't care about return val
void SerIo::Init(char *devicePath)
{
	SerialHandler = open(devicePath, O_RDWR | O_NOCTTY | O_SYNC | O_NDELAY);

	if (SerialHandler < 0) {
		std::printf("SerIo::Init - Failed to open %s.\n", devicePath);
		IsInitialized = false;
	}
	else {
		IsInitialized = true;
		SetAttributes(SerialHandler, B115200);
		SetLowLatency(SerialHandler);
	}
}

int SerIo::Write(uint8_t *write_buffer, uint8_t bytes_to_write)
{
	std::printf("sending 0x%02X\n", bytes_to_write);
	for(uint8_t i = 0; i < sizeof(write_buffer); i++) {
		std::printf(" %02X", write_buffer[i]);
	}
	std::cout << "\n" << std::endl;

	int ret = write(SerialHandler, write_buffer, sizeof(write_buffer));

	if (ret != bytes_to_write) {
		std::printf("SerIo::Write: Only wrote %02X of %02X to the port!\n", ret, bytes_to_write);
		return 0;
	}

	return 1;
}

int SerIo::Read(uint8_t *read_buffer)
{
	fd_set fd_serial;
	struct timeval tv;

	int serial = SerialHandler;

	FD_ZERO(&fd_serial);
	FD_SET(SerialHandler, &fd_serial);

	/* set blocking timeout to TIMEOUT_SELECT */
	tv.tv_sec = 0;
	tv.tv_usec = 500 * 1000;

	int asd = select(serial + 1, &fd_serial, NULL, NULL, &tv);

	if (asd == 0) {
		return StatusCode::SerialTimeout;
	}
	else if (asd > 0) {
		if (!FD_ISSET(serial, &fd_serial)) {
			return StatusCode::SerialTimeout;
		}
		else {
			// TODO: Assume this is okay?
		}
	}
	else {
		return StatusCode::SerialReadError;
	}

	uint8_t temp_buffer[256];
	int bytes,n;
	
	ioctl(serial, FIONREAD, &bytes);

	if (!bytes) {
		return 0;
	}

	n = read(serial, read_buffer, bytes);

	if (n < 0) {
		// TODO: would n ever be less than 0?
	}
	else if (n == 0) {
		// TODO: Why would n be equal to 0 if select told us we're good?
	}
	else {
		//std::cout << "SerIo::Read:";
		//for (uint8_t i = 0; i < n; i++) {
		//	read_buffer[i] = temp_buffer[i]; // wouldn't it be better to just memcpy?
		//std::memcpy(read_buffer, &temp_buffer, bytes);
		//	std::printf(" %02X", read_buffer[i]);
		//}
		//std::cout << "\n";
	}

	return 0;
}

int SerIo::SetAttributes(int SerialHandler, int baud)
{
	struct termios options;
	int status;
	tcgetattr(SerialHandler, &options);

	cfmakeraw(&options);
	cfsetispeed(&options, baud);
	cfsetospeed(&options, baud);

	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;

	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 0; // Ten seconds (100 deciseconds)

	tcsetattr(SerialHandler, TCSANOW, &options);

	ioctl(SerialHandler, TIOCMGET, &status);

	status |= TIOCM_DTR;
	status |= TIOCM_RTS;

	ioctl(SerialHandler, TIOCMSET, &status);

	// Why are we sleeping?
	usleep(100 * 1000); // 10mS

	return 0;
}

int SerIo::SetLowLatency(int SerialHandler)
{
	struct serial_struct serial_settings;

	if (ioctl(SerialHandler, TIOCGSERIAL, &serial_settings) < 0) {
		std::cout << "SerIo::SetLowLatency - Failed to read serial settings for low latency mode.\n";
		return 0;
	}

	serial_settings.flags |= ASYNC_LOW_LATENCY;
	if (ioctl(SerialHandler, TIOCSSERIAL, &serial_settings) < 0) {
		std::cout << "SerIo::SetLowLatency - Failed to write serial settings for low latency mode.\n";
		return 0;
	}

	return 1;
}