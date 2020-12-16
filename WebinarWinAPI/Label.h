#pragma once
#include "BaseControl.h"

namespace webinar
{
	class Label : public WindowControl
	{

	public:
		Label(const HWND& parent, TCHAR* strLabelName, const int& x, const int& y, const int& width, const int& height, const int& id);
		virtual ~Label();
	};
}