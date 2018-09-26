#pragma comment(lib, "Ws2_32.lib")

#include<stdio.h>
#include<WinSock2.h>
#include<iostream>
#include<string>
#define SERVER_PORT 4396

bool isConnected = false;

struct student {
	char name[32];
	int age;
};

int choose() {
	int res = 0;

	while (true) {
		std::cout << "请选择需要执行的操作：" << std::endl;
		std::cout << "1.连接服务器" << std::endl;
		std::cout << "2.断开服务器" << std::endl;
		std::cout << "3.获取服务器时间" << std::endl;
		std::cout << "4.获取名字" << std::endl;
		std::cout << "5.获取客户端列表" << std::endl;
		std::cout << "6.发送消息" << std::endl;
		std::cout << "7.退出" << std::endl;
		std::cin >> res;
		if (res <= 0 || res >= 8)
			std::cout << "该操作无效，请重新选择" << std::endl;
		else
			break;
	}
	return res;
}

int connectServer() {
	char ip_addr[50], port[10];
	std::cout << "请依次输入服务器的IP地址和端口号" << std::endl;
	std::cin >> ip_addr >> port;
	WORD wVersionRequested;
	WSADATA wsaData;
	int ret;
	SOCKET sClient;
	struct sockaddr_in saServer;
	struct student stu;
	char *ptr = (char*)&stu;
	BOOL fSuccess = TRUE;

	//if (argc != 4) {
	//	printf("usage:inform WinClient serverIP name age\n");
	//	return -1;
	//}
	// WinSock initialization;
	wVersionRequested = MAKEWORD(2, 2);
	ret = WSAStartup(wVersionRequested, &wsaData);

	if (ret != 0) {
		printf("WSAStartup() failed!\n");
		return -2;
	}

	// ensure that support WinSock DLL version 2.2
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		printf("Invalid Winsock version!\n");
		return -3;
	}

	// create socket using TCP protocol 
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET) {
		WSACleanup();
		printf("socket() failed!\n");
		return -4;

	}

	saServer.sin_family = AF_INET;
	saServer.sin_port = htons(SERVER_PORT);
	saServer.sin_addr.S_un.S_addr = inet_addr(ip_addr);

	// construct server address information
	ret = connect(sClient, (struct sockaddr*)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		printf("connect() failed!\n");
		closesocket(sClient);
		WSACleanup();
		return -5;
	}
}


int main(int argc, char* argv[]) {


	strcpy(stu.name, argv[2]);
	stu.age = atoi(argv[3]);
	ret = send(sClient, (char*)&stu, sizeof(stu), 0);
	if (ret == SOCKET_ERROR) {
		printf("send() failed!\n");
	}
	else {
		printf("student info has been sent!\n");
	}

	closesocket(sClient);
	WSACleanup();

	system("pause");
	return 0;
}