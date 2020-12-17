#pragma once
#include "WindowControl.h"

namespace webinar
{
	class Button : public WindowControl
	{

	public:
		Button(const HWND & parent, TCHAR * strButtonName, const int &x, const int &y, const int &width, const int &height, const int &id);
		virtual ~Button();
	};
}