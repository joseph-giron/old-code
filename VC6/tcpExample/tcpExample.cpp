// tcpExample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "winsock2.h"

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[])
{
	WSADATA wd;
	char b[128] = {0};
	int l, nAddrLen;
	SOCKET s, n;
	sockaddr_in localAddr, remoteAddr;

	WSAStartup(MAKEWORD(2, 2), &wd);			//��ʼ��WSA��
	//�����׽���
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(INVALID_SOCKET  == s)
	{
		printf("Create socket error\n");
		goto ret;
	}
	//�󶨶˿�
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(12121);
	localAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if(SOCKET_ERROR == bind(s, (const sockaddr*)&localAddr, sizeof(localAddr)))
	{
		printf("Bind port error\n");
		goto ret;
	}
	//�����׽���Ϊ����ģʽ
	if(SOCKET_ERROR == listen(s, 5))
	{
		printf("Listen error\n");
		goto ret;
	}
	//�ȴ��ͻ������� 
	nAddrLen = sizeof(remoteAddr);
	n = accept(s, (struct sockaddr*)&remoteAddr, &nAddrLen);
	printf("�յ�һ�����ӣ�\n");
	//���տͻ��˷���������
	l = recv(n, b, 32, 0);
	//��ӡ����
	printf("�յ���Ϣ��\n");
	printf(b);
	//��ͻ��˷�������
	strcpy(b, "Hello client!");
	send(n, b, 32, 0);
	//�˳�
ret:
	getchar();
	return 0;
}
