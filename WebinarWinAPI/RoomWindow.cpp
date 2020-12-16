#include <memory>
#include <Windows.h>
#include <Winsock.h>
#include <string>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <conio.h>
#include <exception>
#include <Mmsystem.h>
#include <audioclient.h>
#include <mmdeviceapi.h>

#include "RoomWindow.h"
#include "Button.h"
#include "TextBox.h"
#include "Role.h"
#include "Client.h"
#include "Mmsystem.h"
#include "mmreg.h"
#include "WaveWriter.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Winmm.lib")

#define FPS		                30
#define BUFFER_SIZE             1024
#define REFTIMES_PER_SEC		500000
#define REFTIMES_PER_MILLISEC	500
#define MAX_LOOP_BEFORE_STOP	20

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

HRESULT RecordAudioStream(const WCHAR* fileName);

DWORD WINAPI ThreadSoundRecorder(LPVOID _param)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (SUCCEEDED(hr))
	{
		while (true) {
			RecordAudioStream(L"capture.wav");
			PlaySound(L"capture.wav", NULL, SND_ASYNC | SND_NODEFAULT);
		}

		CoUninitialize();
	}
	return 0;
}

HRESULT RecordAudioStream(const WCHAR* fileName)
{
	HRESULT hr = S_OK;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	IAudioCaptureClient* pCaptureClient = NULL;
	WAVEFORMATEX* pwfx = NULL;
	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
	REFERENCE_TIME hnsActualDuration;
	UINT32 bufferFrameCount;
	WaveWriter waveWriter;
	UINT32 uiFileLength = 0;
	BOOL bExtensibleFormat = FALSE;

	try
	{
		IF_FAILED_THROW(CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator));
		IF_FAILED_THROW(pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice));
		IF_FAILED_THROW(pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient));

		IF_FAILED_THROW(pAudioClient->GetMixFormat(&pwfx));

		switch (pwfx->wFormatTag)
		{
		case WAVE_FORMAT_EXTENSIBLE:
			TRACE((L"WAVE_FORMAT_EXTENSIBLE"));
			bExtensibleFormat = TRUE;

			WAVEFORMATEXTENSIBLE* pWaveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pwfx);
			break;
		}

		IF_FAILED_THROW(pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL));
		IF_FAILED_THROW(pAudioClient->GetBufferSize(&bufferFrameCount));
		IF_FAILED_THROW(pAudioClient->GetService(IID_IAudioCaptureClient, (void**)&pCaptureClient));

		IF_FAILED_THROW(waveWriter.Initialize(fileName, bExtensibleFormat) ? S_OK : E_FAIL);

		hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

		IF_FAILED_THROW(pAudioClient->Start());

		BOOL bDone = FALSE;
		UINT32 packetLength = 0;
		UINT32 numFramesAvailable;
		BYTE* pData;
		DWORD flags;
		int iLoop = 0;

		while (bDone == FALSE)
		{
			Sleep(hnsActualDuration / REFTIMES_PER_MILLISEC / 2);

			IF_FAILED_THROW(pCaptureClient->GetNextPacketSize(&packetLength));

			while (packetLength != 0)
			{
				IF_FAILED_THROW(pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL));


				waveWriter.WriteWaveData(pData, numFramesAvailable * pwfx->nBlockAlign) ? S_OK : E_FAIL;

				uiFileLength += numFramesAvailable;

				IF_FAILED_THROW(pCaptureClient->ReleaseBuffer(numFramesAvailable));

				IF_FAILED_THROW(pCaptureClient->GetNextPacketSize(&packetLength));
			}

			if (iLoop++ == MAX_LOOP_BEFORE_STOP)
				bDone = TRUE;
		}
	}
	catch (HRESULT) {}

	if (hr == S_OK && pwfx != NULL)
		waveWriter.FinalizeHeader(pwfx, uiFileLength, bExtensibleFormat);

	if (pAudioClient)
	{
		pAudioClient->Reset();
		LOG_HRESULT(pAudioClient->Stop());
		SAFE_RELEASE(pAudioClient);
	}

	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pCaptureClient);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pDevice);

	return hr;
}

