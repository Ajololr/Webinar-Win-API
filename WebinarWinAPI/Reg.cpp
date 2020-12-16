#undef UNICODE
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <AclAPI.h>
#include <sddl.h>
#include <string>

using namespace std;

#define MAX_LINE 50

#define CREATE_KEY 0
#define WRITE_REG_DWORD 1
#define READ_REG_DWORD 2
#define WRITE_REG_STRING 3
#define READ_REG_STRING 4
#define FIND_REG_KEY 5
#define READ_KEY_FLAG 6
#define NOTIFY_KEY_CHANGED 7
#define EXIT 8

HANDLE hKeyChangedEvent;
HANDLE hKeyChangedThread;

BOOL FindKey(HKEY currentKey, LPCSTR keyName);
int ReadKeyFlags(HKEY currentKey);
LPCSTR ReadStringFromRegistry(HKEY key, LPCSTR subkey, LPCSTR valueName);
void WriteStringInRegistry(HKEY key, LPCSTR subkey, LPCSTR valueName, LPCSTR value);
void  WriteDwordInRegistry(HKEY key, LPCSTR subkey, LPCSTR valueName, DWORD value);
DWORD ReadDwordValueRegistry(HKEY key, LPCSTR subkey, LPCSTR valueName);
void CreateRegistryKey(HKEY key, LPCSTR subkey);
void CreateThreadAndEvent();
DWORD WINAPI onKeyChanged(LPVOID lpParam);

