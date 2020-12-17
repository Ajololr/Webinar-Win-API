#include <windows.h>
#include <tchar.h>
#include <exception>

#include "Choice.h"
#include "Reg.h"

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	try
	{
		CreateRegistryKey(HKEY_CURRENT_USER, SUB_KEY);

		MSG msg;

		webinar::Choice ChoiceWindow(_T("Онлайн-вебинар"), hInstance, 100, 100, 380, 300, nCmdShow);

		BOOL bRet;

		while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
		{
			if (bRet == -1)
			{
				throw std::exception("Critical exception in Callback Function");
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		return msg.wParam;
	}
	catch (std::exception ex)
	{
		MessageBoxA(NULL, ex.what(), "Initial Error", MB_OK);
		return -1;
	}
	catch (...)
	{
		MessageBoxA(NULL, "Undefined Exception", "Critical ERROR", MB_OK);
		return -1;
	}
}