#include"superServer.h"

int main(int argc, char* argv[])
{
	superServer s;
	//初始化服务器	
	if (!s.InitSever())
	{
		s.ExitServer();
		return SERVER_SETUP_FAIL;
	}

	//启动服务
	if (!s.StartService())
	{
		s.ShowServerStartMsg(FALSE);
		s.ExitServer();
		return SERVER_SETUP_FAIL;
	}

	//停止服务
	s.StopService();

	//服务器退出
	s.ExitServer();

	return 0;
}