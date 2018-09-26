#define WM_SOCKET (WM_USER+1)

SOCKET tcps, tcps_listen;
BOOL beSendBlock = FALSE;

//socket��ʼ��
inline int NetInit()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(1, 1);
	//��ʼ��winsock
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		MessageBox(NULL, (LPCWSTR)"Winsock startup error!", (LPCWSTR)"Error", 0);
		return 0;
	}
	//���winsock�汾
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		MessageBox(NULL, (LPCWSTR)"Invalid winsock version!", (LPCWSTR)"Error", 0);
		return 0;
	}
	return 1;
}

//socket�ر�
inline void NetEnd()
{
	closesocket(tcps);
	WSACleanup();
}

//socket����
inline int NetListen(int port)
{
	//����ջ�����
	ClearBuf();
	//����socket
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
	//��socket
	int err = bind(tcps_listen, (SOCKADDR*)&tcpaddr, sizeof(tcpaddr));
	if (err != 0)
	{
		MessageBox(NULL,(LPCWSTR)"Bind error!", (LPCWSTR)"Error", 0);
		return 0;
	}
	//����
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

//socket����
inline int NetConn(LPCSTR ipstr, int port)
{
	//����ջ�����
	ClearBuf();
	//����socket
	tcps = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tcps == INVALID_SOCKET)
	{
		MessageBox(NULL, (LPCWSTR)"Can't create socket!", (LPCWSTR)"Error", 0);
		return 0;
	}
	//�����첽ѡ��ģʽ��ע����ĵ�socket�¼�
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
	//����
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

//socket�Ͽ�����
inline void NetDisconn()
{
	closesocket(tcps);
}

//���ͻ��������û����������
inline int NetSend()
{
	//��������ڷ��������У����ܷ���
	if (beSendBlock) return 0;
	//ѭ�����Ϸ���
	while (sendpos<cur)
	{
		int rs = send(tcps, (const char*)(buf + sendpos), cur - sendpos, 0);
		if (SOCKET_ERROR == rs)
		{
			if (WSAEWOULDBLOCK == WSAGetLastError())
				//���socket������������Ҫ�ȵ��յ�FD_WRITE��Ϣ���ܷ���
			{
				//���÷����������Ϊ��
				beSendBlock = TRUE;
				//������ؼ���Ϊֻ��
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

//����socket��Ϣ�����������Ϊ�ؼ�
inline void ProcessSocketMsg(WPARAM wParam, LPARAM lParam)
{
	//������
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
		//��Ҫ�ļ���socket��Ϣ�Ĵ���
		switch (WSAGETSELECTEVENT(lParam))
		{
			//�Է����ӵ�����
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
			//���ӶԷ��ɹ�
		case FD_CONNECT:
			SendMessage(hConnBtn, WM_SETTEXT, 0, (long)"Disconnect");
			EnableWindow(hConnBtn, TRUE);
			SendMessage(hIWnd, EM_SETREADONLY, FALSE, 0);
			conned = TRUE;
			break;
			//�Է��ر�����
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
			//�յ��Է�����������
		case FD_READ:
			if (cur >= BUFSIZE - 1) ClearBuf();
			//������������
			cur += recv(tcps, (char*)(buf + cur), BUFSIZE - cur - 1, 0);
			//��������ı���ʾ�ؼ�������
			SendMessage(hOWnd, WM_SETTEXT, 0, (long)buf);
			break;
			//���ͻ�������գ�ָʾ���Լ�������
		case FD_WRITE:
			//����ϴη������ڻ���������������
			if (beSendBlock)
			{
				//���÷����������Ϊ��
				beSendBlock = FALSE;
				//��������
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
