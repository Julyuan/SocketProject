#include <stdlib.h>
#include "resource.h"
#include <winsock2.h>
#include <richedit.h>

//设置缓冲区的大小
#define BUFSIZE (1024*64)

HINSTANCE hInst;
HWND hWnd, hOWnd, hIWnd, hIPEdit, hPortEdit, hConnBtn, hListenBtn;
//这个缓冲区既用来存放显示文本，又用来作socket I/O缓冲区
TCHAR buf[BUFSIZE];
//cur:缓冲区中空闲区的当前位置
//sendpos:缓冲区中需要发送的数据的初始位置
int cur = 0, sendpos = 0;
//用来指示当前程序状态
BOOL conned = FALSE, listening = FALSE;

void Input();
void ClearBuf();
void PrintLastError();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#include "net.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;
	//注册窗口类
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = (LPCWSTR)"CHAT";
	wcex.hIconSm = NULL;
	RegisterClassEx(&wcex);
	//创建程序主窗口
	hWnd = CreateWindow((LPCWSTR)"CHAT", (LPCWSTR)"Chat",
		WS_OVERLAPPEDWINDOW &~WS_MAXIMIZEBOX &~WS_THICKFRAME, 200, 200, 505, 410, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		return FALSE;
	}
	//显示窗口
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	//初始化socket
	if (!NetInit()) return 0;
	MSG msg;
	//加载加速键表
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ACCEL));
	//消息循环
	while (GetMessage(&msg, NULL, 0, 0))
	{
		//check for accelerator
		if (!TranslateAccelerator(hWnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	//关闭socket
	NetEnd();
	return msg.wParam;
}

//主窗口消息处理函数
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HFONT hfont;
	//根据不同的消息做不同的处理
	switch (message)
	{
	case WM_SOCKET:
		//如果是socket消息就调用ProcessSocketMsg进行处理
		ProcessSocketMsg(wParam, lParam);
		break;
	case WM_CREATE:
		//如果是主窗口创建的消息就初始化各控件
		LoadLibrary((LPCWSTR)"RICHED20.DLL");
		//输出聊天内容的控件
		hOWnd = CreateWindowEx(WS_EX_CLIENTEDGE, RICHEDIT_CLASS, NULL,
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT |
			ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
			10, 10, 350, 250, hWnd, NULL, hInst, NULL);
		//输入聊天内容的控件
		hIWnd = CreateWindowEx(WS_EX_CLIENTEDGE, RICHEDIT_CLASS, NULL,
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT |
			ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
			10, 270, 350, 100, hWnd, NULL, hInst, NULL);
		//输入IP的控件
		hIPEdit = CreateWindowEx(WS_EX_CLIENTEDGE, (LPCWSTR)"EDIT", (LPCWSTR)"127.0.0.1",
			WS_CHILD | WS_VISIBLE | ES_LEFT |
			ES_AUTOHSCROLL,
			370, 10, 120, 25, hWnd, (HMENU)ID_IPEDIT,
			hInst, NULL);
		//输入端口号的控件
		hPortEdit = CreateWindowEx(WS_EX_CLIENTEDGE, (LPCWSTR)"EDIT", (LPCWSTR)"8888",
			WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER,
			370, 45, 50, 25, hWnd, (HMENU)ID_PORTEDIT,
			hInst, NULL);
		//连接按钮
		hConnBtn = CreateWindowEx(0, (LPCWSTR)"BUTTON", (LPCWSTR)"Connect",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON |
			BS_DEFPUSHBUTTON, 370, 80, 100, 25, hWnd,
			(HMENU)ID_CONNBTN, hInst, NULL);
		//侦听按钮
		hListenBtn = CreateWindowEx(0, (LPCWSTR)"BUTTON", (LPCWSTR)"Listen",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON |
			BS_DEFPUSHBUTTON, 370, 115, 100, 25, hWnd,
			(HMENU)ID_LISTENBTN, hInst, NULL);
		//设置显示字体外观
		LOGFONT lf;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfEscapement = 0;
		strcpy((char*)lf.lfFaceName, "Verdana");
		lf.lfHeight = 15;
		lf.lfItalic = FALSE;
		lf.lfOrientation = 0;
		lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf.lfPitchAndFamily = FF_MODERN | DEFAULT_PITCH;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfStrikeOut = FALSE;
		lf.lfUnderline = FALSE;
		lf.lfWeight = FW_BOLD;
		lf.lfWidth = 7;
		hfont = CreateFontIndirect(&lf);
		SendMessage(hOWnd, WM_SETFONT, (WPARAM)hfont, MAKELPARAM(FALSE, 0));
		SendMessage(hIWnd, WM_SETFONT, (WPARAM)hfont, MAKELPARAM(FALSE, 0));
		SendMessage(hIPEdit, WM_SETFONT,
			(WPARAM)hfont, MAKELPARAM(FALSE, 0));
		SendMessage(hPortEdit, WM_SETFONT, (WPARAM)hfont,
			MAKELPARAM(FALSE, 0));
		SendMessage(hConnBtn, WM_SETFONT, (WPARAM)hfont,
			MAKELPARAM(FALSE, 0));
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
			//当用户按下ctrl+enter时
		case IDM_INPUT: Input(); break;
			//当用户按下连接按钮时
		case ID_CONNBTN:
			if (conned)
				//如果连接已经建立，这个按钮的含意是关闭连接
			{
				NetDisconn();
				SendMessage(hConnBtn, WM_SETTEXT, 0, (long)"Connect");
				EnableWindow(hIPEdit, TRUE);
				EnableWindow(hPortEdit, TRUE);
				EnableWindow(hListenBtn, TRUE);
				SendMessage(hIWnd, EM_SETREADONLY, TRUE, 0);
				conned = FALSE;
			}
			else
				//用户请求建立连接
			{
				char ipstr[1024], portstr[16];
				SendMessage(hIPEdit, WM_GETTEXT, 1024, (long)ipstr);
				SendMessage(hPortEdit, WM_GETTEXT, 16, (long)portstr);
				//连接
				if (NetConn(ipstr, atoi(portstr)))
				{
					EnableWindow(hConnBtn, FALSE);
					EnableWindow(hIPEdit, FALSE);
					EnableWindow(hPortEdit, FALSE);
					EnableWindow(hListenBtn, FALSE);
				}
			}
			break;
			//当用户按下侦听按钮
		case ID_LISTENBTN:
			if (listening)
				//如果已经在侦听，这个按钮的含意是取消侦听
			{
				closesocket(tcps_listen);
				EnableWindow(hIPEdit, TRUE);
				EnableWindow(hPortEdit, TRUE);
				EnableWindow(hConnBtn, TRUE);
				SendMessage(hListenBtn, WM_SETTEXT, 0, (long)"Listen");
				listening = FALSE;
			}
			else
			{
				char portstr[16];
				SendMessage(hPortEdit, WM_GETTEXT, 16, (long)portstr);
				//侦听
				if (NetListen(atoi(portstr)))
				{
					EnableWindow(hConnBtn, FALSE);
					EnableWindow(hIPEdit, FALSE);
					EnableWindow(hPortEdit, FALSE);
				}
			}
			break;
		}
		break;
	case WM_PAINT:
		//重绘窗口
		hdc = BeginPaint(hWnd, &ps);
		RECT rt;
		GetClientRect(hWnd, &rt);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		//退出程序
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//消除缓冲区内容，并清空输出文本显示控件的内容
void ClearBuf()
{
	SendMessage(hOWnd, WM_SETTEXT, 0, NULL);
	memset(buf, 0, BUFSIZE);
	cur = 0;
}

//发送用户输入内容，并将它显示在输出文本显示控件中
inline void Input()
{
	sendpos = cur;
	if (cur + SendMessage(hIWnd, WM_GETTEXTLENGTH, 0, 0) + 1 >= BUFSIZE) ClearBuf();
	cur += SendMessage(hIWnd, WM_GETTEXT, BUFSIZE - cur - 1, long(buf + cur));
	//在输出文本结尾增加一个换行符
	buf[cur++] = '\n';
	if (NetSend())
	{
		SendMessage(hOWnd, WM_SETTEXT, 0, (long)buf);
		SendMessage(hIWnd, WM_SETTEXT, 0, NULL);
	}
}

//输出错误信息
void PrintLastError()
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)"Error", MB_OK | MB_ICONINFORMATION);
	LocalFree(lpMsgBuf);
}
