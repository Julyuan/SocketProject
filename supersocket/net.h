#pragma once

#define WM_SOCKET (WM_USER + 1)

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

}


