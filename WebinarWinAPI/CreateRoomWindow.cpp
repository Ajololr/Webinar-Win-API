#include <tchar.h>
#include <memory>
#include <Winsock.h>
#include <string>
#include "Label.h"
#include "TextBox.h"
#include "Button.h"
#include "CreateRoomWindow.h"
#include "RoomWindow.h"
#include "Role.h"

#pragma comment(lib, "Ws2_32.lib")

namespace webinar
{
	std::vector<WindowControl*> createWindowControls;

	bool ClickCreateRoom(WPARAM, LPARAM);

	HINSTANCE hwndCRW = NULL;

	CreateRoomWindow::CreateRoomWindow(TCHAR* strWindowClassName, HINSTANCE hInstance, int x, int y, int width, int heigth, int iShowWindow)
	{
		_wndClass.cbSize = sizeof(_wndClass);
		_wndClass.style = CS_HREDRAW | CS_VREDRAW;
		_wndClass.lpfnWndProc = WndProc;
		_wndClass.lpszMenuName = nullptr;
		_wndClass.lpszClassName = strWindowClassName;
		_wndClass.cbWndExtra = NULL;
		_wndClass.cbClsExtra = NULL;
		_wndClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
		_wndClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);
		_wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		_wndClass.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
		_wndClass.hInstance = hInstance;

		if (!RegisterClassEx(&_wndClass))
		{
			throw std::exception("Window Class Initial Error");
		}

		WindowControl::_hwndWindow = CreateWindow(_wndClass.lpszClassName, // имя класса
			strWindowClassName,
			DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU,
			x, y, width, heigth,
			HWND(NULL),
			NULL,
			HINSTANCE(hInstance),
			NULL);

		if (!WindowControl::_hwndWindow)
		{
			throw std::exception("Window Cerate Error");
		}

		hwndCRW = (HINSTANCE)_hwndWindow;

		ShowWindow(WindowControl::_hwndWindow, iShowWindow);
		UpdateWindow(WindowControl::_hwndWindow);

		WindowControl::_strWindowName = reinterpret_cast<wchar_t>(strWindowClassName);

	}


	CreateRoomWindow::~CreateRoomWindow()
	{
		WSACleanup();
	}

	LRESULT CreateRoomWindow::WndProc(HWND hWnd, // дескриптор окна
		UINT uMsg, // сообщение, посылаемое ОС
		WPARAM wParam, // параметры
		LPARAM lParam) // сообщений, для последующего обращения
	{
		try
		{
			switch (uMsg)
			{
			case WM_CREATE:
			{
				createWindowControls.push_back(new Label(hWnd, _T("Create room"), 140, 15, 50, 20, 1));
				createWindowControls.push_back(new Label(hWnd, _T("Name"), 100, 50, 75, 20, 2));
				createWindowControls.push_back(new TextBox(hWnd, _T(""), 100, 80, 140, 20, 3));
				createWindowControls.push_back(new Label(hWnd, _T("Password"), 100, 110, 75, 20, 4));
				createWindowControls.push_back(new TextBox(hWnd, _T(""), 100, 140, 140, 20, 5));
				createWindowControls.push_back(new Button(hWnd, _T("Start webinar"), 115, 190, 100, 25, 6));


				createWindowControls[5]->SetEvent(ClickCreateRoom, WM_COMMAND);

				return 0;
			}
			case WM_COMMAND:

				for (int i = 0; i < createWindowControls.size(); i++)
				{
					if (createWindowControls[i]->GatID() == LOWORD(wParam))
					{
						createWindowControls[i]->EventStart(uMsg, wParam, lParam);
					}
				}

				return 0;

			case WM_DESTROY:

				PostQuitMessage(NULL);

				return 0;

			default:
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
			}
		}
		catch (std::exception ex)
		{

			MessageBoxA(NULL, ex.what(), NULL, MB_ICONERROR);

			PostQuitMessage(-1);
			return -1;
		}
	}

	bool ClickCreateRoom(WPARAM wParam, LPARAM lParam)
	{
		userName.resize(20);
		GetWindowTextA(createWindowControls[2]->GetHandler(), &userName[0], 11);

		if (!strlen(&userName[0]))
		{
			MessageBoxA(NULL, "Enter name", NULL, MB_ICONERROR);
			return false;
		}

		webinar::RoomWindow room(L"Teacher room", hwndCRW, 100, 20, 1335, 660, 1);
		ShowWindow((HWND)hwndCRW, SW_HIDE);
		return true;
	}
}