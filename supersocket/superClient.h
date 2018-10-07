#pragma once
#include <winsock2.h>
#include<cstdio>
#include<ctime>
#include<vector>

#define TIMEFOR_THREAD_CLIENT		500		//线程睡眠时间

#define	MAX_NUM_CLIENT		10				//接受的客户端连接最多数量
#define	MAX_NUM_BUF			48				//缓冲区的最大长度
#define INVALID_OPERATOR	1				//无效的操作符
#define INVALID_NUM			2				//分母为零
#define ZERO				0				//零

//数据包类型
//#define EXPRESSION			'E'				//算数表达式
#define BYEBYE				'B'				//消息byebye
#define TIME				'T'				//获取时间
#define EXIT				'E'				//退出
#define NAME				'N'				//获取名字
#define LIST				'L'				//获取客户端列表
#define SEND				'S'				//向其他客户端发送消息
#define HEADERLEN			(sizeof(hdr))	//头长度

//数据包头结构，该结构在win32下为4byte
typedef struct _head
{
	char			type;	//类型		
	unsigned short	len;	//数据包的长度(包括头的长度)
}hdr, *phdr;

//数据包中的数据结构
typedef struct _data
{
	char	buf[MAX_NUM_BUF];//数据
}DATABUF, *pDataBuf;

class superServer;
class CClient
{
public:
	CClient(const SOCKET sClient, const sockaddr_in &addrClient, superServer* super, int ID);
	virtual ~CClient();

public:
	DATABUF		m_data;				//数据
	CRITICAL_SECTION m_cs;			//临界区对象
	HANDLE		m_hEvent;			//事件对象
	int			m_iID;

	superServer* Super;
	BOOL IntToChar(int total, int index, char* des);
	static BOOL ImformationEncapsulation(char* caTerm, char* cID, char* caIP, char* caPort) {
		memcpy(caTerm, cID,1);
		memcpy(caTerm + 1, caIP, 4);
		memcpy(caTerm + 5, caPort, 2);
		return TRUE;
	}
	std::vector<DATABUF> DataConvert(char* str, int type);
	BOOL		StartRuning(void);					//创建发送和接收数据线程
	void		HandleData(const char* pExpr);		//计算表达式

	static void		OutputPackageInBinary(const char* src, int len);
	BOOL		IsConning(void) {					//是否连接存在
		return m_bConning;
	}
	void		DisConning(void) {					//断开与客户端的连接
		m_bConning = FALSE;
	}
	BOOL		IsExit(void) {						//接收和发送线程是否已经退出
		return m_bExit;
	}

public:
	static DWORD __stdcall	 RecvDataThread(void* pParam);		//接收客户端数据
	static DWORD __stdcall	 SendDataThread(void* pParam);		//向客户端发送数据

private:
	CClient();

private:
	SOCKET		m_socket;			//套接字
	sockaddr_in	m_addr;				//地址
	HANDLE		m_hThreadSend;		//发送数据线程句柄
	HANDLE		m_hThreadRecv;		//接收数据线程句柄
	BOOL		m_bConning;			//客户端连接状态
	BOOL		m_bExit;			//线程退出
};
