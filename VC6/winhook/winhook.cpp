// winhook.cpp : Defines the entry point for the DLL application.
//��ģ��ʵ���������ܣ���װ���ӣ�ж�ع��ӣ���⹳����ǰж�ع���

#include <windows.h>
#include "stdafx.h"
#include "winhook.h"

#pragma comment(linker, "/SECTION:Shared,RWS")		//����dll���ݶ�
HWND hShell = 0;

//���ӻص�����
int HOOKPROC HookProc( int dwCode, int wParam, int lParam )
{
	return 0;		//���ش���Ϣ����ֹ�䱻���͵�Ŀ�괰��
}

//��װ������깳��
WINHOOK_API int InstallHook()
{
	return 1;
}

//ж�ؼ�����깳��
WINHOOK_API int UninstallHook()
{
	return 1;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


// This is an example of an exported variable
WINHOOK_API int nWinhook=0;

// This is an example of an exported function.
WINHOOK_API int fnWinhook(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see winhook.h for the class definition
CWinhook::CWinhook()
{ 
	return; 
}

