#pragma once

#define WM_SOCKET (WM_USER + 1)
#include<WinSock2.h>

SOCKET tcps, tcps_listen;
BOOL beSendBlock = FALSE;

// socket initialization
inline int NetInit() {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);

	// initialization winsock
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {
		MessageBox(NULL, (LPCWSTR)"Winsock startup error!", (LPCWSTR)"Error", 0);
		return 0;
	}

	// check winsock version
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		MessageBox(NULL, (LPCWSTR)"Invalid", (LPCWSTR)"Error", 0);
		return 0;
	}

	return 1;

}

// socket¹Ø±Õ
inline void NetEnd() {
	closesocket(tcps);
	WSACleanup();
}

// socket¼àÌý
inline int NetListen(int port) {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {
		MessageBox(NULL, (LPCWSTR)"Winsock startup error!", (LPCWSTR)"Error", 0);
		return 0;
	}

	// check winsock verison
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		MessageBox(NULL, (LPCWSTR)"Invalid winsock version!", (LPCWSTR)"Error", 0);
		return 0;
	}
	return 1;
}

// socket close

inline void NetEnd() {
	closesocket(tcps);
	WSACleanup();
}

// socket listen
inline int NetListen(int port) {
	// clean buffer
	ClearBuf();
	Clear
	// create socket
}

