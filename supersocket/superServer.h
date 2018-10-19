#ifndef _SUPER_SERVER
#define _SUPER_SERVER
#include<iostream>
#include<WinSock2.h>
#include<list>
#include<queue>
#include<map>
#include"superClient.h"

#define SERVERPORT			5556			//������TCP�˿�
#define SERVER_SETUP_FAIL	1				//����������ʧ��

#define TIMEFOR_THREAD_EXIT			5000	//���߳�˯��ʱ��
#define TIMEFOR_THREAD_HELP			1500	//������Դ�߳��˳�ʱ��
#define TIMEFOR_THREAD_SLEEP		500		//�ȴ��ͻ��������߳�˯��ʱ��

typedef std::list <CClient*> CLIENTLIST;			//����

struct Message {
	int iSrcID;
	int iDesID;
	DATABUF pData;
};

class superServer {
public:
	static std::queue<Message>MessageQueue;
	static HANDLE	hThreadAccept;						//���ܿͻ��������߳̾��
	static HANDLE	hThreadHelp;						//�ͷ���Դ�߳̾��
	static HANDLE	hThreadDistributeMsg;

	static SOCKET	sServer;							//�����׽���
	static BOOL	bServerRunning;						//�������Ĺ���״̬
	static HANDLE	hServerEvent;						//�رշ������¼�����
	static CLIENTLIST clientlist;				//�������ӵ�����
	static CRITICAL_SECTION	csClientList;			//����������ٽ�������
	static CRITICAL_SECTION csMessageQueue;
	static CRITICAL_SECTION csClientTable;
	static std::map<int, CClient*>mClientTable;
	static int iCount;
	BOOL	InitSever(void);					//��ʼ��
	BOOL	StartService(void);					//��������
	void	StopService(void);					//ֹͣ����
	BOOL	CreateHelperAndAcceptThread(void);	//�������տͻ��������߳�
	void	ExitServer(void);					//�������˳�

	void	InitMember(void);					//��ʼ��ȫ�ֱ���
	BOOL	InitSocket(void);					//��ʼ��SOCKET

	void	ShowTipMsg(BOOL bFirstInput);		//��ʾ��ʾ��Ϣ
	void	ShowServerStartMsg(BOOL bSuc);		//��ʾ�������Ѿ�����
	void	ShowServerExitMsg(void);			//��ʾ�����������˳�


	static DWORD WINAPI HelperThread(LPVOID pParam);			//�ͷ���Դ
	static DWORD WINAPI AcceptThread(LPVOID pParam);			//���ܿͻ��������߳�
	static DWORD WINAPI DistributeMessageThread(LPVOID pParam);
	void	ShowConnectNum();								//��ʾ�ͻ��˵�������Ŀ	
};


#endif
