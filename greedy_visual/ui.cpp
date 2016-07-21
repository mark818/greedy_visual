// greedy_visual.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ui.h"

#define MAX_LOADSTRING 100

HWND hwnd = 0;
UI *UI::self = 0;

UI::UI(HINSTANCE hinst) noexcept : 
hinst(hinst), scheduler(nodes), network(&scheduler) {
	self = this;

	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hinst;
	wcex.hIcon = LoadIcon(hinst, MAKEINTRESOURCE(IDI_GREEDY_VISUAL));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GREEDY_VISUAL);
	wcex.lpszClassName = L"greedy_visual";
	wcex.hIconSm = wcex.hIcon;
	RegisterClassExW(&wcex);
}

void UI::run() {
	hwnd = CreateWindowW(L"greedy_visual", L"greedy_visual", WS_OVERLAPPEDWINDOW &  (~WS_THICKFRAME) & (~WS_MAXIMIZEBOX), 20, 20, 1520, 820, 0, 0, hinst, 0);
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	PTP_POOL tp = CreateThreadpool(0);
	SetThreadpoolThreadMaximum(tp, 8);
	InitializeThreadpoolEnvironment(&cbe);
	SetThreadpoolCallbackPool(&cbe, tp);

	HACCEL hAccelTable = LoadAccelerators(hinst, MAKEINTRESOURCE(IDC_GREEDY_VISUAL));

	MSG msg;

	// 主消息循环: 
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}