namespace webinar
{
	const int SCREEN_WIDTH = 1920;
	const int SCREEN_HEIGHT = 1080;
	const int CHAT_PORT = 5567;
	const int VOICE_LISTEN_PORT = 9663;
	const int VIDEO_LISTEN_PORT = 9881;
	const int SCREENSHOT_MS = 1000 / FPS;

	bool isEndOfWebinar = true;

	const char* HOST_ADRESS = "192.168.100.4";

	HBITMAP bmp = NULL;
	HINSTANCE Hrw = NULL;

	std::vector<WindowControl*> formControls;

	CRITICAL_SECTION critSec = { 0 };

	WSADATA verSoc = { 0 };

	SOCKET clientChatSocket = NULL;
	SOCKET servChatSocket = NULL;
	SOCKET videoSocket = NULL;
	SOCKET voiceSocket = NULL;

	const int CLIENT_COUNT = 30;
	int connectedClientsCount = 0;
	std::vector<client> clientSockets;

	void InitChat();

	void InitVoiceSocket();

	void InitVideoSocket();

	void ConnectToChat();

	void ConnectToVoice();

	void ConnectToVideo();

	int TakeScreenShot();

	void DrawImage(HWND hwnd, HDC hdc);

	const long videoRecvSize = 70000000;

	char videoRecvBuff[videoRecvSize];

	char voiceBuf[8294400];

	char* bmpBuffer = 0;

	DWORD WINAPI ThreadChatListener(LPVOID);
	DWORD WINAPI ThreadChatConnectionsListener(LPVOID);
	DWORD WINAPI ThreadScreenShot(LPVOID _param);
	DWORD WINAPI ThreadSoundRecorder(LPVOID _param);
	DWORD WINAPI ThreadFuncVideoListener(LPVOID _param);
	DWORD WINAPI ThreadFuncVoiceListener(LPVOID _param);
	DWORD WINAPI ThreadClientListen(LPVOID _param);

	std::string messagesData;

	bool ClickSend(WPARAM, LPARAM);
	bool ClickConnect(WPARAM, LPARAM);
	bool ClickStart(WPARAM, LPARAM);

	RoomWindow::RoomWindow(TCHAR * strWindowClassName, HINSTANCE hInstance, int x, int y, int width, int heigth, int iShowWindow)
	{
		_wndClass.cbSize = sizeof(_wndClass);
		_wndClass.style = CS_HREDRAW | CS_VREDRAW;
		_wndClass.lpfnWndProc = WndProc;
		_wndClass.lpszMenuName = nullptr;
		_wndClass.lpszClassName = strWindowClassName;
		_wndClass.cbWndExtra = NULL;
		_wndClass.cbClsExtra = NULL;
		_wndClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
		_wndClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);
		_wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		_wndClass.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
		_wndClass.hInstance = hInstance;

		if (!RegisterClassEx(&_wndClass))
		{
			throw std::exception("Window Class Initial Error");
		}

		WindowControl::_hwndWindow = CreateWindow(_wndClass.lpszClassName,
			strWindowClassName, 
			WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
			x, y, width, heigth,
			HWND(NULL), 
			NULL, 
			HINSTANCE(hInstance),
			NULL);

		if (!WindowControl::_hwndWindow)
		{
			throw std::exception("Window Cerate Error");
		}

		Hrw = (HINSTANCE)_hwndWindow;

		ShowWindow(WindowControl::_hwndWindow, iShowWindow);
		UpdateWindow(WindowControl::_hwndWindow);

