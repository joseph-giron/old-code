// SimpleHook.cpp : Defines the entry point for the DLL application.
//

#include "windows.h"
#include "stdafx.h"
DWORD* lpAddr;
PROC OldProc;
BOOL __stdcall MyTerminateProcess(HANDLE hProcess,UINT uExitCode)
{
	MessageBox(NULL,"û�����������˰� ","API HOOK",0);
	return 0;
}


int ApiHook(char *DllName,//DLL�ļ���
     char *ProcName,//ҪHOOK�ĺ�������
     PROC NewFunAddr//���ǹ���ĺ�����ַ
     )
{
	HMODULE hMoudle=LoadLibrary(DllName);
	HWND hParent;
	OldProc=GetProcAddress(hMoudle,ProcName);
	//����ԭ������ַ�����ǵĺ�����ƫ��
	DWORD offsets=(int)MyTerminateProcess-(int)OldProc-5;
	//д��ԭ������ڴ�jmp MyTerminateProcessָ��
	//���ڲ����޸ı�����ָ���һ��ע�ӵ�����Ϊ�򸸽��̷������ݣ��ø�����ʵ��
/*	_asm
	{
		push ebx
		mov ebx,OldProc
		mov byte ptr [ebx],0xE9
		inc ebx
		push offsets
		pop [ebx]
		pop ebx
	}  */
	//���Ҹ����̴��ھ��
	hParent=FindWindow(NULL,"Inline Hook");
	if(!hParent)
		MessageBox(NULL,"Not find the parent window","Error",MB_OK);
	//�Զ�����Ϣ�Ĳ���1ΪҪHook����ڵ�ַ������2ΪҪ��ת��ƫ�Ƶ�ַ
	Sleep(5000);
	SendMessage(hParent,WM_USER+1,(unsigned int)OldProc,(unsigned int)offsets);
	return 0;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
       DWORD ul_reason_for_call, 
       LPVOID lpReserved
       )
{	if(ul_reason_for_call==DLL_PROCESS_ATTACH)
		ApiHook("ntdll.dll","ZwTerminateProcess",(PROC)MyTerminateProcess);
    return TRUE; 
}