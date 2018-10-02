#pragma once
#include <winsock2.h>
#include<cstdio>
#include<ctime>
#include<vector>

#define TIMEFOR_THREAD_CLIENT		500		//�߳�˯��ʱ��

#define	MAX_NUM_CLIENT		10				//���ܵĿͻ��������������
#define	MAX_NUM_BUF			48				//����������󳤶�
#define INVALID_OPERATOR	1				//��Ч�Ĳ�����
#define INVALID_NUM			2				//��ĸΪ��
#define ZERO				0				//��

//���ݰ�����
//#define EXPRESSION			'E'				//��������ʽ
#define BYEBYE				'B'				//��Ϣbyebye
#define TIME				'T'				//��ȡʱ��
#define EXIT				'E'				//�˳�
#define NAME				'N'				//��ȡ����
#define LIST				'L'				//��ȡ�ͻ����б�
#define SEND				'S'				//�������ͻ��˷�����Ϣ
#define HEADERLEN			(sizeof(hdr))	//ͷ����

//���ݰ�ͷ�ṹ���ýṹ��win32��Ϊ4byte
typedef struct _head
{
	char			type;	//����		
	unsigned short	len;	//���ݰ��ĳ���(����ͷ�ĳ���)
}hdr, *phdr;

//���ݰ��е����ݽṹ
typedef struct _data
{
	char	buf[MAX_NUM_BUF];//����
}DATABUF, *pDataBuf;

typedef struct _package {
	hdr head;
	DATABUF data;
}PACKAGE, *pPackage;

class superServer;
class CClient
{
public:
	CClient(const SOCKET sClient, const sockaddr_in &addrClient, superServer* super);
	virtual ~CClient();

public:
	superServer* Super;
	BOOL IntToChar(int total, int index, char* des);
	std::vector<PACKAGE> DataConvert(char* str, int type);
	BOOL		StartRuning(void);					//�������ͺͽ��������߳�
	void		HandleData(const char* pExpr);		//�������ʽ
	BOOL		IsConning(void) {					//�Ƿ����Ӵ���
		return m_bConning;
	}
	void		DisConning(void) {					//�Ͽ���ͻ��˵�����
		m_bConning = FALSE;
	}
	BOOL		IsExit(void) {						//���պͷ����߳��Ƿ��Ѿ��˳�
		return m_bExit;
	}

public:
	static DWORD __stdcall	 RecvDataThread(void* pParam);		//���տͻ�������
	static DWORD __stdcall	 SendDataThread(void* pParam);		//��ͻ��˷�������

private:
	CClient();
private:
	SOCKET		m_socket;			//�׽���
	sockaddr_in	m_addr;				//��ַ
	DATABUF		m_data;				//����
	HANDLE		m_hEvent;			//�¼�����
	HANDLE		m_hThreadSend;		//���������߳̾��
	HANDLE		m_hThreadRecv;		//���������߳̾��
	CRITICAL_SECTION m_cs;			//�ٽ�������
	BOOL		m_bConning;			//�ͻ�������״̬
	BOOL		m_bExit;			//�߳��˳�
};