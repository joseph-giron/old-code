// tcpClient.cpp : Defines the entry point for the console application.
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
	SOCKET s;
	sockaddr_in remoteAddr;

	WSAStartup(MAKEWORD(2, 2), &wd);			//��ʼ��WSA��
	//�����׽���
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(INVALID_SOCKET  == s)
	{
		printf("Create socket error\n");
		goto ret;
	}
	//���ӷ����
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(12121);
	remoteAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if(SOCKET_ERROR == connect(s, (const sockaddr*)&remoteAddr, sizeof(remoteAddr)))
	{
		printf("Connet Failue!\n");
		goto ret;
	}
	printf("���ӳɹ�!\n");
	//�����˷�������
	strcpy(b, "Hello sever");
	send(s, b, 32, 0);
	//�ӷ���˽������� 
	recv(s, b, 32, 0);
	//��ӡ���� 
	printf("�յ���Ϣ��\n");
	printf(b);
	//�˳�
ret:
	getchar();
	return 0;
}
