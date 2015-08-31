///////////////////////////////////
//
//////////////////////////////////
#include "stdafx.h"
#include "syskey.h"

//��װ����  ����1����������   �������������ļ�·��
bool InstallService(char *szDriver,char *szFile,bool bStart)
{
   SC_HANDLE sh=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
   if(!sh)return false;
   SC_HANDLE rh=CreateService(sh,szDriver,szDriver,SERVICE_ALL_ACCESS,
                                                   SERVICE_KERNEL_DRIVER,
                                                   SERVICE_DEMAND_START,
                                                   SERVICE_ERROR_NORMAL,
                                                   szFile,NULL,NULL,NULL,
                                                   NULL,NULL);
   if(!rh)
   {
     if(GetLastError()==ERROR_SERVICE_EXISTS)
     {
        //�����Ѱ�װ�����
        rh=OpenService(sh,szDriver,SERVICE_ALL_ACCESS);
        if(!rh)
        {
          CloseServiceHandle(sh);
          return false;
        }
     }
     else
     {
        CloseServiceHandle(sh);
        return false;
     }
   }
   if(bStart)                //�Ƿ�Ҫ��������
     StartService(rh,0,NULL);
   CloseServiceHandle(sh);
   CloseServiceHandle(rh);
   return true;
}

MKeyBoard::MKeyBoard()
{
	char syspath[] = "E:\\Program Files\\Borland\\CBuilder6\\Projects\\��д���̶˿�\\Keyboard.sys";
	if(!InstallService("mykey", syspath, 1))
	{
		MessageBox(0, "Error", "Install Service failue!", MB_OK);
		ExitProcess(0);
	}

	hDevice = CreateFile("\\\\.\\wriKeyPort", GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if(hDevice == INVALID_HANDLE_VALUE)
	{
		MessageBox(0, "Error", "Open device failue!", MB_OK);
		ExitProcess(0);
	}
}
//������ɨ����
void MKeyBoard::up(char c)
{
	c &= 0x7f;
	DWORD n;
	WriteFile(hDevice, &c, 1, &n, 0);
	return;
}
//������ɨ����
void MKeyBoard::down(char c)
{
	c &= 0x80;
	DWORD n;
	WriteFile(hDevice, &c, 1, &n, 0);
	return;
}
//������ɨ����
void MKeyBoard::click(char c)
{
	down(c);
	Sleep(200);
	click(c);
	return;
}










