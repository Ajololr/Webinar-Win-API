#pragma once
#include "BaseControl.h"

namespace webinar
{
	class CreateRoomWindow : public WindowControl
	{
		WNDCLASSEX _wndClass;

		static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	public:
		CreateRoomWindow(TCHAR* strWindowClassName, HINSTANCE hInstance, int x, int y, int width, int heigth, int iShowWindow);
		virtual ~CreateRoomWindow();

	};
}