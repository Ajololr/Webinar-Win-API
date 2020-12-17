#pragma once
#include "WindowControl.h"

namespace webinar
{
	class RoomWindow : public WindowControl
	{
		WNDCLASSEX _wndClass;

		static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	public:
		RoomWindow(TCHAR *strWindowClassName, HINSTANCE hInstance, int x, int y, int width, int heigth, int iShowWindow);
		virtual ~RoomWindow();

	};
}