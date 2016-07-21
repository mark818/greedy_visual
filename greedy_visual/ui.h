#pragma once

#include "resource.h"
#include "stdafx.h"
#include "Scheduler.h"
#include "Network.h"

class UI {
public:
	UI(HINSTANCE hinst) noexcept;
	UI(const UI&) = delete;
	UI(const UI&&) = delete;
	~UI();
	void run();

private:
	struct node {
		explicit node() noexcept {}
		node(RECT && rect) noexcept {
			this->rect = { rect.left, rect.top, rect.right, rect.bottom };
		}
		RECT rect;
	};
	static LRESULT __stdcall WndProc(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR __stdcall About(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR __stdcall Set(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR __stdcall Assign(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR __stdcall Tcp(HWND, UINT, WPARAM, LPARAM);
	static void _stdcall scheduler_poster(PTP_CALLBACK_INSTANCE, LPVOID, PTP_WORK) noexcept;
	unsigned int nodes = 506, increment = 20;
	Scheduler scheduler;
	PTP_POOL tp;
	TP_CALLBACK_ENVIRON_V3 cbe;
	Network network;
	std::vector<node> points;
	HINSTANCE hinst;
	static UI *self;
};