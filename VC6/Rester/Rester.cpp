// Rester.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <windows.h>

struct TIME
{
	short hour;
	short minu;
};

//����������
class CONFIG
{
public:
	int		WorkTime;
	int		RestTime;
	char	Pswd[32];
	TIME	StartTime;
	TIME	EndTime;

	int ReadConfig( CONFIG *Config );//��ȡ����
	int SaveConfig( CONFIG *Config );//�洢����

private:
	HKEY hConfig;//��������ע����
}Config;

//��ȡ����
int CONFIG::ReadConfig( CONFIG *Config )
{
	char str[256] = {0};
	DWORD dwDisposition = 0; 
	DWORD dwCount = 0;
	LONG lRet = RegCreateKeyEx( HKEY_LOCAL_MACHINE, "SoftWare\\rester",
		0, 0, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &hConfig, &dwDisposition );
	if( lRet != ERROR_SUCCESS )//��ע����ʧ��
	{
		wsprintf( str, "RegCreateKey failue, error code is %d", lRet );
		OutputDebugString( str );
		return 0;
	}

	if( REG_CREATED_NEW_KEY == dwDisposition )//�½���һ��ע����������һ������
	{
		return 0;
	}

	lRet = RegQueryValueEx( hConfig, NULL, 0, NULL, (LPBYTE)Config, &dwCount );//��ȡ��ֵ�е���������
	if( lRet != ERROR_SUCCESS )//��ȡʧ��
	{
		wsprintf( str, "Query data failue, error code is %d", lRet );
		OutputDebugString( str );
		return 0;
	}

	return 1;
}

//�洢����
int CONFIG::SaveConfig( CONFIG *Config )
{
	DWORD dwCount = sizeof( CONFIG );
	LONG lRet = 0;
	char str[256] = {0};

	if( hConfig != NULL )
	{
		lRet = RegSetValueEx( hConfig, NULL, 0, REG_SZ, (LPBYTE)Config, dwCount );
		if( lRet != ERROR_SUCCESS )//����ʧ��
		{
			wsprintf( str, "Query data failue, error code is %d", lRet );
			OutputDebugString( str );
			return 0;
		}
		return 1;
	}
	
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	
	return 0;
}



