#include<stdlib.h>
//#include"resource.h"
#include<WinSock2.h>
#include<Richedit.h>

#define BUFSIZE (1024*64)

HINSTANCE hInst;
HWND hWnd, hOWnd, hIWnd, hIPEdit, hPortEdit, hConnBtn, hListenBtn;

// �����ʾ�ı�����������socket I/O������
TCHAR buf[BUFSIZE];

//cur:�������п������ĵ�ǰλ��
//sendpos:����������Ҫ���͵����ݵĳ�ʼλ��
int cur = 0, sendpos = 0;

// ����ָʾ��ǰ����״̬
BOOL conned = FALSE, listening = FALSE;

void Input();
void ClearBuf();
void PrintLastError();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM IParam);

#include"net.h"




