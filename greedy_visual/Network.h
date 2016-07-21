#pragma once
#include "stdafx.h"
#include "WinSock2.h"
#include "Scheduler.h"
#pragma comment(lib, "ws2_32.lib")

class Network {
public:
	Network(Scheduler* ) noexcept;
	bool init(const char *ip) noexcept;	
	~Network() noexcept;

	Network(const Network &) = delete;
	Network(const Network &&) = delete;
	
private:
	static Network *self;
	Scheduler *scheduler;
	PTP_POOL sockets;
	TP_CALLBACK_ENVIRON_V3 sockets_cbe;
	SOCKET svr;
	static unsigned long _stdcall start(void *) noexcept;
	static void _stdcall serve(PTP_CALLBACK_INSTANCE, PVOID, PTP_WORK);
	std::pair<unsigned int, unsigned int> _stdcall scheduler_poster(unsigned int, unsigned int) noexcept;
};

