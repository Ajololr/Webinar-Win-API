#include "TextBox.h"

TextBox::TextBox(const HWND & parent, TCHAR * strButtonName, const int &x, const int &y, const int &width, const int &height,
		const int &id, const DWORD params)
{
	WindowControl::_hwndWindow = CreateWindow(
		TEXT("Edit"),
		strButtonName,
		params,
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
		throw std::exception("Can't create edit");
	}

	WindowControl::_iID = id;

	WindowControl::_strWindowName = reinterpret_cast<wchar_t>(strButtonName);
}

TextBox::~TextBox()
{
}
