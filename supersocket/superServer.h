#ifndef _SUPER_SERVER
#define _SUPER_SERVER
#include<iostream>
#include<WinSock2.h>
#include<list>
#include<queue>
#include<map>
#include"superClient.h"

#define SERVERPORT			5556			//服务器TCP端口
#define SERVER_SETUP_FAIL	1				//启动服务器失败

#define TIMEFOR_THREAD_EXIT			5000	//主线程睡眠时间
#define TIMEFOR_THREAD_HELP			1500	//清理资源线程退出时间
#define TIMEFOR_THREAD_SLEEP		500		//等待客户端请求线程睡眠时间

typedef std::list <CClient*> CLIENTLIST;			//链表

struct Message {
	int iSrcID;
	int iDesID;
	DATABUF pData;
};

class superServer {
public:
	static std::queue<Message>MessageQueue;
	static HANDLE	hThreadAccept;						//接受客户端连接线程句柄
	static HANDLE	hThreadHelp;						//释放资源线程句柄
	static HANDLE	hThreadDistributeMsg;

	static SOCKET	sServer;							//监听套接字
	static BOOL	bServerRunning;						//服务器的工作状态
	static HANDLE	hServerEvent;						//关闭服务器事件对象
	static CLIENTLIST clientlist;				//管理连接的链表
	static CRITICAL_SECTION	csClientList;			//保护链表的临界区对象
	static CRITICAL_SECTION csMessageQueue;
	static CRITICAL_SECTION csClientTable;
	static std::map<int, CClient*>mClientTable;
	static int iCount;
	BOOL	InitSever(void);					//初始化
	BOOL	StartService(void);					//启动服务
	void	StopService(void);					//停止服务
	BOOL	CreateHelperAndAcceptThread(void);	//创建接收客户端连接线程
	void	ExitServer(void);					//服务器退出

	void	InitMember(void);					//初始化全局变量
	BOOL	InitSocket(void);					//初始化SOCKET

	void	ShowTipMsg(BOOL bFirstInput);		//显示提示信息
	void	ShowServerStartMsg(BOOL bSuc);		//显示服务器已经启动
	void	ShowServerExitMsg(void);			//显示服务器正在退出


	static DWORD WINAPI HelperThread(LPVOID pParam);			//释放资源
	static DWORD WINAPI AcceptThread(LPVOID pParam);			//接受客户端连接线程
	static DWORD WINAPI DistributeMessageThread(LPVOID pParam);
	void	ShowConnectNum();								//显示客户端的连接数目	
};


#endif
