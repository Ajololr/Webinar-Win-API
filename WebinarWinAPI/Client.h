#pragma once
#include <winsock.h>
#include <string>

struct client
{
	std::string name;
	SOCKET chatSocket;
	SOCKET videoSocket;
	SOCKET voiceSocket;
};