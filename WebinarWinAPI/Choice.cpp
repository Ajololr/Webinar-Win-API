#include "Choice.h"
#include "Button.h"
#include <tchar.h>
#include "TextBox.h"
#include <memory>
#include <Winsock.h>
#include <string>
#include "Label.h"
#include "CreateRoomWindow.h"
#include "JoinRoomWindow.h"
#include "RoomWindow.h"
#include "Role.h"

#pragma comment(lib, "Ws2_32.lib")

Role userRole;
std::string userName;

namespace webinar
{

	std::vector<WindowControl*> controls;

	bool ClickJoin(WPARAM, LPARAM);

	bool ClickCreate(WPARAM, LPARAM);

	HINSTANCE hwndChoice = NULL;

	Choice::Choice(TCHAR* strWindowClassName, HINSTANCE hInstance, int x, int y, int width, int heigth, int iShowWindow)
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

		hwndChoice = (HINSTANCE)_hwndWindow;

		ShowWindow(WindowControl::_hwndWindow, iShowWindow);
		UpdateWindow(WindowControl::_hwndWindow);

		WindowControl::_strWindowName = reinterpret_cast<wchar_t>(strWindowClassName);

	}


	Choice::~Choice()
	{
		WSACleanup();
	}

	LRESULT Choice::WndProc(HWND hWnd,
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
				controls.push_back(new Label(hWnd, _T("Welcome to Webinar!"), 100, 15, 150, 20, 1));
				controls.push_back(new Button(hWnd, _T("Create"), 130, 80, 80, 25, 2));
				controls.push_back(new Button(hWnd, _T("Connect"), 130, 130, 80, 25, 3));


				controls[1]->SetEvent(ClickCreate, WM_COMMAND);
				controls[2]->SetEvent(ClickJoin, WM_COMMAND);

				return 0;
			}
			case WM_COMMAND:

				for (int i = 0; i < controls.size(); i++)
				{
					if (controls[i]->GatID() == LOWORD(wParam))
					{
						controls[i]->EventStart(uMsg, wParam, lParam);
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

	bool ClickCreate(WPARAM wParam, LPARAM lParam)
	{
		userRole = TEACHER;
		webinar::CreateRoomWindow createRoomWindow(L"Create room", hwndChoice, 100, 100, 380, 300, 1);
		ShowWindow((HWND)hwndChoice, SW_HIDE);
		return true;
	}


	bool ClickJoin(WPARAM wParam, LPARAM lParam)
	{
		try
		{
			userRole = STUDENT;
			webinar::JoinRoomWindow joinRoomWindow(L"Join room", hwndChoice, 100, 100, 380, 300, 1);
			ShowWindow((HWND)hwndChoice, SW_HIDE);
		}
		catch (std::exception ex)
		{

			MessageBoxA(NULL, ex.what(), NULL, MB_ICONERROR);
			return false;
		}
		return true;
	}
}