#ifndef SERIO_H
#define SERIO_H

#include <iostream>
#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

class SerIo
{
public:
	enum StatusCode {
		StatusOkay,
		SerialTimeout,
		SerialReadError,
	};

	bool IsInitialized;

	SerIo(char *devicePath);
	~SerIo();

	int Read(std::vector<uint8_t> &buffer);
	int Write(std::vector<uint8_t> &buffer);
private:
	int SerialHandler;

	void SetAttributes(int SerialHandler, int baud);
	int SetLowLatency(int SerialHandler);
};

#endif
