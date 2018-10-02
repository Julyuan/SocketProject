// Server.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <afx.h>
#include <winbase.h>
#include <winsock2.h>
#include <process.h>
#include <list>
#include "superServer.h"

/*
* 变量的第一个字母为小写且该字母表明该变量的类型，如 nErrCode.为整形的错误代码
* 函数的第一个字母为大写且表明函数的功能。如void AcceptThread(void).为接受连接的线程
*/
#pragma comment(lib, "ws2_32.lib")

using namespace std;

HANDLE superServer::hServerEvent = CreateEvent(NULL, TRUE, FALSE, NULL);	//手动设置事件，初始化为无信息号状态
HANDLE superServer::hThreadAccept = NULL;									//设置为NULL
HANDLE superServer::hThreadHelp = NULL;										//设置为NULL
SOCKET superServer::sServer = INVALID_SOCKET;								//设置为无效的套接字
BOOL superServer::bServerRunning = FALSE;									//服务器为没有运行状态
CLIENTLIST superServer::clientlist;
CRITICAL_SECTION superServer::csClientList;
int superServer::iCount = 0;


/**
* 初始化
*/
BOOL	superServer::InitSever(void)
{

	InitMember();//初始化全局变量

				 //初始化SOCKET
	if (!InitSocket())
		return FALSE;

	return TRUE;
}


/**
* 初始化全局变量
*/
void	superServer::InitMember(void)
{
	InitializeCriticalSection(&csClientList);				//初始化临界区
	clientlist.clear();										//清空链表
}

/**
*  初始化SOCKET
*/
BOOL superServer::InitSocket(void)
{
	//返回值
	int reVal;

	//初始化Windows Sockets DLL
	WSADATA  wsData;
	reVal = WSAStartup(MAKEWORD(2, 2), &wsData);

	//创建套接字			
	sServer = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == sServer)
		return FALSE;

	//设置套接字非阻塞模式
	unsigned long ul = 1;
	reVal = ioctlsocket(sServer, FIONBIO, (unsigned long*)&ul);
	if (SOCKET_ERROR == reVal)
		return FALSE;

	//绑定套接字
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(SERVERPORT);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	reVal = bind(sServer, (struct sockaddr*)&serAddr, sizeof(serAddr));
	if (SOCKET_ERROR == reVal)
		return FALSE;

	//监听
	reVal = listen(sServer, SOMAXCONN);
	if (SOCKET_ERROR == reVal)
		return FALSE;

	return TRUE;
}

/**
*  启动服务
*/
BOOL	superServer::StartService(void)
{
	BOOL reVal = TRUE;	//返回值

	ShowTipMsg(TRUE);	//提示用户输入

	char cInput;		//输入字符		
	do
	{
		cin >> cInput;
		if ('s' == cInput || 'S' == cInput)
		{
			if (CreateHelperAndAcceptThread())	//创建清理资源和接受客户端请求的线程
			{
				ShowServerStartMsg(TRUE);		//创建线程成功信息
			}
			else {
				reVal = FALSE;
			}
			break;//跳出循环体

		}
		else {
			ShowTipMsg(TRUE);
		}

	} while (cInput != 's' &&//必须输入's'或者'S'字符
		cInput != 'S');

	return reVal;
}

/**
*  停止服务
*/
void	superServer::StopService(void)
{
	BOOL reVal = TRUE;	//返回值

	ShowTipMsg(FALSE);	//提示用户输入

	char cInput;		//输入的操作字符
	for (; bServerRunning;)
	{
		cin >> cInput;
		if (cInput == 'E' || cInput == 'e')
		{
			if (IDOK == MessageBox(NULL, (LPCWSTR)"Are you sure?", //等待用户确认退出的消息框
				(LPCWSTR)"Server", MB_OKCANCEL))
			{
				break;//跳出循环体
			}
			else {
				Sleep(TIMEFOR_THREAD_EXIT);	//线程睡眠
			}
		}
		else {
			Sleep(TIMEFOR_THREAD_EXIT);		//线程睡眠
		}
	}

	bServerRunning = FALSE;		//服务器退出

	ShowServerExitMsg();		//显示服务器退出信息

	Sleep(TIMEFOR_THREAD_EXIT);	//给其他线程时间退出

	WaitForSingleObject(hServerEvent, INFINITE);//等待清理资源线程发送的事件

	return;
}

