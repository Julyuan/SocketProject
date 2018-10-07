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
//#define EXPRESSION			'E'				//�������ʽ
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

class superServer;
class CClient
{
public:
	CClient(const SOCKET sClient, const sockaddr_in &addrClient, superServer* super, int ID);
	virtual ~CClient();

public:
	DATABUF		m_data;				//����
	CRITICAL_SECTION m_cs;			//�ٽ�������
	HANDLE		m_hEvent;			//�¼�����
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
	BOOL		StartRuning(void);					//�������ͺͽ��������߳�
	void		HandleData(const char* pExpr);		//������ʽ

	static void		OutputPackageInBinary(const char* src, int len);
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
	HANDLE		m_hThreadSend;		//���������߳̾��
	HANDLE		m_hThreadRecv;		//���������߳̾��
	BOOL		m_bConning;			//�ͻ�������״̬
	BOOL		m_bExit;			//�߳��˳�
};
