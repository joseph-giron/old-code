// SimpleHook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
DWORD* lpAddr;
PROC OldProc;
BOOL __stdcall MyTerminateProcess(HANDLE hProcess,UINT uExitCode)
{
MessageBox(NULL,"û�����������˰� ","API HOOK",0);
return 0;
}

int ApiHook(char *DllName,//DLL�ļ���
     PROC OldFunAddr,//ҪHOOK�ĺ�����ַ
     PROC NewFunAddr//���ǹ���ĺ�����ַ
     )
{
//�õ���������ģ�����ַ
HMODULE lpBase = GetModuleHandle(NULL); //���ص�ǰexe�Ļ���ַ
IMAGE_DOS_HEADER *dosHeader; //����dosͷ�ṹ
IMAGE_NT_HEADERS *ntHeader; //PE�ṹ
IMAGE_IMPORT_BY_NAME *ImportName;//������� �������ֺ���Ŵ�ŵĽṹ
//��λ��DOSͷ
dosHeader=(IMAGE_DOS_HEADER*)lpBase; //��õ�ǰ���̻���ַ��dosͷ
//��λ��PEͷ
ntHeader=(IMAGE_NT_HEADERS32*)((BYTE*)lpBase+dosHeader->e_lfanew);// e_lfanew��ŵ���peͷ��ƫ��~ת���ṹ����
//���pe�ļ��Ƿ���Ч
if(ntHeader->Signature !=IMAGE_NT_SIGNATURE)MessageBox(NULL,"������Ч��pe�ļ�","����",MB_OK);
//��λ�������
IMAGE_IMPORT_DESCRIPTOR *pImportDesc=(IMAGE_IMPORT_DESCRIPTOR*)((BYTE*)lpBase+ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
//����ַ+��������ʼRVA

// ���� �õ� iat�ṹ��RVA,������dll�ļ�����Ҫhook�ĺ���,���Ǹ�DLL,��++���¸�dll��PE��
while(pImportDesc->FirstThunk)
{
   //�õ�DLL�ļ���
   char* pszDllName = (char*)((BYTE*)lpBase + pImportDesc->Name);
   //�Ƚϵõ���DLL�ļ����Ƿ��ҪHOOK�������ڵ�DLL��ͬ
   if(lstrcmpiA(pszDllName, DllName) == 0)
   {
    break;
   }
   pImportDesc++;
}
//��λ��FirstThunk����ָ���IMAGE_THUNK_DATA����ʱ����ṹ�Ѿ��Ǻ�����ڵ��ַ��
IMAGE_THUNK_DATA* pThunk = (IMAGE_THUNK_DATA*)
   ((BYTE*)lpBase + pImportDesc->FirstThunk); //�õ����Ҫhook�ĺ�����dll��IAT
//�����ⲿ��IAT��
while(pThunk->u1.Function)
{
   lpAddr = (DWORD*)&(pThunk->u1.Function);
   //�ȽϺ�����ַ�Ƿ���ͬ
   if(*lpAddr == (DWORD)OldFunAddr)
   { 
    DWORD dwOldProtect;
    //�޸��ڴ��������
    VirtualProtect(lpAddr, sizeof(DWORD), PAGE_READWRITE, &dwOldProtect);
    //API��������ڵ��ַ�ĳ����ǹ���ĺ����ĵ�ַ
    WriteProcessMemory(GetCurrentProcess(),lpAddr, &NewFunAddr, sizeof(DWORD), NULL);
   }
   pThunk++;
}
return 0;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
       DWORD ul_reason_for_call, 
       LPVOID lpReserved
       )
{
switch (ul_reason_for_call)
{
case DLL_PROCESS_ATTACH:
   //�õ�TerminateProcess������ַ
   OldProc = (PROC)TerminateProcess;
   //��λ���޸�IAT��
   ApiHook("kernel32.dll",OldProc,(PROC)MyTerminateProcess);
   break;
case DLL_PROCESS_DETACH:
   //�ָ�IAT����API��������ڵ��ַ
   WriteProcessMemory(GetCurrentProcess(),lpAddr, &OldProc, sizeof(DWORD), NULL);
   break; 
}
    return TRUE; 
}