#include "Button.h"


namespace webinar
{
	Button::Button(const HWND & parent,TCHAR * strButtonName, const int &x, const int &y, const int &width, const int &height,const int &id)
	{
		WindowControl::_hwndWindow = CreateWindow(
			TEXT("Button"),  
			strButtonName,  
			WS_CHILD | WS_VISIBLE,
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
			throw std::exception("Can't create button");
		}

		WindowControl::_iID = id;

		WindowControl::_strWindowName = reinterpret_cast<wchar_t>(strButtonName);
	}

	Button::~Button()
	{
	}
}