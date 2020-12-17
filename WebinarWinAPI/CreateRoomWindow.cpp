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
#include "Reg.h"
#include "Convert.h"

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

		WindowControl::_hwndWindow = CreateWindow(_wndClass.lpszClassName,
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

	LRESULT CreateRoomWindow::WndProc(HWND hWnd, 
		UINT uMsg, 
		WPARAM wParam, 
		LPARAM lParam) 
	{
		try
		{
			switch (uMsg)
			{
			case WM_CREATE:
			{
				LPCSTR strResult = ReadStringFromRegistry(HKEY_CURRENT_USER, SUB_KEY, KEY_NAME);

				if (!strResult) {
					strResult = "";
				}
				createWindowControls.push_back(new Label(hWnd, _T("Ваше имя:"), 100, 50, 75, 20, 2));
				createWindowControls.push_back(new TextBox(hWnd, &Convert::StrToWStr(strResult)[0], 100, 80, 140, 20, 3));
				createWindowControls.push_back(new Button(hWnd, _T("Создать комнату"), 100, 190, 130, 25, 6));

				createWindowControls[2]->SetEvent(ClickCreateRoom, WM_COMMAND);

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
		GetWindowTextA(createWindowControls[1]->GetHandler(), &userName[0], 11);

		if (!strlen(&userName[0]))
		{
			MessageBoxA(NULL, "Введите имя", NULL, MB_ICONERROR);
			return false;
		}
		WriteStringInRegistry(HKEY_CURRENT_USER, SUB_KEY, KEY_NAME, userName.c_str());

		webinar::RoomWindow room(L"Вебинар", hwndCRW, 100, 20, 1335, 660, 1);
		ShowWindow((HWND)hwndCRW, SW_HIDE);
		return true;
	}
}