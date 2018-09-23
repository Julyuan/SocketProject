//#pragma comment(lib, "Ws2_32.lib")
//#include<WinSock2.h>
//#include<stdio.h>
//
//#define SERVER_PORT 6666
//
//struct student {
//	char name[32];
//	int age;
//};
//
//int main() {
//	WORD wVersionRequested;
//	WSADATA wsaData;
//	int ret, nLeft, length;
//
//	SOCKET sListen, sServer;
//	struct sockaddr_in saServer, saClient;
//	struct student stu;
//	char *ptr;
//
//	// WinSock initialization
//	wVersionRequested = MAKEWORD(2, 2);
//	ret = WSAStartup(wVersionRequested, &wsaData);
//	if (ret != 0) {
//		printf("WSAStartup() failed!\n");
//		return -1;
//	}
//
//	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
//		WSACleanup();
//		printf("Invalid Winsock version!\n");
//	}
//
//	//create socket using TCP protocol
//	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	if (sListen == INVALID_SOCKET) {
//		WSACleanup();
//		printf("socket() failed!\n");
//		return -1;
//	}
//
//	// build local address information
//	saServer.sin_family = AF_INET; // address family
//	saServer.sin_port = htons(SERVER_PORT); //
//	saServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
//
//	// bind
//	ret = bind(sListen, (struct sockaddr *)&saServer, sizeof(saServer));
//	if (ret == SOCKET_ERROR) {
//		printf("bind() failed! code:%d\n", WSAGetLastError());
//		closesocket(sListen);
//		WSACleanup();
//		return -1;
//	}
//
//	// listen connection request
//	ret = listen(sListen, 5);
//	if (ret == SOCKET_ERROR) {
//		printf("listen() failed! code:%d\n", WSAGetLastError());
//		closesocket(sListen);
//		WSACleanup();
//		return -1;
//	}
//
//	printf("Waiting for client connecting!\n");
//	printf("tips:Ctrl+c to quit!\n");
//
//	length = sizeof(saClient);
//	sServer = accept(sListen, (struct sockaddr*)&saClient, &length);
//	if (sServer == INVALID_SOCKET) {
//		printf("accept() failed! code:%d\n", WSAGetLastError());
//		closesocket(sListen);
//		WSACleanup();
//		return -1;
//	}
//
//	printf("Accepted client: %s:%d\n",
//		inet_ntoa(saClient.sin_addr), ntohs(saClient.sin_port));
//
//	nLeft = sizeof(stu);
//	ptr = (char *)&stu;
//
//	while (nLeft > 0) {
//		ret = recv(sServer, ptr, nLeft, 0);
//		if (ret == SOCKET_ERROR) {
//			printf("recv() failed!\n");
//			break;
//		}
//
//		if (ret == 0) {
//			printf("client has closed the connection!\n");
//			break;
//		}
//
//		nLeft -= ret;
//		ptr += ret;
//	}
//
//	if (!nLeft)
//		printf("name: %s\nage:%d\n", stu.name, stu.age);
//
//	closesocket(sListen);
//	closesocket(sServer);
//	WSACleanup();
//
//	system("pause");
//	return 0;
//}