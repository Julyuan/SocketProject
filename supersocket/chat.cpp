#include<stdlib.h>
//#include"resource.h"
#include<WinSock2.h>
#include<Richedit.h>

#define BUFSIZE (1024*64)

HINSTANCE hInst;
HWND hWnd, hOWnd, hIWnd, hIPEdit, hPortEdit, hConnBtn, hListenBtn;

// 存放显示文本，又用来作socket I/O缓冲区
TCHAR buf[BUFSIZE];

//cur:缓冲区中空闲区的当前位置
//sendpos:缓冲区中需要发送的数据的初始位置
int cur = 0, sendpos = 0;

// 用来指示当前程序状态
BOOL conned = FALSE, listening = FALSE;

void Input();
void ClearBuf();
void PrintLastError();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM IParam);

#include"net.h"




