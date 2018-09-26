#define WM_SOCKET (WM_USER+1)

SOCKET tcps, tcps_listen;
BOOL beSendBlock = FALSE;

//socket初始化
inline int NetInit()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	//初始化winsock
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		MessageBox(NULL, (LPCWSTR)"Winsock startup error!", (LPCWSTR)"Error", 0);
		return 0;
	}
	//检查winsock版本
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		MessageBox(NULL, (LPCWSTR)"Invalid winsock version!", (LPCWSTR)"Error", 0);
		return 0;
	}
	return 1;
}

//socket关闭
inline void NetEnd()
{
	closesocket(tcps);
	WSACleanup();
}

//socket监听
inline int NetListen(int port)
{
	//先清空缓冲区
	ClearBuf();
	//创建socket
	tcps_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tcps == INVALID_SOCKET)
	{
		MessageBox(NULL, (LPCWSTR)"Can't create socket!", (LPCWSTR)"Error", 0);
		return 0;
	}
	if (SOCKET_ERROR == WSAAsyncSelect(tcps_listen, hWnd, WM_SOCKET,
		FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE))
	{
		MessageBox(NULL, (LPCWSTR)"Select error!", (LPCWSTR)"Error", 0);
		return 0;
	}
	SOCKADDR_IN tcpaddr;
	tcpaddr.sin_family = AF_INET;
	tcpaddr.sin_port = htons(port);
	tcpaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//绑定socket
	int err = bind(tcps_listen, (SOCKADDR*)&tcpaddr, sizeof(tcpaddr));
	if (err != 0)
	{
		MessageBox(NULL,(LPCWSTR)"Bind error!", (LPCWSTR)"Error", 0);
		return 0;
	}
	//侦听
	err = listen(tcps_listen, 1);
	if (err != 0)
	{
		MessageBox(NULL, (LPCWSTR)"Listen error!", (LPCWSTR)"Error", 0);
		return 0;
	}
	SendMessage(hListenBtn, WM_SETTEXT, 0, (long)"Cancel");
	listening = TRUE;
	return 1;
}

//socket连接
inline int NetConn(LPCSTR ipstr, int port)
{
	//先清空缓冲区
	ClearBuf();
	//创建socket
	tcps = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tcps == INVALID_SOCKET)
	{
		MessageBox(NULL, (LPCWSTR)"Can't create socket!", (LPCWSTR)"Error", 0);
		return 0;
	}
	//进入异步选择模式，注册关心的socket事件
	if (SOCKET_ERROR == WSAAsyncSelect(tcps, hWnd, WM_SOCKET,
		FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE))
	{
		MessageBox(NULL, (LPCWSTR)"Select error!", (LPCWSTR)"Error", 0);
		return 0;
	}
	unsigned long ip = inet_addr(ipstr);
	if (INADDR_NONE == ip)
	{
		hostent * he = gethostbyname(ipstr);
		ip = *((unsigned long *)(he->h_addr_list[0]));
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = ip;
	addr.sin_port = htons(port);
	//连接
	if (SOCKET_ERROR == connect(tcps, (sockaddr*)&addr, sizeof(addr)))
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			MessageBox(hWnd, (LPCWSTR)"Error when connect!", (LPCWSTR)"Error", 0);
			return 0;
		}
	}
	return 1;
}

//socket断开连接
inline void NetDisconn()
{
	closesocket(tcps);
}

//发送缓冲区中用户输入的数据
inline int NetSend()
{
	//如果正处于发送阻塞中，不能发送
	if (beSendBlock) return 0;
	//循环不断发送
	while (sendpos<cur)
	{
		int rs = send(tcps, (const char*)(buf + sendpos), cur - sendpos, 0);
		if (SOCKET_ERROR == rs)
		{
			if (WSAEWOULDBLOCK == WSAGetLastError())
				//如果socket缓冲区已满，要等到收到FD_WRITE消息才能发送
			{
				//设置发送阻塞标记为有
				beSendBlock = TRUE;
				//将输入控件设为只读
				SendMessage(hIWnd, EM_SETREADONLY, TRUE, 0);
				return 0;
			}
		}
		else
		{
			sendpos += rs;
		}
	}
	return 1;
}

//处理socket消息，这个函数最为关键
inline void ProcessSocketMsg(WPARAM wParam, LPARAM lParam)
{
	//错误处理
	if (WSAGETSELECTERROR(lParam))
	{
		switch (WSAGETSELECTERROR(lParam))
		{
		case WSAETIMEDOUT: MessageBox(hWnd, (LPCWSTR)"Connect timeout!", (LPCWSTR)"Error", 0); break;
		default: MessageBox(hWnd, (LPCWSTR)"Some error occurs!", (LPCWSTR)"Error", 0); break;
		}
		NetDisconn();
		SendMessage(hConnBtn, WM_SETTEXT, 0, (long)"Connect");
		EnableWindow(hIPEdit, TRUE);
		EnableWindow(hPortEdit, TRUE);
		EnableWindow(hConnBtn, TRUE);
		EnableWindow(hListenBtn, TRUE);
		conned = FALSE;
		listening = FALSE;
		return;
	}
	else
	{
		//重要的几个socket消息的处理
		switch (WSAGETSELECTEVENT(lParam))
		{
			//对方连接到本机
		case FD_ACCEPT:
			tcps = accept(tcps_listen, NULL, NULL);
			closesocket(tcps_listen);
			SendMessage(hConnBtn, WM_SETTEXT, 0, (long)"Disconnect");
			SendMessage(hListenBtn, WM_SETTEXT, 0, (long)"Listen");
			SendMessage(hIWnd, EM_SETREADONLY, FALSE, 0);
			EnableWindow(hListenBtn, FALSE);
			EnableWindow(hConnBtn, TRUE);
			listening = FALSE;
			conned = TRUE;
			break;
			//连接对方成功
		case FD_CONNECT:
			SendMessage(hConnBtn, WM_SETTEXT, 0, (long)"Disconnect");
			EnableWindow(hConnBtn, TRUE);
			SendMessage(hIWnd, EM_SETREADONLY, FALSE, 0);
			conned = TRUE;
			break;
			//对方关闭连接
		case FD_CLOSE:
			MessageBox(hWnd, (LPCWSTR)"Remote close the connection!", (LPCWSTR)"OK", 0);
			NetDisconn();
			SendMessage(hConnBtn, WM_SETTEXT, 0, (long)"Connect");
			EnableWindow(hIPEdit, TRUE);
			EnableWindow(hPortEdit, TRUE);
			EnableWindow(hListenBtn, TRUE);
			SendMessage(hIWnd, EM_SETREADONLY, TRUE, 0);
			conned = FALSE;
			break;
			//收到对方发来的数据
		case FD_READ:
			if (cur >= BUFSIZE - 1) ClearBuf();
			//读到缓冲区中
			cur += recv(tcps, (char*)(buf + cur), BUFSIZE - cur - 1, 0);
			//更新输出文本显示控件的内容
			SendMessage(hOWnd, WM_SETTEXT, 0, (long)buf);
			break;
			//发送缓冲区变空，指示可以继续发送
		case FD_WRITE:
			//如果上次发送由于缓冲区满而被阻塞
			if (beSendBlock)
			{
				//设置发送阻塞标记为无
				beSendBlock = FALSE;
				//继续发送
				if (NetSend())
				{
					SendMessage(hOWnd, WM_SETTEXT, 0, (long)buf);
					SendMessage(hIWnd, WM_SETTEXT, 0, NULL);
					SendMessage(hIWnd, EM_SETREADONLY, FALSE, 0);
				}
			}
			break;
		}
	}
}
