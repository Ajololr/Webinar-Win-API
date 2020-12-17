#include "Reg.h"

using namespace std;

	void CreateRegistryKey(HKEY key, LPWSTR subkey)
	{
		DWORD dwDisposition;
		HKEY  hKey;
		DWORD Ret;
		Ret =
			RegCreateKeyEx(key, subkey, 0,
				NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
		if (Ret != ERROR_SUCCESS)
		{
			printf("������ �������� �����\n");
		}
		else
			printf("���� ������\n");
		RegCloseKey(hKey);
	}

	void WriteStringInRegistry(HKEY key, LPWSTR subkey, LPWSTR valueName, LPCSTR value)
	{
		DWORD disposition;
		HKEY openedKey;
		DWORD result;
		result = RegOpenKeyEx(key, subkey, 0, KEY_WRITE, &openedKey);
		if (result != ERROR_SUCCESS)
			printf("������ ������� ����");
		int l = strlen(value);
		BYTE* b = (LPBYTE)value;
		result = RegSetValueEx(openedKey, valueName, 0, REG_SZ, (LPBYTE)value, strlen(value) + 1);
		if (result != ERROR_SUCCESS)
			printf("������ ���������� ������");
		RegCloseKey(openedKey);
	}

	LPCSTR ReadStringFromRegistry(HKEY key, LPWSTR subkey, LPWSTR valueName)
	{
		DWORD result;
		HKEY openedKey;
		result = RegOpenKeyEx(key, subkey, 0, KEY_READ, &openedKey);
		if (result != ERROR_SUCCESS)
		{
			printf("���� �� ����� ���� ������");
			return NULL;
		}
		char* buffer = new char[MAX_LINE];
		DWORD len = MAX_LINE;
		result = RegQueryValueEx(openedKey, valueName, NULL, NULL, (BYTE*)buffer, &len);
		if (result != ERROR_SUCCESS)
		{
			printf("������ ��������� ��������");
			return NULL;
		}
		RegCloseKey(openedKey);
		return buffer;
	}
