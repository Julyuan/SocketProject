#include"superServer.h"

int main(int argc, char* argv[])
{
	superServer s;
	//��ʼ��������	
	if (!s.InitSever())
	{
		s.ExitServer();
		return SERVER_SETUP_FAIL;
	}

	//��������
	if (!s.StartService())
	{
		s.ShowServerStartMsg(FALSE);
		s.ExitServer();
		return SERVER_SETUP_FAIL;
	}

	//ֹͣ����
	s.StopService();

	//�������˳�
	s.ExitServer();

	return 0;
}