/**
*  显示提示信息
*/
void	superServer::ShowTipMsg(BOOL bFirstInput)
{
	if (bFirstInput)//第一次
	{
		cout << endl;
		cout << endl;
		cout << "**********************" << endl;
		cout << "*                    *" << endl;
		cout << "* s(S): Start server *" << endl;
		cout << "*                    *" << endl;
		cout << "**********************" << endl;
		cout << "Please input:" << endl;

	}
	else {//退出服务器		
		cout << endl;
		cout << endl;
		cout << "**********************" << endl;
		cout << "*                    *" << endl;
		cout << "* e(E): Exit  server *" << endl;
		cout << "*                    *" << endl;
		cout << "**********************" << endl;
		cout << " Please input:" << endl;
	}
}
/**
*  释放资源
*/
void  superServer::ExitServer(void)
{
	DeleteCriticalSection(&csClientList);	//释放临界区对象
	CloseHandle(hServerEvent);				//释放事件对象句柄
	closesocket(sServer);					//关闭SOCKET					
	WSACleanup();							//卸载Windows Sockets DLL
}



/**
* 创建释放资源线程和接收客户端请求线程
*/
BOOL  superServer::CreateHelperAndAcceptThread(void)
{

	bServerRunning = TRUE;//设置服务器为运行状态

	unsigned long ulThreadId;
	hThreadHelp = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HelperThread, this, 0, &ulThreadId);
	if (NULL == hThreadHelp)
	{
		bServerRunning = FALSE;
		return FALSE;
	}
	else {
		CloseHandle(hThreadHelp);
	}

	//创建接收客户端请求线程
	hThreadAccept = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AcceptThread, this, 0, &ulThreadId);
	if (NULL == hThreadAccept)
	{
		bServerRunning = FALSE;
		return FALSE;
	}
	else {
		CloseHandle(hThreadAccept);
	}

	return TRUE;
}

/**
* 显示启动服务器成功与失败消息
*/
void  superServer::ShowServerStartMsg(BOOL bSuc)
{
	if (bSuc)
	{
		cout << "**********************" << endl;
		cout << "*                    *" << endl;
		cout << "* Server succeeded!  *" << endl;
		cout << "*                    *" << endl;
		cout << "**********************" << endl;
	}
	else {
		cout << "**********************" << endl;
		cout << "*                    *" << endl;
		cout << "* Server failed   !  *" << endl;
		cout << "*                    *" << endl;
		cout << "**********************" << endl;
	}

}

/**
* 显示退出服务器消息
*/
void  superServer::ShowServerExitMsg(void)
{

	cout << "**********************" << endl;
	cout << "*                    *" << endl;
	cout << "* Server exit...     *" << endl;
	cout << "*                    *" << endl;
	cout << "**********************" << endl;
}