LRESULT __stdcall UI::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HDC memdc = 0;
	static HBITMAP hb = 0;
    switch (message)
    {
	case WM_CREATE: {
		HWND set = CreateWindowW(L"BUTTON", L"设置节点数", WS_CHILD | WS_VISIBLE, 500, 30, 120, 30, hWnd, (HMENU) 111, self->hinst, 0);
		HWND assign = CreateWindowW(L"BUTTON", L"发布任务", WS_CHILD | WS_VISIBLE, 700, 30, 100, 30, hWnd, (HMENU) 222, self->hinst, 0);
		HWND tcp = CreateWindowW(L"BUTTON", L"打开tcp监听", WS_CHILD | WS_VISIBLE, 900, 30, 120, 30, hWnd, (HMENU)333, self->hinst, 0);
		HDC hdc = GetDC(hWnd);
		memdc = CreateCompatibleDC(hdc);
		hb = CreateCompatibleBitmap(hdc, 1500, 800);
		ReleaseDC(hWnd, hdc);
		SelectObject(memdc, hb);
		SetBkMode(memdc, TRANSPARENT);
		PostMessageW(hWnd, WM_COMMAND, static_cast<WPARAM>(RESET), 0);
		break;
	}
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
			case 111:
				DialogBoxW(self->hinst, MAKEINTRESOURCE(IDD_SET), hwnd, Set);
				break;
			case 222:
				DialogBoxW(self->hinst, MAKEINTRESOURCE(IDD_ASSIGN), hwnd, Assign);
				break;
			case 333:
				DialogBoxW(self->hinst, MAKEINTRESOURCE(IDD_TCP), hwnd, Tcp);
				break;
			case RESET: {
				self->points.clear();
				self->points.reserve(self->nodes);
				RECT rect;
				GetClientRect(hWnd, &rect);
				HBRUSH white = CreateSolidBrush(RGB(255, 255, 255));
				HBRUSH green = CreateSolidBrush(RGB(0, 255, 0)); 
				FillRect(memdc, &rect, white);
				DeleteObject(white);
				HPEN pen = CreatePen(1, 2, 0);
				SelectObject(memdc, pen);
				int root = static_cast<int>(sqrt(self->nodes));
				int hor = root;
				int ver = self->nodes/ hor;
				int remainder = self->nodes - hor * ver;
				if (remainder) {
					ver++;
					for (int y = 100, lines = 0; lines < ver; y += 600 / ver, lines++) {
						if (lines == ver - 1) {
							for (int x = 100, cnt = 0; cnt < remainder; x += 1300 / hor, cnt++) {
								Rectangle(memdc, x - 1, y - 1, x + 1400 / hor / 2 + 2, y + 600 / ver - 3);
								RECT rect = { x, y, x + 1400 / hor / 2, y + 600 / ver - 5 };
								FillRect(memdc, &rect, green);
								self->points.push_back(std::move(rect));
							}
							break;
						}
						for (int x = 100; x < 1400 + 1 - 1300 / hor; x += 1300 / hor) {
							Rectangle(memdc, x - 1, y - 1, x + 1400 / hor / 2 + 2, y + 600 / ver - 3);
							RECT rect = { x, y, x + 1400 / hor / 2, y + 600 / ver - 5 };
							FillRect(memdc, &rect, green);
							self->points.push_back(std::move(rect));
						}
					}
				} else {
					// + 1 to account for floating point precision
					for (int y = 100; y < 700 + 1 - 600 / ver; y += 600 / ver)
						// + 1 to account for floating point precision
						for (int x = 100; x < 1400 + 1 - 1300 / hor; x += 1300 / hor) {
							Rectangle(memdc, x - 1, y - 1, x + 1400 / hor / 2 + 2, y + 600 / ver - 3);
							RECT rect = { x, y, x + 1400 / hor / 2, y + 600 / ver - 5 };
							FillRect(memdc, &rect, green);
							self->points.push_back(std::move(rect));
						}
				}
				DeleteObject(pen);
				DeleteObject(green);
				InvalidateRect(hWnd, &rect, true);
				break;
			}
			case COLOR_NODE: {
				unsigned int index = HIWORD(lParam);
				unsigned int num = LOWORD(lParam);
				node &cur = self->points[index];
				unsigned char r = min(20 * num, 255);
				HBRUSH b = CreateSolidBrush(RGB(r, 255-r, 0));
				FillRect(memdc, &cur.rect, b);
				DeleteObject(b);
				std::wstring text = std::to_wstring(num);
				TextOutW(memdc, cur.rect.left, cur.rect.top, text.c_str(), static_cast<int>(text.length())); 
				InvalidateRect(hWnd, &cur.rect, false);
				goto direct;
				break;
			}
            case IDM_ABOUT:
                DialogBox(self->hinst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, reinterpret_cast<DLGPROC>(About));
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
direct:
			PAINTSTRUCT ps;
			RECT rect;
			GetClientRect(hWnd, &rect);
			HDC hdc = BeginPaint(hWnd, &ps);
			BitBlt(hdc, 0, 0, rect.right, rect.bottom, memdc, 0, 0, SRCCOPY);
			EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


INT_PTR __stdcall UI::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR __stdcall UI::Set(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
			EndDialog(hDlg, LOWORD(wParam));
		else if (LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			int translated;
			unsigned int nodes = GetDlgItemInt(hDlg, NODES, &translated, false);
			nodes = max(9, min(nodes, 506));
			self->nodes = nodes;
			self->scheduler.reset(nodes);
			SendMessage(hwnd, WM_COMMAND, RESET, 0);
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR __stdcall UI::Assign(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
			EndDialog(hDlg, LOWORD(wParam));
		else if (LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			int translated;
			unsigned num = GetDlgItemInt(hDlg, TASK, &translated, false);
			if (!num)
				return 0;
			unsigned int *data = new unsigned int[3];
			data[0] = GetDlgItemInt(hDlg, NODES, &translated, false);
			data[1] = GetDlgItemInt(hDlg, DURATION, &translated, false);
			data[2] = 0;
			PTP_WORK work = CreateThreadpoolWork(scheduler_poster, data, &self->cbe);
			for (unsigned int i = 0; i < num; i++)
				SubmitThreadpoolWork(work);
			CloseThreadpoolWork(work);
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR UI::Tcp(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
			EndDialog(hDlg, LOWORD(wParam));
		else if (LOWORD(wParam) == IDOK) {
			EndDialog(hDlg, LOWORD(wParam));
			char ip[16];
			GetDlgItemTextA(hDlg, IP, ip, 15);
			self->network.init(ip);
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void UI::scheduler_poster(PTP_CALLBACK_INSTANCE, LPVOID context, PTP_WORK) noexcept {
	unsigned int *data = reinterpret_cast<unsigned int*>(context);
	unsigned int start = data[0], duration = data[1];
	std::pair<unsigned int, unsigned int> p = self->scheduler.new_task(start, duration);
	PostMessage(hwnd, WM_COMMAND, COLOR_NODE, p.first << 16 | p.second);
}

UI::~UI() {
	CloseThreadpool(tp);
}
