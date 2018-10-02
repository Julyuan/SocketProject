#include <process.h>
#include "superClient.h"
#include "superServer.h"

/*
* ���캯��
*/
CClient::CClient(const SOCKET sClient, const sockaddr_in &addrClient, superServer* super, int ID)
{
	//��ʼ������
	m_iID = ID;
	m_hThreadRecv = NULL;
	m_hThreadSend = NULL;
	m_socket = sClient;
	m_addr = addrClient;
	m_bConning = FALSE;
	m_bExit = FALSE;
	memset(m_data.buf, 0, MAX_NUM_BUF);

	//�����¼�
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);//�ֶ������ź�״̬����ʼ��Ϊ���ź�״̬

													 //��ʼ���ٽ���
	InitializeCriticalSection(&m_cs);
}
/*
* ��������
*/
CClient::~CClient()
{
	closesocket(m_socket);			//�ر��׽���
	m_socket = INVALID_SOCKET;		//�׽�����Ч
	DeleteCriticalSection(&m_cs);	//�ͷ��ٽ�������	
	CloseHandle(m_hEvent);			//�ͷ��¼�����
}

/*
* �������ͺͽ��������߳�
*/
BOOL CClient::StartRuning(void)
{
	m_bConning = TRUE;//��������״̬
	printf("127\n");
					  
	unsigned long ulThreadId; //�������������߳�
	m_hThreadRecv = CreateThread(NULL, 0, RecvDataThread, this, 0, &ulThreadId);
	if (NULL == m_hThreadRecv)
	{
		return FALSE;
	}
	else {
		CloseHandle(m_hThreadRecv);
	}

	printf("128\n");
	//�������տͻ������ݵ��߳�
	m_hThreadSend = CreateThread(NULL, 0, SendDataThread, this, 0, &ulThreadId);
	if (NULL == m_hThreadSend)
	{
		return FALSE;
	}
	else {
		CloseHandle(m_hThreadSend);
	}

	return TRUE;
}


/*
* ���տͻ�������
*/
DWORD  CClient::RecvDataThread(void* pParam)
{
	printf("129\n");
	CClient *pClient = (CClient*)pParam;	//�ͻ��˶���ָ��
	int		reVal;							//����ֵ
	char	temp[MAX_NUM_BUF];				//��ʱ����

	memset(temp, 0, MAX_NUM_BUF);

	for (; pClient->m_bConning;)				//����״̬
	{
		reVal = recv(pClient->m_socket, temp, MAX_NUM_BUF, 0);	//��������

																//������󷵻�ֵ
		if (SOCKET_ERROR == reVal)
		{
			int nErrCode = WSAGetLastError();

			if (WSAEWOULDBLOCK == nErrCode)	//�������ݻ�����������
			{
				continue;						//����ѭ��
			}
			else if (WSAENETDOWN == nErrCode ||//�ͻ��˹ر�������
				WSAETIMEDOUT == nErrCode ||
				WSAECONNRESET == nErrCode)
			{
				break;							//�߳��˳�				
			}
		}

		//�ͻ��˹ر�������
		if (reVal == 0)
		{
			break;
		}

		//�յ�����
		if (reVal > HEADERLEN)
		{
			printf("130\n");
			pClient->HandleData(temp);		//��������


			memset(temp, 0, MAX_NUM_BUF);	//�����ʱ����
		}

		Sleep(TIMEFOR_THREAD_CLIENT);		//�߳�˯��
	}

	pClient->m_bConning = FALSE;			//��ͻ��˵����ӶϿ�
	SetEvent(pClient->m_hEvent);			//֪ͨ���������߳��˳�

	return 0;								//�߳��˳�
}

