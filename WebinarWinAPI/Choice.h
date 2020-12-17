#pragma once
#include "WindowControl.h"

namespace webinar
{

	class Choice : public WindowControl
	{
		WNDCLASSEX _wndClass;

		static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	public:
		Choice(TCHAR* strWindowClassName, HINSTANCE hInstance, int x, int y, int width, int heigth, int iShowWindow);
		virtual ~Choice();

	};
}