DWORD WINAPI superServer::DistributeMessageThread(LPVOID pParam) {
	
	superServer* pServer = (superServer*)pParam;
	for (; bServerRunning;) {
		while (!pServer->MessageQueue.empty()) {
			Message temp = pServer->MessageQueue.front();
			pServer->MessageQueue.pop();
			CClient*des = pServer->mClientTable[temp.iDesID];
			EnterCriticalSection(&des->m_cs);
			phdr pHeaderSend = (phdr)des->m_data.buf;				//发送的数据		
			pHeaderSend->type = temp.pData.head.type;				//单词类型
			pHeaderSend->len = temp.pData.head.len;		//数据包长度
			memcpy(des->m_data.buf + HEADERLEN, &temp.pData.data, pHeaderSend->len-HEADERLEN);	//复制数据到m_data"
			LeaveCriticalSection(&des->m_cs);
			SetEvent(des->m_hEvent);	//通知发送数据线程
		}


	}
	return 0;
}
/**
* 接受客户端连接
*/
DWORD WINAPI superServer::AcceptThread(LPVOID pParam)
{
	superServer* super = (superServer*)pParam;
	SOCKET  sAccept;							//接受客户端连接的套接字
	sockaddr_in addrClient;						//客户端SOCKET地址

	for (; bServerRunning;)						//服务器的状态
	{
		memset(&addrClient, 0, sizeof(sockaddr_in));					//初始化
		int			lenClient = sizeof(sockaddr_in);					//地址长度
		sAccept = accept(sServer, (sockaddr*)&addrClient, &lenClient);	//接受客户请求

		if (INVALID_SOCKET == sAccept)
		{
			int nErrCode = WSAGetLastError();
			if (nErrCode == WSAEWOULDBLOCK)	//无法立即完成一个非阻挡性套接字操作
			{
				Sleep(TIMEFOR_THREAD_SLEEP);
				continue;//继续等待
			}
			else {
				return 0;//线程退出
			}

		}
		else//接受客户端的请求
		{

			CClient *pClient = new CClient(sAccept, addrClient, super, iCount++);	//创建客户端对象	
			mClientTable.insert(std::pair<int, CClient*>(iCount - 1, pClient));		
			EnterCriticalSection(&csClientList);				//进入在临界区
			clientlist.push_back(pClient);						//加入链表
			LeaveCriticalSection(&csClientList);				//离开临界区

			pClient->StartRuning();								//为接受的客户端建立接收数据和发送数据线程			
		}
	}

	return 0;//线程退出
}

/**
* 清理资源
*/
DWORD WINAPI superServer::HelperThread(LPVOID pParam)
{
	for (; bServerRunning;)//服务器正在运行
	{
		EnterCriticalSection(&csClientList);//进入临界区

		
												//清理已经断开的连接客户端内存空间
		auto iter = clientlist.begin();
		for (iter; iter != clientlist.end();)
		{
			CClient *pClient = (CClient*)*iter;
			if (pClient->IsExit())			//客户端线程已经退出
			{
				clientlist.erase(iter++);	//删除节点
				delete pClient;				//释放内存
				pClient = NULL;
			}
			else {
				iter++;						//指针下移
			}
		}

		LeaveCriticalSection(&csClientList);//离开临界区

		Sleep(TIMEFOR_THREAD_HELP);
	}


	//服务器停止工作
	if (!bServerRunning)
	{
		//断开每个连接,线程退出
		EnterCriticalSection(&csClientList);
		auto iter = clientlist.begin();
		for (iter; iter != clientlist.end();)
		{
			CClient *pClient = (CClient*)*iter;
			//如果客户端的连接还存在，则断开连接，线程退出
			if (pClient->IsConning())
			{
				pClient->DisConning();
			}
			++iter;
		}
		//离开临界区
		LeaveCriticalSection(&csClientList);

		//给连接客户端线程时间，使其自动退出
		Sleep(TIMEFOR_THREAD_SLEEP);

		//进入临界区
		EnterCriticalSection(&csClientList);

		//确保为每个客户端分配的内存空间都回收。
		//如果不加入while这层循环，可能存在这样的情况。当pClient->IsExit()时，该线程还没有退出。
		//那么就需要从链表的开始部分重新判断。
		while (0 != clientlist.size())
		{
			iter = clientlist.begin();
			for (iter; iter != clientlist.end();)
			{
				CClient *pClient = (CClient*)*iter;
				if (pClient->IsExit())			//客户端线程已经退出
				{
					clientlist.erase(iter++);	//删除节点
					delete pClient;				//释放内存空间
					pClient = NULL;
				}
				else {
					iter++;						//指针下移
				}
			}
			//给连接客户端线程时间，使其自动退出
			Sleep(TIMEFOR_THREAD_SLEEP);
		}
		LeaveCriticalSection(&csClientList);//离开临界区

	}

	clientlist.clear();		//清空链表

	SetEvent(hServerEvent);	//通知主线程退出

	return 0;
}
