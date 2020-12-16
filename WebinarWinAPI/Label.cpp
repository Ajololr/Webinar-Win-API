#include "Label.h"


namespace webinar
{
	Label::Label(const HWND& parent, TCHAR* strLabelName, const int& x, const int& y, const int& width, const int& height, const int& id)
	{
		WindowControl::_hwndWindow = CreateWindow(
			TEXT("static"),
			strLabelName,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			x,        
			y,              
			width,        
			height,      
			parent,     
			reinterpret_cast<HMENU>(id),      
			GetModuleHandle(NULL),   
			nullptr        
		);

		if (!WindowControl::_hwndWindow)
		{
			throw std::exception("Can't create label");
		}

		WindowControl::_iID = id;

		WindowControl::_strWindowName = reinterpret_cast<wchar_t>(strLabelName);
	}

	Label::~Label()
	{
	}
}