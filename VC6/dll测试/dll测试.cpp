// dll����.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "stdio.h"

typedef (CALLBACK * lpFunAddr)(char *,char *);

int main(int argc, char* argv[])
{
	
	HINSTANCE h=LoadLibrary("����.dll");
	lpFunAddr pCal=(lpFunAddr)GetProcAddress(h,"calculate");
	char str[1024];
	char result[15];
	printf("��������ʽ��");
	scanf("%s",str);
	pCal(str,result);
	printf("������Ϊ��%s",result);
	getchar();
	getchar();
	return 0;
}