		WindowControl::_strWindowName = reinterpret_cast<wchar_t>(strWindowClassName);
	}

	RoomWindow::~RoomWindow()
	{
		WSACleanup();
	}

	LRESULT RoomWindow::WndProc(HWND hWnd, 
		UINT uMsg, 
		WPARAM wParam,
		LPARAM lParam) 
	{
		try
		{
			switch (uMsg)
			{
			case WM_CREATE:
			{
				formControls.push_back(new TextBox(hWnd, _T(""), 1024, 0, 300, 576, 11));
				formControls.push_back(new TextBox(hWnd, _T(""), 1024, 586, 200, 30, 12));
				formControls.push_back(new Button(hWnd, _T("Send"), 1234, 586, 70, 30, 13));
				formControls[2]->SetEvent(ClickSend, WM_COMMAND);
				if (userRole == TEACHER) {
					formControls.push_back(new Button(hWnd, _T("Start webinar"), 50, 586, 100, 30, 15));
					formControls[3]->SetEvent(ClickStart, WM_COMMAND);
				}
				else {
					formControls.push_back(new Button(hWnd, _T("Ñonnect"), 50, 586, 100, 30, 10));
					formControls[3]->SetEvent(ClickConnect, WM_COMMAND);
				}

				return 0;
			}case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc, memDC;
				RECT rect;
				HWND hwnd = (HWND)Hrw;

				GetClientRect(hwnd, &rect);

				hdc = BeginPaint(hwnd, &ps);

				memDC = CreateCompatibleDC(hdc);
				HBITMAP memBmp = CreateCompatibleBitmap(hdc, 1024, 576);
				HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBmp);

				DrawImage(hwnd, memDC);

				BitBlt(hdc, 0, 0, 1024, 576, memDC, 0, 0, SRCCOPY);

				DeleteDC(memDC);
				DeleteObject(memBmp);
				DeleteObject(oldBitmap);

				EndPaint(hwnd, &ps);
				return 0;
			}
			case WM_COMMAND:

				for (int i = 0; i < formControls.size(); i++)
				{
					if (formControls[i]->GatID() == LOWORD(wParam))
					{
						formControls[i]->EventStart(uMsg, wParam, lParam);
					}
				}

				return 0;

			case WM_DESTROY:

				PostQuitMessage(NULL);

				return 0;

			default:
				return DefWindowProc(hWnd, uMsg, wParam, lParam);
			}
		}
		catch (std::exception ex)
		{

			MessageBoxA(NULL, ex.what(), NULL, MB_ICONERROR);

			PostQuitMessage(-1);
			return -1;
		}
	}

	void DrawImage(HWND hwnd, HDC hdc) {
		HDC hdcMem;
		BITMAP bitmap;
		HGDIOBJ oldBitmap;

		hdcMem = CreateCompatibleDC(hdc);
		oldBitmap = SelectObject(hdcMem, bmp);

		GetObject(bmp, sizeof(bitmap), &bitmap); 

		SetStretchBltMode(hdc, HALFTONE);
		StretchBlt(hdc, 0, 0, 1024, 576, hdcMem, 0, 0,SCREEN_WIDTH, SCREEN_HEIGHT, SRCCOPY);

		SelectObject(hdcMem, oldBitmap);
		DeleteDC(hdcMem);
		DeleteObject(oldBitmap);
	}

	bool ClickStart(WPARAM wParam, LPARAM lParam) {
		isEndOfWebinar = false;
		InitChat();
		InitVideoSocket();
		InitVoiceSocket();
		ConnectToChat();
		return true;
	}

	bool ClickConnect(WPARAM wParam, LPARAM lParam) {
		SetWindowTextA(formControls[3]->GetHandler(), "Disconnect");
		ConnectToChat();
		ConnectToVideo();
		ConnectToVoice();
		return true;
	}

	bool ClickSend(WPARAM wParam, LPARAM lParam)
	{
		if (clientChatSocket == NULL) {
			MessageBoxA(NULL, "No socket", NULL, MB_ICONERROR);
			return false;
		}

		UINT lenOfMessageText = GetWindowTextLength(formControls[1]->GetHandler());

		if (lenOfMessageText < 1)
			return false;

		std::string messageText;
		messageText.resize(lenOfMessageText+1);

		GetWindowTextA(formControls[1]->GetHandler(), &messageText[0], messageText.size());

		send(clientChatSocket, &messageText[0], messageText.size(), NULL);
	}

	void Brodcast(std::string message)
	{
		for (int i = 0; i < clientSockets.size(); i++)
		{
			if (clientSockets[i].chatSocket && clientSockets[i].chatSocket != SOCKET_ERROR)
			{
				int sizeSent = send(clientSockets[i].chatSocket,
					&message[0],
					strlen(&message[0]),
					0);

				if (sizeSent == SOCKET_ERROR)
				{
					closesocket(clientSockets[i].chatSocket);
					clientSockets[i].chatSocket = SOCKET_ERROR;
				}
			}
		}
	}

	void InitChat()
	{
		try
		{
			WSADATA verSoc = { 0 };

			std::cout << "Initialization socket vertion" << std::endl;
			if (WSAStartup(0x0202, &verSoc))
			{
				throw("Can't initialize a socket servion");
			}
			{
				servChatSocket = socket(AF_INET, SOCK_STREAM, 0);
				if (servChatSocket == INVALID_SOCKET)
				{
					WSACleanup();
					throw("Can't create the server socket");
				}

				sockaddr_in local_addr = { 0 };
				local_addr.sin_family = AF_INET;
				local_addr.sin_port = htons(CHAT_PORT);
				local_addr.sin_addr.s_addr = NULL;

				if (bind(servChatSocket,
					reinterpret_cast<sockaddr*>(&local_addr),
					sizeof(local_addr)) == SOCKET_ERROR)
				{
					closesocket(servChatSocket);
					WSACleanup();
					throw("Can't bind a socket with the sockaddr struct");
				}
			}

			clientSockets = std::vector<client>(CLIENT_COUNT);

			if (listen(servChatSocket, clientSockets.size()) == SOCKET_ERROR)
			{
				closesocket(servChatSocket);
				WSACleanup();
				throw("Can't set max amount of clients");
			}

			InitializeCriticalSection(&critSec);

			std::cout << "Server online, waiting for clients..." << std::endl;

			CreateThread(NULL,NULL,ThreadChatConnectionsListener,NULL,NULL,NULL);

		}
		catch (std::exception& ex)
		{

			MessageBoxA(NULL, ex.what(), NULL, MB_ICONERROR);

			PostQuitMessage(-1);
		}
		catch (...)
		{

			MessageBoxA(NULL, "Undefined chat init exception", NULL, MB_ICONERROR);

			PostQuitMessage(-1);
		}
	}

	void InitVoiceSocket() 
	{
		voiceSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (voiceSocket == INVALID_SOCKET)
		{
			WSACleanup();
			throw("Can't create the server socket");
		}

		sockaddr_in local_addr = { 0 };
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = htons(VOICE_LISTEN_PORT);
		local_addr.sin_addr.s_addr = NULL;

		if (bind(voiceSocket,
			reinterpret_cast<sockaddr*>(&local_addr),
			sizeof(local_addr)) == SOCKET_ERROR)
		{
			closesocket(voiceSocket);
			WSACleanup();
			throw("Can't bind a socket with the sockaddr struct");
		}

		if (listen(voiceSocket, clientSockets.size()) == SOCKET_ERROR)
		{
			closesocket(voiceSocket);
			WSACleanup();
			throw("Can't set max amount of clients");
		}

		CreateThread(NULL,NULL,ThreadSoundRecorder,NULL,NULL,NULL);
	}

	void InitVideoSocket()
	{
		videoSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (videoSocket == INVALID_SOCKET)
		{
			WSACleanup();
			throw("Can't create the server socket");
		}

		sockaddr_in local_addr = { 0 };
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = htons(VIDEO_LISTEN_PORT);
		local_addr.sin_addr.s_addr = NULL;

		if (bind(videoSocket,
			reinterpret_cast<sockaddr*>(&local_addr),
			sizeof(local_addr)) == SOCKET_ERROR)
		{
			closesocket(videoSocket);
			WSACleanup();
			throw("Can't bind a socket with the sockaddr struct");
		}

		if (listen(videoSocket, clientSockets.size()) == SOCKET_ERROR)
		{
			closesocket(videoSocket);
			WSACleanup();
			throw("Can't set max amount of clients");
		}

		CreateThread(NULL, NULL, ThreadScreenShot, NULL, NULL, NULL);
	}

	void ConnectTo(int port, SOCKET* Socket) {

		if (*Socket != NULL)
		{
			closesocket(*Socket);
			Socket = NULL;
			WSACleanup();
		}

		if (WSAStartup(0x0202, &verSoc))
		{
			throw(std::exception("Can't initialize socket version: ", WSAGetLastError()));
		}

		*Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (*Socket == SOCKET_ERROR)
		{
			WSACleanup();
			throw(std::exception("Can't create socket: ", GetLastError()));
		}

		std::string ip = HOST_ADRESS;

		if (!strlen(&ip[0]))
		{
			throw(std::exception("Incorrect host ip"));
		}

		struct sockaddr_in 	addrClient = { 0 };
		addrClient.sin_addr.s_addr = inet_addr(&ip[0]);

		if (INADDR_NONE == addrClient.sin_addr.s_addr)
		{
			struct hostent* hostName = gethostbyname(&ip[0]);

			if (!hostName)
			{
				closesocket(*Socket);
				throw(std::exception("Can't get host address"));
			}

			addrClient.sin_addr = *(struct in_addr*)hostName->h_addr_list[0];

		}

		addrClient.sin_family = AF_INET;
		addrClient.sin_port =  htons(port);

		if (SOCKET_ERROR == connect(*Socket,
			(struct sockaddr*)&addrClient,
			sizeof(addrClient)))
		{
			closesocket(*Socket);
			throw(std::exception("Can't connect to server"));
		}
	}

	void ConnectToChat()
	{
		try
		{
			ConnectTo(CHAT_PORT, &clientChatSocket);

			CreateThread(NULL,
				NULL,
				ThreadChatListener,
				reinterpret_cast<LPVOID>(clientChatSocket),
				NULL,
				NULL);

			send(clientChatSocket,
				userName.c_str(),
				strlen(userName.c_str()),
				NULL);
		}
		catch (std::exception ex)
		{
			if (clientChatSocket != NULL)
			{
				closesocket(clientChatSocket);
				clientChatSocket = NULL;
			}
			MessageBoxA(NULL, ex.what(), NULL, MB_ICONERROR);
		}
	}

	void ConnectToVoice()
	{
		ConnectTo(VOICE_LISTEN_PORT, &voiceSocket);

		CreateThread(NULL,
			NULL,
			ThreadFuncVoiceListener,
			NULL,
			NULL,
			NULL);
	}

	void ConnectToVideo()
	{
		ConnectTo(VIDEO_LISTEN_PORT, &videoSocket);

		 CreateThread(NULL,
			NULL,
			ThreadFuncVideoListener,
			NULL,
			NULL,
			NULL);
	}

	int TakeScreenShot() {
		HDC hScreen = GetDC(NULL);
		int ScreenX = SCREEN_WIDTH;
		int ScreenY = SCREEN_HEIGHT;


		HDC hdcMem = CreateCompatibleDC(hScreen);
		DeleteObject(bmp);
		bmp = CreateCompatibleBitmap(hScreen, ScreenX, ScreenY);
		HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, bmp);

		SetStretchBltMode(hdcMem, HALFTONE);

		StretchBlt(hdcMem,
			0, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT,
			hScreen,
			0, 0,
			ScreenX,
			ScreenY,
			SRCCOPY);
		SelectObject(hdcMem, hOld);

		BITMAPINFOHEADER bmi = { 0 };
		bmi.biSize = sizeof(BITMAPINFOHEADER);
		bmi.biPlanes = 1;
		bmi.biBitCount = 32;
		bmi.biWidth = SCREEN_WIDTH;
		bmi.biHeight = -SCREEN_HEIGHT;
		bmi.biCompression = BI_RGB;
		bmi.biSizeImage = 0;

		if (!bmpBuffer) {
			bmpBuffer = (char*)malloc(4 * SCREEN_WIDTH * SCREEN_HEIGHT);
		}

		int s = GetDIBits(hdcMem, bmp, 0, SCREEN_HEIGHT, bmpBuffer, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

		ReleaseDC(GetDesktopWindow(), hScreen);
		DeleteDC(hScreen);
		DeleteDC(hdcMem);
		DeleteObject(hOld);

		return 4 * SCREEN_WIDTH * SCREEN_HEIGHT;
	}

	DWORD WINAPI ThreadChatListener(LPVOID _param)
	{
		const int SIZE_BUF = 1024;

		SOCKET Socket = reinterpret_cast<SOCKET>(_param);

		char buf_recv[SIZE_BUF] = { 0 };
		int size_recv;
		do
		{
			memset(buf_recv, 0, sizeof(buf_recv));
			size_recv = recv(Socket,
				buf_recv,
				sizeof(buf_recv) - 1,
				0);

			int err = WSAGetLastError();
			if (size_recv && size_recv != SOCKET_ERROR)
			{
				messagesData += "\r\n";
				messagesData += buf_recv;
				SetWindowTextA(formControls[0]->GetHandler(), messagesData.c_str());
			}
		} while (size_recv && size_recv != SOCKET_ERROR);

		return 0;
	}

	DWORD WINAPI ThreadChatConnectionsListener(LPVOID _param)
	{
		bool first = true;

		while (servChatSocket != SOCKET_ERROR)
		{
			try
			{
				SOCKET connectingSocketClient = NULL;
				sockaddr_in clientAddrInfo = { 0 };

				int sizeClientAddrInfo = sizeof(clientAddrInfo);

				connectingSocketClient = accept(servChatSocket,
					reinterpret_cast<sockaddr*>(&clientAddrInfo),
					&sizeClientAddrInfo);


				if (connectingSocketClient == INVALID_SOCKET && servChatSocket != INVALID_SOCKET)
					throw("Couldn't connect a client");
				if (servChatSocket == INVALID_SOCKET)
					continue;
				else
				{
					if (connectedClientsCount < clientSockets.size())
					{
						EnterCriticalSection(&critSec);

						for (int i = 0; i < clientSockets.size(); i++)
						{
							if (!clientSockets[i].chatSocket)
							{
								clientSockets[i].chatSocket = connectingSocketClient;

								CreateThread(NULL, NULL, ThreadClientListen, reinterpret_cast<LPVOID>(i), NULL, NULL);

								if (!first) {
									SOCKET connectingSocketClient1 = NULL;
									sockaddr_in clientAddrInfo1 = { 0 };
									int sizeClientAddrInfo1 = sizeof(clientAddrInfo1);
									connectingSocketClient1 = accept(videoSocket,
										reinterpret_cast<sockaddr*>(&clientAddrInfo1),
										&sizeClientAddrInfo1);

									clientSockets[i].videoSocket = connectingSocketClient1;

									SOCKET connectingSocketClient2 = NULL;
									sockaddr_in clientAddrInfo2 = { 0 };
									int sizeClientAddrInfo2 = sizeof(clientAddrInfo2);
									connectingSocketClient2 = accept(voiceSocket,
										reinterpret_cast<sockaddr*>(&clientAddrInfo2),
										&sizeClientAddrInfo2);

									clientSockets[i].voiceSocket = connectingSocketClient2;
								}
								else {
									first = false;
								}

								break;
							}
						}
						connectedClientsCount++;
						LeaveCriticalSection(&critSec);
					}
					else
					{
						char errorMesage[] = "Server is full";
						send(connectingSocketClient, errorMesage, sizeof(errorMesage), 0);
						closesocket(connectingSocketClient);
						throw "Can't connect a client";
					}
				}
			}
			catch (std::exception& ex)
			{
				MessageBoxA(NULL, ex.what(), NULL, MB_ICONERROR);
			}
			catch (...)
			{
				MessageBoxA(NULL, "Unhandled client exception", NULL, MB_ICONERROR);
			}
		}

		DeleteCriticalSection(&critSec);

		closesocket(servChatSocket);

		WSACleanup();
	}

	DWORD WINAPI ThreadScreenShot(LPVOID _param)
	{
		do
		{
			int len = TakeScreenShot();

			InvalidateRect((HWND)Hrw, NULL, false);

			for (int i = 1; i < clientSockets.size(); i++)
			{
				if (clientSockets[i].videoSocket && clientSockets[i].videoSocket != SOCKET_ERROR)
				{
					int sizeSent = send(clientSockets[i].videoSocket, (const char*)bmpBuffer, len, 0);
					int err = WSAGetLastError();
					
					if (sizeSent == SOCKET_ERROR)
					{
						closesocket(clientSockets[i].videoSocket);
						clientSockets[i].videoSocket = SOCKET_ERROR;
					}
				}
			}

			Sleep(SCREENSHOT_MS);
		} while (!isEndOfWebinar);

		return 0;
	}

	DWORD WINAPI ThreadFuncVideoListener(LPVOID _param)
	{
		int bytesToRead = 4 * SCREEN_WIDTH * SCREEN_HEIGHT;
		int nbytes;
		sockaddr_in sender_addr;
		int address_size = sizeof(struct sockaddr_in);

		while (true) {
			nbytes = recv(videoSocket, videoRecvBuff + (4 * SCREEN_WIDTH * SCREEN_HEIGHT - bytesToRead), bytesToRead, 0);
				
			if (nbytes == SOCKET_ERROR) {
				closesocket(videoSocket);
				break;
			}
			bytesToRead = bytesToRead - nbytes;
			if (bytesToRead == 0) {
				bmp = CreateBitmap(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 32, videoRecvBuff);
				InvalidateRect((HWND)Hrw, NULL, false);
				bytesToRead = 4 * SCREEN_WIDTH * SCREEN_HEIGHT;
			}

		}
		return 0;
	}

	DWORD WINAPI ThreadSoundRecorder(LPVOID _param)
	{
		while (true)
		{
			HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			RecordAudioStream(L"capture.wav");
			FILE* temp;
			fopen_s(&temp, "capture.wav", "rb");
			int len = fread(&voiceBuf, 1, 6400000, temp);

			for (int i = 1; i < clientSockets.size(); i++)
			{
				if (clientSockets[i].voiceSocket && clientSockets[i].voiceSocket != SOCKET_ERROR)
				{
					int sizeSent = send(clientSockets[i].voiceSocket, (const char*)voiceBuf, len, 0);

					if (sizeSent == SOCKET_ERROR)
					{
						int j = WSAGetLastError();
						closesocket(clientSockets[i].voiceSocket);
						clientSockets[i].voiceSocket = SOCKET_ERROR;
					}
				}
			}
			fclose(temp);
			CoUninitialize();
		}
		return 0;
	}

	DWORD WINAPI ThreadFuncVoiceListener(LPVOID _param)
	{
		
		int nbytes;
		sockaddr_in sender_addr;
		int address_size = sizeof(struct sockaddr_in);

		while (true) {

			nbytes = recv(voiceSocket, voiceBuf, 8294400,0);

			if (nbytes == SOCKET_ERROR) {
				closesocket(voiceSocket);
				break;
			}
			FILE* temp;
			errno_t err = fopen_s(&temp, "Capture2.wav", "wb");
			if (!err) {
				int len = fwrite(&voiceBuf, 1, nbytes, temp);
				fclose(temp);
				PlaySound(L"Capture2.wav", NULL, SND_ASYNC | SND_NODEFAULT);
			}
		}
		return 0;
	}

	DWORD WINAPI ThreadClientListen(LPVOID _param)
	{
		int index = reinterpret_cast<int>(_param);;
		try
		{
			while (clientSockets[index].chatSocket
				!=
				SOCKET_ERROR)
			{
				std::string messageBuffer(BUFFER_SIZE, '\0');

				int receivedSize = recv(clientSockets[index].chatSocket,
					&messageBuffer[0],
					messageBuffer.size(),
					0);

				if (receivedSize != SOCKET_ERROR && clientSockets[index].chatSocket != SOCKET_ERROR)
				{
					if (!clientSockets[index].name.size())
					{
						clientSockets[index].name = messageBuffer;
						clientSockets[index].name.resize(strlen(&messageBuffer[0]));
						char connectMsg[] = "Connected: ";

						EnterCriticalSection(&critSec);

						Brodcast(connectMsg + messageBuffer);
					}
					else
					{
						EnterCriticalSection(&critSec);

						std::string longMessageCreator = clientSockets[index].name + ": " + messageBuffer;
						Brodcast(longMessageCreator);
					}
					LeaveCriticalSection(&critSec);
				}
				else
				{
					EnterCriticalSection(&critSec);
					closesocket(clientSockets[index].chatSocket);
					clientSockets[index].chatSocket = SOCKET_ERROR;
					LeaveCriticalSection(&critSec);
				}
			}
		}
		catch (std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		}
		EnterCriticalSection(&critSec);

		closesocket(clientSockets[index].chatSocket);
		clientSockets[index].chatSocket = NULL;

		Brodcast("Disconnected: " + clientSockets[index].name);

		clientSockets[index].name.resize(0);

		connectedClientsCount--;

		LeaveCriticalSection(&critSec);

		return 0;
	}
}
