#include "stdafx.h"
#include "Resource.h"
#include "Network.h"
using namespace std;

Network *Network::self = 0;

Network::Network(Scheduler *s) noexcept :
scheduler(s), sockets(CreateThreadpool(0)) {
	self = this;
	SetThreadpoolThreadMaximum(sockets, 8);
	InitializeThreadpoolEnvironment(&sockets_cbe);
	SetThreadpoolCallbackRunsLong(&sockets_cbe);
	SetThreadpoolCallbackPool(&sockets_cbe, sockets);
}

bool Network::init(const char *ip) noexcept {
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(3333);
	if ((addr.sin_addr.S_un.S_addr = inet_addr(ip)) == INADDR_NONE) {
		MessageBoxW(hwnd, L"Invalid IP Address", 0, MB_OK);
		return false;
	}

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)){
		MessageBoxW(hwnd, L"WSAStartup failed", 0, MB_OK);
		return false;
	}

	svr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (svr == INVALID_SOCKET) {
		MessageBoxW(hwnd, L"Invalid socket", 0, MB_OK);
		return false;
	}

	if (bind(svr, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in))) {
		wchar_t buf[20];
		wsprintfW(buf, L"bind failed %u", GetLastError());
		MessageBoxW(hwnd, buf, 0, MB_OK);
		return false;
	}

	if (listen(svr, SOMAXCONN)){
		MessageBoxW(hwnd, L"Unable to listen", 0, MB_OK);
		return false;
	}
	
	if (!CreateThread(0, 0, start, 0, 0, 0))
		return false;

	return true;
}

unsigned long _stdcall Network::start(void *) noexcept {
	SOCKET clt;
	while (clt = accept(self->svr, 0, 0)) {
		PTP_WORK work = CreateThreadpoolWork(serve, reinterpret_cast<void *>(clt), &self->sockets_cbe);
		SubmitThreadpoolWork(work);
	}
	return 0;
}

void Network::serve(PTP_CALLBACK_INSTANCE, PVOID context, PTP_WORK) {
	SOCKET clt = reinterpret_cast<SOCKET>(context);
	char buf[8];
	while (recv(clt, buf, sizeof(buf), 0) > 0) {
		unsigned int start = *(reinterpret_cast<unsigned int *>(buf));
		unsigned int duration = *(reinterpret_cast<unsigned int *>(buf + 4));
		pair<unsigned int, unsigned int> p = self->scheduler_poster(start, duration);
		char * temp = reinterpret_cast<char *>(&p);
		send(clt, temp, sizeof(p), 0);
	}
	closesocket(clt);
}

pair<unsigned int, unsigned int> Network::scheduler_poster(unsigned int start, unsigned int duration) noexcept {
	pair<unsigned int, unsigned int> p = self->scheduler->new_task(start, duration);
	PostMessage(hwnd, WM_COMMAND, COLOR_NODE, p.first << 16 | p.second);
	return p;
}


Network::~Network() {
	closesocket(svr);
	CloseThreadpool(sockets);
	WSACleanup();
}

