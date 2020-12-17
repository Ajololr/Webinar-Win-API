#pragma once
#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <AclAPI.h>
#include <sddl.h>
#include <string>

#undef UNICODE
#define _CRT_SECURE_NO_WARNINGS
#define MAX_LINE 50
#define SUB_KEY L"Webinar"
#define KEY_NAME L"userName"

#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

void CreateRegistryKey(HKEY key, LPWSTR subkey); 
void WriteStringInRegistry(HKEY key, LPWSTR subkey, LPWSTR valueName, LPCSTR value);
LPCSTR ReadStringFromRegistry(HKEY key, LPWSTR subkey, LPWSTR valueName);

#endif