// ��ѯ������ַ.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "windows.h"
int main(int argc, char* argv[])
{
	char Name[20];
	char File[20];
	DWORD addr;
	printf("Dll�ļ�����");
	scanf("%s",File);
	printf("\nAPI��������");
	scanf("%s",Name);
	printf("\n������ַ�ǣ�%0xH",
			GetProcAddress(LoadLibrary(File),Name));
	getchar();
	getchar();
	return 0;
}
