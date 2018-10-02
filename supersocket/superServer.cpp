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
* �����ĵ�һ����ĸΪСд�Ҹ���ĸ�����ñ��������ͣ��� nErrCode.Ϊ���εĴ������
* �����ĵ�һ����ĸΪ��д�ұ��������Ĺ��ܡ���void AcceptThread(void).Ϊ�������ӵ��߳�
*/
#pragma comment(lib, "ws2_32.lib")

using namespace std;

HANDLE superServer::hServerEvent = CreateEvent(NULL, TRUE, FALSE, NULL);	//�ֶ������¼�����ʼ��Ϊ����Ϣ��״̬
HANDLE superServer::hThreadAccept = NULL;									//����ΪNULL
HANDLE superServer::hThreadHelp = NULL;										//����ΪNULL
SOCKET superServer::sServer = INVALID_SOCKET;								//����Ϊ��Ч���׽���
BOOL superServer::bServerRunning = FALSE;									//������Ϊû������״̬
CLIENTLIST superServer::clientlist;
CRITICAL_SECTION superServer::csClientList;
int superServer::iCount = 0;


/**
* ��ʼ��
*/
BOOL	superServer::InitSever(void)
{

	InitMember();//��ʼ��ȫ�ֱ���

				 //��ʼ��SOCKET
	if (!InitSocket())
		return FALSE;

	return TRUE;
}


/**
* ��ʼ��ȫ�ֱ���
*/
void	superServer::InitMember(void)
{
	InitializeCriticalSection(&csClientList);				//��ʼ���ٽ���
	clientlist.clear();										//�������
}

/**
*  ��ʼ��SOCKET
*/
BOOL superServer::InitSocket(void)
{
	//����ֵ
	int reVal;

	//��ʼ��Windows Sockets DLL
	WSADATA  wsData;
	reVal = WSAStartup(MAKEWORD(2, 2), &wsData);

	//�����׽���			
	sServer = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == sServer)
		return FALSE;

	//�����׽��ַ�����ģʽ
	unsigned long ul = 1;
	reVal = ioctlsocket(sServer, FIONBIO, (unsigned long*)&ul);
	if (SOCKET_ERROR == reVal)
		return FALSE;

	//���׽���
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(SERVERPORT);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	reVal = bind(sServer, (struct sockaddr*)&serAddr, sizeof(serAddr));
	if (SOCKET_ERROR == reVal)
		return FALSE;

	//����
	reVal = listen(sServer, SOMAXCONN);
	if (SOCKET_ERROR == reVal)
		return FALSE;

	return TRUE;
}

/**
*  ��������
*/
BOOL	superServer::StartService(void)
{
	BOOL reVal = TRUE;	//����ֵ

	ShowTipMsg(TRUE);	//��ʾ�û�����

	char cInput;		//�����ַ�		
	do
	{
		cin >> cInput;
		if ('s' == cInput || 'S' == cInput)
		{
			if (CreateHelperAndAcceptThread())	//����������Դ�ͽ��ܿͻ���������߳�
			{
				ShowServerStartMsg(TRUE);		//�����̳߳ɹ���Ϣ
			}
			else {
				reVal = FALSE;
			}
			break;//����ѭ����

		}
		else {
			ShowTipMsg(TRUE);
		}

	} while (cInput != 's' &&//��������'s'����'S'�ַ�
		cInput != 'S');

	return reVal;
}

/**
*  ֹͣ����
*/
void	superServer::StopService(void)
{
	BOOL reVal = TRUE;	//����ֵ

	ShowTipMsg(FALSE);	//��ʾ�û�����

	char cInput;		//����Ĳ����ַ�
	for (; bServerRunning;)
	{
		cin >> cInput;
		if (cInput == 'E' || cInput == 'e')
		{
			if (IDOK == MessageBox(NULL, (LPCWSTR)"Are you sure?", //�ȴ��û�ȷ���˳�����Ϣ��
				(LPCWSTR)"Server", MB_OKCANCEL))
			{
				break;//����ѭ����
			}
			else {
				Sleep(TIMEFOR_THREAD_EXIT);	//�߳�˯��
			}
		}
		else {
			Sleep(TIMEFOR_THREAD_EXIT);		//�߳�˯��
		}
	}

	bServerRunning = FALSE;		//�������˳�

	ShowServerExitMsg();		//��ʾ�������˳���Ϣ

	Sleep(TIMEFOR_THREAD_EXIT);	//�������߳�ʱ���˳�

	WaitForSingleObject(hServerEvent, INFINITE);//�ȴ�������Դ�̷߳��͵��¼�

	return;
}

