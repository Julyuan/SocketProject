#include <stdlib.h>
#include "resource.h"
#include <winsock2.h>
#include <richedit.h>

//���û������Ĵ�С
#define BUFSIZE (1024*64)

HINSTANCE hInst;
HWND hWnd, hOWnd, hIWnd, hIPEdit, hPortEdit, hConnBtn, hListenBtn;
//��������������������ʾ�ı�����������socket I/O������
TCHAR buf[BUFSIZE];
//cur:�������п������ĵ�ǰλ��
//sendpos:����������Ҫ���͵����ݵĳ�ʼλ��
int cur = 0, sendpos = 0;
//����ָʾ��ǰ����״̬
BOOL conned = FALSE, listening = FALSE;

void Input();
void ClearBuf();
void PrintLastError();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#include "net.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;
	//ע�ᴰ����
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
	//��������������
	hWnd = CreateWindow((LPCWSTR)"CHAT", (LPCWSTR)"Chat",
		WS_OVERLAPPEDWINDOW &~WS_MAXIMIZEBOX &~WS_THICKFRAME, 200, 200, 505, 410, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		return FALSE;
	}
	//��ʾ����
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	//��ʼ��socket
	if (!NetInit()) return 0;
	MSG msg;
	//���ؼ��ټ���
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ACCEL));
	//��Ϣѭ��
	while (GetMessage(&msg, NULL, 0, 0))
	{
		//check for accelerator
		if (!TranslateAccelerator(hWnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	//�ر�socket
	NetEnd();
	return msg.wParam;
}

//��������Ϣ������
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HFONT hfont;
	//���ݲ�ͬ����Ϣ����ͬ�Ĵ���
	switch (message)
	{
	case WM_SOCKET:
		//�����socket��Ϣ�͵���ProcessSocketMsg���д���
		ProcessSocketMsg(wParam, lParam);
		break;
	case WM_CREATE:
		//����������ڴ�������Ϣ�ͳ�ʼ�����ؼ�
		LoadLibrary((LPCWSTR)"RICHED20.DLL");
		//����������ݵĿؼ�
		hOWnd = CreateWindowEx(WS_EX_CLIENTEDGE, RICHEDIT_CLASS, NULL,
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT |
			ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
			10, 10, 350, 250, hWnd, NULL, hInst, NULL);
		//�����������ݵĿؼ�
		hIWnd = CreateWindowEx(WS_EX_CLIENTEDGE, RICHEDIT_CLASS, NULL,
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT |
			ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
			10, 270, 350, 100, hWnd, NULL, hInst, NULL);
		//����IP�Ŀؼ�
		hIPEdit = CreateWindowEx(WS_EX_CLIENTEDGE, (LPCWSTR)"EDIT", (LPCWSTR)"127.0.0.1",
			WS_CHILD | WS_VISIBLE | ES_LEFT |
			ES_AUTOHSCROLL,
			370, 10, 120, 25, hWnd, (HMENU)ID_IPEDIT,
			hInst, NULL);
		//����˿ںŵĿؼ�
		hPortEdit = CreateWindowEx(WS_EX_CLIENTEDGE, (LPCWSTR)"EDIT", (LPCWSTR)"8888",
			WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER,
			370, 45, 50, 25, hWnd, (HMENU)ID_PORTEDIT,
			hInst, NULL);
		//���Ӱ�ť
		hConnBtn = CreateWindowEx(0, (LPCWSTR)"BUTTON", (LPCWSTR)"Connect",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON |
			BS_DEFPUSHBUTTON, 370, 80, 100, 25, hWnd,
			(HMENU)ID_CONNBTN, hInst, NULL);
		//������ť
		hListenBtn = CreateWindowEx(0, (LPCWSTR)"BUTTON", (LPCWSTR)"Listen",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON |
			BS_DEFPUSHBUTTON, 370, 115, 100, 25, hWnd,
			(HMENU)ID_LISTENBTN, hInst, NULL);
		//������ʾ�������
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
			//���û�����ctrl+enterʱ
		case IDM_INPUT: Input(); break;
			//���û��������Ӱ�ťʱ
		case ID_CONNBTN:
			if (conned)
				//��������Ѿ������������ť�ĺ����ǹر�����
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
				//�û�����������
			{
				char ipstr[1024], portstr[16];
				SendMessage(hIPEdit, WM_GETTEXT, 1024, (long)ipstr);
				SendMessage(hPortEdit, WM_GETTEXT, 16, (long)portstr);
				//����
				if (NetConn(ipstr, atoi(portstr)))
				{
					EnableWindow(hConnBtn, FALSE);
					EnableWindow(hIPEdit, FALSE);
					EnableWindow(hPortEdit, FALSE);
					EnableWindow(hListenBtn, FALSE);
				}
			}
			break;
			//���û�����������ť
		case ID_LISTENBTN:
			if (listening)
				//����Ѿ��������������ť�ĺ�����ȡ������
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
				//����
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
		//�ػ洰��
		hdc = BeginPaint(hWnd, &ps);
		RECT rt;
		GetClientRect(hWnd, &rt);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		//�˳�����
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//�������������ݣ����������ı���ʾ�ؼ�������
void ClearBuf()
{
	SendMessage(hOWnd, WM_SETTEXT, 0, NULL);
	memset(buf, 0, BUFSIZE);
	cur = 0;
}

//�����û��������ݣ���������ʾ������ı���ʾ�ؼ���
inline void Input()
{
	sendpos = cur;
	if (cur + SendMessage(hIWnd, WM_GETTEXTLENGTH, 0, 0) + 1 >= BUFSIZE) ClearBuf();
	cur += SendMessage(hIWnd, WM_GETTEXT, BUFSIZE - cur - 1, long(buf + cur));
	//������ı���β����һ�����з�
	buf[cur++] = '\n';
	if (NetSend())
	{
		SendMessage(hOWnd, WM_SETTEXT, 0, (long)buf);
		SendMessage(hIWnd, WM_SETTEXT, 0, NULL);
	}
}

//���������Ϣ
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