/*
* //��ͻ��˷�������
*/
DWORD CClient::SendDataThread(void* pParam)
{
	CClient *pClient = (CClient*)pParam;//ת����������ΪCClientָ��

	for (; pClient->m_bConning;)//����״̬
	{
		//�յ��¼�֪ͨ
		if (WAIT_OBJECT_0 == WaitForSingleObject(pClient->m_hEvent, INFINITE))
		{
			//���ͻ��˵����ӶϿ�ʱ�����������߳����˳���Ȼ����̺߳��˳����������˳���־
			if (!pClient->m_bConning)
			{
				pClient->m_bExit = TRUE;
				break;
			}

			//�����ٽ���
			EnterCriticalSection(&pClient->m_cs);
			//��������
			phdr pHeader = (phdr)pClient->m_data.buf;
			int nSendlen = pHeader->len;

			int val = send(pClient->m_socket, pClient->m_data.buf, nSendlen, 0);
			//�����ش���
			if (SOCKET_ERROR == val)
			{
				int nErrCode = WSAGetLastError();
				if (nErrCode == WSAEWOULDBLOCK)//�������ݻ�����������
				{
					continue;
				}
				else if (WSAENETDOWN == nErrCode ||
					WSAETIMEDOUT == nErrCode ||
					WSAECONNRESET == nErrCode)//�ͻ��˹ر�������
				{
					//�뿪�ٽ���
					LeaveCriticalSection(&pClient->m_cs);
					pClient->m_bConning = FALSE;	//���ӶϿ�
					pClient->m_bExit = TRUE;		//�߳��˳�
					break;
				}
				else {
					//�뿪�ٽ���
					LeaveCriticalSection(&pClient->m_cs);
					pClient->m_bConning = FALSE;	//���ӶϿ�
					pClient->m_bExit = TRUE;		//�߳��˳�
					break;
				}
			}
			//�ɹ���������
			//�뿪�ٽ���
			LeaveCriticalSection(&pClient->m_cs);
			//�����¼�Ϊ���ź�״̬
			ResetEvent(&pClient->m_hEvent);
		}

	}

	return 0;
}
/*
*  ������ʽ,�������
*/
void CClient::HandleData(const char* pExpr)
{
	printf("%s\n", pExpr);
	memset(m_data.buf, 0, MAX_NUM_BUF);//���m_data

	if (BYEBYE == ((phdr)pExpr)->type)
	{
		EnterCriticalSection(&m_cs);
		phdr pHeaderSend = (phdr)m_data.buf;				//���͵�����		
		pHeaderSend->type = BYEBYE;							//��������
		pHeaderSend->len = HEADERLEN + strlen("OK");		//���ݰ�����
		memcpy(m_data.buf + HEADERLEN, "OK", strlen("OK"));	//�������ݵ�m_data"
		LeaveCriticalSection(&m_cs);
		SetEvent(m_hEvent);	//֪ͨ���������߳�

	}
	else if (EXIT == ((phdr)pExpr)->type) {
		EnterCriticalSection(&m_cs);
		phdr pHeaderSend = (phdr)m_data.buf;				//���͵�����		
		pHeaderSend->type = EXIT;							//��������
		int len = strlen("Exit");
		pHeaderSend->len = HEADERLEN + len;		//���ݰ�����
		memcpy(m_data.buf + HEADERLEN, "Exit", len);	//�������ݵ�m_data"
		LeaveCriticalSection(&m_cs);		
		SetEvent(m_hEvent);	//֪ͨ���������߳�

	}	
	else if (TIME == ((phdr)pExpr)->type) {
		time_t timep;
		//struct tm *p;
		time(&timep);
		//printf("%lld\n", timep);
		EnterCriticalSection(&m_cs);
		phdr pHeaderSend = (phdr)m_data.buf;
		pHeaderSend->type = TIME;
		char temp[MAX_NUM_BUF];
		sprintf(temp, "%lld", timep);
		int len = strlen(temp);
		pHeaderSend->len = HEADERLEN + len;
		memcpy(m_data.buf + HEADERLEN, temp, len);	//�������ݵ�m_data"
		LeaveCriticalSection(&m_cs);
		SetEvent(m_hEvent);	//֪ͨ���������߳�

	}
	else if (NAME == ((phdr)pExpr)->type) {
		char temp[MAX_NUM_BUF];
		DWORD dwSize = sizeof(temp);
		GetComputerNameEx((COMPUTER_NAME_FORMAT)0,(LPWSTR)temp, &dwSize);
		int len = strlen(temp);
		EnterCriticalSection(&m_cs);
		phdr pHeaderSend = (phdr)m_data.buf;
		pHeaderSend->type = NAME;
		pHeaderSend->len = HEADERLEN + len;
		memcpy(m_data.buf + HEADERLEN, temp, len);
		LeaveCriticalSection(&m_cs);
		SetEvent(m_hEvent);
	}
	else if (LIST == ((phdr)pExpr)->type){
		std::vector<PACKAGE> pac;
		int count = 0;
		PACKAGE temp;

		for (auto iter: this->Super->clientlist) {
			u_long ulIPNum = iter->m_addr.sin_addr.S_un.S_addr;
			int iPortNum = iter->m_addr.sin_port;
			count++;
			temp.head.type = LIST;
			
			char caIP[4];
			char cID;
			char caPort[2];
			char cLeft = 0;
			char* buffer;
			int a[4];
			buffer = inet_ntoa(iter->m_addr.sin_addr);
			sscanf(buffer, "%d.%d.%d.%d", &a[0], &a[1], &a[2], &a[3]);
			memset(temp.data.buf, 0, sizeof(temp.data.buf));
			for(int i=0;i<4;i++)
				caIP[i] = (char)a[i];
			caPort[0] = iPortNum % 256;
			iPortNum /= 256;
			caPort[1] = iPortNum;
			cID = iter->m_iID;
			if (count == 1) {
				temp.head.type = LIST;

				memcpy(temp.data.buf + 2, &cID,1);
				memcpy(temp.data.buf + 3, caIP, 4);
				memcpy(temp.data.buf + 7, caPort, 2);
				memcpy(temp.data.buf + 9, &cLeft, 1);
			}
			else {
				memcpy(temp.data.buf + 2 + 10*count, caIP, 4);
				memcpy(temp.data.buf + 6 + 10 * count, caPort, 2);
				memcpy(temp.data.buf + 8 + 10 * count, &cLeft, 1);
			}
			if (count == 4) {
				count = 0;
				pac.push_back(temp);
			}
		}
		for (auto iter : pac) {
			EnterCriticalSection(&m_cs);
			phdr pHeaderSend = (phdr)m_data.buf;
			pHeaderSend->type = LIST;
			char temp[MAX_NUM_BUF - HEADERLEN];
			sprintf(temp, "%s", iter.data);
			int len = iter.head.len;
			pHeaderSend->len = HEADERLEN + len;
			memcpy(m_data.buf + HEADERLEN, temp, len);	//�������ݵ�m_data"
			LeaveCriticalSection(&m_cs);
			SetEvent(m_hEvent);	//֪ͨ���������߳�
		}
	}
	
	
	else{//�������ʽ

		//int nFirNum;		//��һ������
		//int nSecNum;		//�ڶ�������
		//char cOper;			//���������
		//int nResult;		//������
		//					//��ʽ����������
		//sscanf(pExpr + HEADERLEN, "%d%c%d", &nFirNum, &cOper, &nSecNum);

		////����
		//switch (cOper)
		//{
		//case '+'://��
		//{
		//	nResult = nFirNum + nSecNum;
		//	break;
		//}
		//case '-'://��
		//{
		//	nResult = nFirNum - nSecNum;
		//	break;
		//}
		//case '*'://��
		//{
		//	nResult = nFirNum * nSecNum;
		//	break;
		//}
		//case '/'://��
		//{
		//	if (ZERO == nSecNum)//��Ч������
		//	{
		//		nResult = INVALID_NUM;
		//	}
		//	else
		//	{
		//		nResult = nFirNum / nSecNum;
		//	}
		//	break;
		//}
		//default:
		//	nResult = INVALID_OPERATOR;//��Ч������
		//	break;
		//}

		////���������ʽ�ͼ���Ľ��д���ַ�������
		//char temp[MAX_NUM_BUF];
		//char cEqu = '=';
		//sprintf(temp, "%d%c%d%c%d", nFirNum, cOper, nSecNum, cEqu, nResult);

		////�������
		//EnterCriticalSection(&m_cs);
		//phdr pHeaderSend = (phdr)m_data.buf;				//���͵�����		
		//pHeaderSend->type = EXPRESSION;						//��������Ϊ�������ʽ
		//pHeaderSend->len = HEADERLEN + strlen(temp);		//���ݰ��ĳ���
		//memcpy(m_data.buf + HEADERLEN, temp, strlen(temp));	//�������ݵ�m_data
		//LeaveCriticalSection(&m_cs);

	}
}

BOOL CClient::IntToChar(int total, int index, char * des)
{
	des[0] = (char)total;
	des[1] = (char)index;
	return TRUE;
}

std::vector<PACKAGE> CClient::DataConvert(char * str, int type)
{
	std::vector<PACKAGE> Result;
	int TotalLength = strlen(str);
	int BufferLength = MAX_NUM_BUF - HEADERLEN;
	int PackageNum = TotalLength / BufferLength + 1;
	int Left = TotalLength - (PackageNum - 1) *BufferLength;
	for (int i = 0; i < PackageNum; i++) {
		PACKAGE temp;
		temp.head.type = type;
		temp.head.len = PackageNum == i + 1 ? Left : BufferLength;
		IntToChar(TotalLength, i + 1, temp.data.buf);
		strncpy(temp.data.buf + 2, str + i*BufferLength, temp.head.len - 2);
		Result.push_back(temp);
	}
	return Result;
}