/**
*  ��ʾ��ʾ��Ϣ
*/
void	superServer::ShowTipMsg(BOOL bFirstInput)
{
	if (bFirstInput)//��һ��
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
	else {//�˳�������		
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
*  �ͷ���Դ
*/
void  superServer::ExitServer(void)
{
	DeleteCriticalSection(&csClientList);	//�ͷ��ٽ�������
	CloseHandle(hServerEvent);				//�ͷ��¼�������
	closesocket(sServer);					//�ر�SOCKET					
	WSACleanup();							//ж��Windows Sockets DLL
}



/**
* �����ͷ���Դ�̺߳ͽ��տͻ��������߳�
*/
BOOL  superServer::CreateHelperAndAcceptThread(void)
{

	bServerRunning = TRUE;//���÷�����Ϊ����״̬

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

	//�������տͻ��������߳�
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
* ��ʾ�����������ɹ���ʧ����Ϣ
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
* ��ʾ�˳���������Ϣ
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
			phdr pHeaderSend = (phdr)des->m_data.buf;				//���͵�����		
			pHeaderSend->type = temp.pData.head.type;				//��������
			pHeaderSend->len = temp.pData.head.len;		//���ݰ�����
			memcpy(des->m_data.buf + HEADERLEN, &temp.pData.data, pHeaderSend->len-HEADERLEN);	//�������ݵ�m_data"
			LeaveCriticalSection(&des->m_cs);
			SetEvent(des->m_hEvent);	//֪ͨ���������߳�
		}


	}
	return 0;
}
/**
* ���ܿͻ�������
*/
DWORD WINAPI superServer::AcceptThread(LPVOID pParam)
{
	superServer* super = (superServer*)pParam;
	SOCKET  sAccept;							//���ܿͻ������ӵ��׽���
	sockaddr_in addrClient;						//�ͻ���SOCKET��ַ

	for (; bServerRunning;)						//��������״̬
	{
		memset(&addrClient, 0, sizeof(sockaddr_in));					//��ʼ��
		int			lenClient = sizeof(sockaddr_in);					//��ַ����
		sAccept = accept(sServer, (sockaddr*)&addrClient, &lenClient);	//���ܿͻ�����

		if (INVALID_SOCKET == sAccept)
		{
			int nErrCode = WSAGetLastError();
			if (nErrCode == WSAEWOULDBLOCK)	//�޷��������һ�����赲���׽��ֲ���
			{
				Sleep(TIMEFOR_THREAD_SLEEP);
				continue;//�����ȴ�
			}
			else {
				return 0;//�߳��˳�
			}

		}
		else//���ܿͻ��˵�����
		{

			CClient *pClient = new CClient(sAccept, addrClient, super, iCount++);	//�����ͻ��˶���	
			mClientTable.insert(std::pair<int, CClient*>(iCount - 1, pClient));		
			EnterCriticalSection(&csClientList);				//�������ٽ���
			clientlist.push_back(pClient);						//��������
			LeaveCriticalSection(&csClientList);				//�뿪�ٽ���

			pClient->StartRuning();								//Ϊ���ܵĿͻ��˽����������ݺͷ��������߳�			
		}
	}

	return 0;//�߳��˳�
}

/**
* ������Դ
*/
DWORD WINAPI superServer::HelperThread(LPVOID pParam)
{
	for (; bServerRunning;)//��������������
	{
		EnterCriticalSection(&csClientList);//�����ٽ���

		
												//�����Ѿ��Ͽ������ӿͻ����ڴ�ռ�
		auto iter = clientlist.begin();
		for (iter; iter != clientlist.end();)
		{
			CClient *pClient = (CClient*)*iter;
			if (pClient->IsExit())			//�ͻ����߳��Ѿ��˳�
			{
				clientlist.erase(iter++);	//ɾ���ڵ�
				delete pClient;				//�ͷ��ڴ�
				pClient = NULL;
			}
			else {
				iter++;						//ָ������
			}
		}

		LeaveCriticalSection(&csClientList);//�뿪�ٽ���

		Sleep(TIMEFOR_THREAD_HELP);
	}


	//������ֹͣ����
	if (!bServerRunning)
	{
		//�Ͽ�ÿ������,�߳��˳�
		EnterCriticalSection(&csClientList);
		auto iter = clientlist.begin();
		for (iter; iter != clientlist.end();)
		{
			CClient *pClient = (CClient*)*iter;
			//����ͻ��˵����ӻ����ڣ���Ͽ����ӣ��߳��˳�
			if (pClient->IsConning())
			{
				pClient->DisConning();
			}
			++iter;
		}
		//�뿪�ٽ���
		LeaveCriticalSection(&csClientList);

		//�����ӿͻ����߳�ʱ�䣬ʹ���Զ��˳�
		Sleep(TIMEFOR_THREAD_SLEEP);

		//�����ٽ���
		EnterCriticalSection(&csClientList);

		//ȷ��Ϊÿ���ͻ��˷�����ڴ�ռ䶼���ա�
		//���������while���ѭ�������ܴ����������������pClient->IsExit()ʱ�����̻߳�û���˳���
		//��ô����Ҫ������Ŀ�ʼ���������жϡ�
		while (0 != clientlist.size())
		{
			iter = clientlist.begin();
			for (iter; iter != clientlist.end();)
			{
				CClient *pClient = (CClient*)*iter;
				if (pClient->IsExit())			//�ͻ����߳��Ѿ��˳�
				{
					clientlist.erase(iter++);	//ɾ���ڵ�
					delete pClient;				//�ͷ��ڴ�ռ�
					pClient = NULL;
				}
				else {
					iter++;						//ָ������
				}
			}
			//�����ӿͻ����߳�ʱ�䣬ʹ���Զ��˳�
			Sleep(TIMEFOR_THREAD_SLEEP);
		}
		LeaveCriticalSection(&csClientList);//�뿪�ٽ���

	}

	clientlist.clear();		//�������

	SetEvent(hServerEvent);	//֪ͨ���߳��˳�

	return 0;
}