void CreateRegistryKey(HKEY key, LPCSTR subkey)
{
	DWORD dwDisposition;
	HKEY  hKey;
	DWORD Ret;
	Ret =
		RegCreateKeyEx(key, subkey, 0,
			NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
	if (Ret != ERROR_SUCCESS)
	{
		printf("Ошибка создания ключа\n");
	}
	else 
		printf("Ключ создан\n");
	RegCloseKey(hKey);
}

void  WriteDwordInRegistry(HKEY key, LPCSTR subkey, LPCSTR valueName, DWORD value)
{
	DWORD disposition;
	HKEY openedKey;
	DWORD result;
	result = RegOpenKeyEx(key, subkey, 0, KEY_WRITE, &openedKey);
	if (result != ERROR_SUCCESS)
	{
		printf("Ключ не может быть открыт\n");
	}
	result = RegSetValueEx(openedKey, valueName, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
	if (result != ERROR_SUCCESS)
	{
		printf("Нельзя произвести запись\n");
	}
	RegCloseKey(openedKey);
}

DWORD ReadDwordValueRegistry(HKEY key, LPCSTR subkey, LPCSTR valueName)
{
	DWORD result;
	HKEY openedKey;
	result = RegOpenKeyEx(key, subkey, 0, KEY_READ, &openedKey);
	if (result != ERROR_SUCCESS)
	{
		printf("Ключ не может быть открыт\n");
		return -1;
	}
	DWORD buffer;
	DWORD len = sizeof(buffer);
	result = RegQueryValueEx(openedKey, valueName, NULL, NULL, (BYTE*)&buffer, &len);
	if (result != ERROR_SUCCESS)
	{
		printf("Нельзя прочитать значение");
		return -1;
	}
	RegCloseKey(openedKey);
	return buffer;
}

void WriteStringInRegistry(HKEY key, LPCSTR subkey, LPCSTR valueName, LPCSTR value)
{
	DWORD disposition;
	HKEY openedKey;
	DWORD result;
	result = RegOpenKeyEx(key, subkey, 0, KEY_WRITE, &openedKey);
	if (result != ERROR_SUCCESS)
		printf("Нельзя создать ключ");
	result = RegSetValueEx(openedKey, valueName, 0, REG_SZ, (BYTE*)value, strlen(value));
	if (result != ERROR_SUCCESS)
		printf("Нельзя произвести запись");
	RegCloseKey(openedKey);
}

LPCSTR ReadStringFromRegistry(HKEY key, LPCSTR subkey, LPCSTR valueName)
{
	DWORD result;
	HKEY openedKey;
	result = RegOpenKeyEx(key, subkey, 0, KEY_READ, &openedKey);
	if (result != ERROR_SUCCESS)
	{
		printf("Ключ не может быть открыт");
		return NULL;
	}
	char* buffer = new char[MAX_LINE];
	DWORD len = MAX_LINE;
	result = RegQueryValueEx(openedKey, valueName, NULL, NULL, (BYTE*)buffer, &len);
	if (result != ERROR_SUCCESS)
	{
		printf("Нельзя прочитать значение");
		return NULL;
	}
	RegCloseKey(openedKey);
	return buffer;;
}

BOOL FindKey(HKEY currentKey, LPCSTR keyName)
{
	DWORD subkeysAmount;
	DWORD maxSubkeyLen, currentSubkeyLen;
	BOOL result;
	RegQueryInfoKey(currentKey, NULL, 0, NULL, &subkeysAmount, &maxSubkeyLen,
		NULL, NULL, NULL, NULL, NULL, NULL);
	char* bufferName = new char[maxSubkeyLen];
	for (int i = 0; i < subkeysAmount; i++)
	{
		currentSubkeyLen = maxSubkeyLen;
		result = RegEnumKeyEx(currentKey, i, bufferName, &currentSubkeyLen, NULL, NULL, NULL, NULL);
		if (result == ERROR_SUCCESS)
		{
			if (!strcmp(bufferName, keyName))
			{
				return TRUE;
			}
			HKEY innerKey;
			result = RegOpenKey(currentKey, bufferName, &innerKey);
			if (result == ERROR_SUCCESS)
			{
				result = FindKey(innerKey, keyName);
				if (result)
				{
					RegCloseKey(innerKey);
					return result;
				}
			}
			RegCloseKey(innerKey);
		}
	}
	return FALSE;
}




const char* ConvertAceStringToString(char* source)
{
	string strSource(source);
	string* strKeyAccess = new string("");
	int strLen = strSource.length();
	int i = 0;
	int semicolonCounter = 0;
	while (i < strLen)
	{
		if (source[i] == ')')
		{
			semicolonCounter = 0;
		}
		if (source[i] == ';')
		{
			semicolonCounter++;
		}
		if (semicolonCounter == 2)
		{
			int start = i + 1;
			do
			{
				i++;
			} while (source[i] != ';');
			strKeyAccess->append(strSource.substr(start, i - start));
			semicolonCounter++;
		}
		if (semicolonCounter == 5)
		{
			int start = i + 1;
			while (source[i] != ')')
			{
				i++;
			}
			strKeyAccess->append(" <-" + strSource.substr(start, i - start) + "\n");
			semicolonCounter = 0;
		}
		i++;
	}
	return strKeyAccess->c_str();
}

int readKeyFlags(HKEY currentKey)
{
	int isSuccess = 1;
	DWORD securityDescriptorSize;
	DWORD subkeysNumber;
	RegQueryInfoKey(currentKey, NULL, 0, NULL, &subkeysNumber,
		NULL, NULL, NULL, NULL, NULL, &securityDescriptorSize, NULL);
	char* buffer = new char[securityDescriptorSize];
	DWORD result;
	result = RegGetKeySecurity(currentKey, DACL_SECURITY_INFORMATION, buffer, &securityDescriptorSize);
	if (result != ERROR_SUCCESS)
	{
		printf("Нельзя прочитать флаги\n");
		isSuccess = 0;
	}
	else
	{
		SECURITY_DESCRIPTOR* security = reinterpret_cast<SECURITY_DESCRIPTOR*>(buffer);
		LPSTR strDacl;
		ConvertSecurityDescriptorToStringSecurityDescriptor(security, SDDL_REVISION_1, DACL_SECURITY_INFORMATION, &strDacl, NULL);
		printf("%s\n", strDacl);
		printf("%s\n", ConvertAceStringToString(strDacl));
	}
	return isSuccess;
}


void notifyKeyChanged(HKEY currentKey)
{
	hKeyChangedEvent = CreateEvent(NULL, TRUE, FALSE, "Ключ изменён");
	DWORD threadId;
	hKeyChangedThread = CreateThread(NULL, 0, onKeyChanged, NULL, 0, &threadId);
	RegNotifyChangeKeyValue(currentKey, TRUE, REG_NOTIFY_CHANGE_LAST_SET, hKeyChangedEvent, TRUE);
}



void CloseEvents()
{
	CloseHandle(hKeyChangedEvent);
}

DWORD WINAPI onKeyChanged(LPVOID lpParam)
{
	WaitForSingleObject(hKeyChangedEvent, INFINITE);
	printf("Ключ изменён\n");
	return 0;
}

int main()
{
	setlocale(LC_ALL, "Russian");
	string rules = "0 - Cоздать ключ\n";
	rules += "1 - Записать число\n";
	rules += "2 - Прочитать число\n";
	rules += "3 - Записать строку\n";
	rules += "4 - Прочитать строку\n";
	rules += "5 - Поиск ключа\n";
	rules += "6 - Прочитать флаги\n";
	rules += "7 - Отслеживать иземения реестра\n";
	rules += "8 - Выход\n";
	
	printf("%s\n", rules.c_str());
	int isContinue = 1;
	char buffer[1024];
	char secondBuffer[1024];
	DWORD dwValue;
	DWORD dwResult;
	LPCSTR strResult;
	HKEY currentKey;
	DWORD disposition;
	char subkey[1024];
	while (isContinue)
	{
		printf("Введите команду: ");
		int command;
		scanf("%d", &command);
		switch (command)
		{
		case CREATE_KEY:
			printf("Введите имя ключа: ");
			scanf("%s", buffer);
			CreateRegistryKey(HKEY_CURRENT_USER, buffer);
			break;
		case WRITE_REG_DWORD:
			printf("Введите имя подключа: ");
			scanf("%s", subkey);
			printf("Введите имя записи: ");
			scanf("%s", buffer);
			printf("Значение записи: ");
			scanf("%d", &dwValue);
			WriteDwordInRegistry(HKEY_CURRENT_USER, subkey, buffer, dwValue);
			break;
		case READ_REG_DWORD:
			printf("Введите имя подключа: ");
			scanf("%s", subkey);
			printf("Введите имя записи: ");
			scanf("%s", buffer);
			dwResult = ReadDwordValueRegistry(HKEY_CURRENT_USER, subkey, buffer);
			if (dwResult != -1)
				printf("Значение записи: %d\n", dwResult);
			break;
		case WRITE_REG_STRING:
			printf("Введите имя подключа: ");
			scanf("%s", subkey);
			printf("Введите имя записи: ");
			scanf("%s", buffer);
			printf("Значение записи: ");
			scanf("%s", secondBuffer);
			WriteStringInRegistry(HKEY_CURRENT_USER, subkey, buffer, secondBuffer);
			break;
		case READ_REG_STRING:
			printf("Введите имя подключа: ");
			scanf("%s", subkey);
			printf("Введите имя записи: ");
			scanf("%s", buffer);
			strResult = ReadStringFromRegistry(HKEY_CURRENT_USER, subkey, buffer);
			if (strResult != NULL)
				printf("Значение записи: %s\n", strResult);
			break;
		
		case FIND_REG_KEY:
			printf("Введите имя ключа: ");
			scanf("%s", buffer);
			if (FindKey(HKEY_CURRENT_USER, buffer))
				printf("Ключ найден\n");
			else
				printf("Ключ не был найден\n");
			break;
		case READ_KEY_FLAG:
		{
			HKEY currentKey;
			printf("Введите имя ключа: ");
			scanf("%s", buffer);
			dwResult = RegOpenKey(HKEY_CURRENT_USER, buffer, &currentKey);
			if (dwResult != ERROR_SUCCESS)
				printf("Ключ не может быть открыт\n");
			else
			{
				readKeyFlags(currentKey);
				RegCloseKey(currentKey);
			}
		}
		break;
		case NOTIFY_KEY_CHANGED:
		{
			printf("Введите имя ключа:");
			scanf("%s", buffer);
			HKEY currentKey;
			dwResult = RegOpenKey(HKEY_CURRENT_USER, buffer, &currentKey);
			if (dwResult != ERROR_SUCCESS)
				printf("Ключ не может быть открыт\n");
			else
				notifyKeyChanged(currentKey);
		}
		break;
		case EXIT:
			isContinue = 0;
			CloseEvents();
			break;
		default:
			break;
		}
	}
	return 0;
}

