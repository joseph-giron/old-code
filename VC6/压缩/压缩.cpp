// ѹ��.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "My minilzo.h"
#include "resource.h"
#include "windows.h"
#include "Commdlg.h"

typedef struct
{
	unsigned long org_data_size;//δѹ�������ݴ�С
	unsigned long cmp_data_size;//ѹ��������ݴ�С
}COMP_DATA, *P_COMP_DATA;

#define DATA(x) x+8

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

void strcut(char *szStr,int n)
{
	szStr[n]=0;
	szStr[n+1]=0;
	szStr[n+2]=0;
}

int CALLBACK ProcDlgMain (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char str[MAX_PATH];
	char szPath[MAX_PATH];
	unsigned long size,nSfile,nDfile;
	unsigned char *pSData,*pDData;
	DWORD dwRead,dwWrite;
	HANDLE hFileS,hFileD;
	P_COMP_DATA pcd;

	switch(message)
	{
	case WM_COMMAND:
		  switch(LOWORD(wParam))
		  {
		  case IDC_SELFILE:
			  if(OpenFileDlg(hwnd,"�����ļ�\0*.*\0",szPath))
			  {
				  SetDlgItemText(hwnd,IDC_FILEPATH,szPath);
				  GetFileTypeSz(szPath,str);
				  if(!lstrcmp(str,"lzl"))//�����չ����lzl����Ϊѹ���ļ�
				  {
					  EnableWindow(GetDlgItem(hwnd,IDC_DECOMPRESS),1);
					  EnableWindow(GetDlgItem(hwnd,IDC_COMPRESS),0);
					  break;
				  }
				  EnableWindow(GetDlgItem(hwnd,IDC_COMPRESS),1);
				  EnableWindow(GetDlgItem(hwnd,IDC_DECOMPRESS),0);
			  }
			  break;
		  case IDC_COMPRESS:
			  GetDlgItemText(hwnd,IDC_FILEPATH,szPath,MAX_PATH);
			  //��Դ�ļ�
			  hFileS=CreateFile(szPath,GENERIC_ALL,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			  if(hFileS==INVALID_HANDLE_VALUE)
			  {
				  MessageBox(hwnd,"�ļ���ʧ��","��ʾ",MB_OK);
				  return 1;
			  }
			  lstrcpy(str,szPath);
			  lstrcat(str,".lzl");			  
			  //��Ŀ���ļ�
			  hFileD=CreateFile(str,GENERIC_ALL,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			  if(hFileD==INVALID_HANDLE_VALUE)
			  {
				  MessageBox(hwnd,"�ļ�����ʧ��","��ʾ",MB_OK);
				  return 1;
			  }
			  //�����ڴ�ռ�
			  nSfile=GetFileSize(hFileS,NULL);
			  pSData=(BYTE *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,nSfile);
			  if(!pSData)
			  {
				  MessageBox(hwnd,"����Դ�ļ��ڴ�ռ�ʧ��","��ʾ",MB_OK);
				  break;
			  }
			  if(!ReadFile(hFileS,pSData,nSfile,&dwRead,0))
			  {
				  MessageBox(hwnd,"��ȡԴ�ļ�ʧ��","��ʾ",MB_OK);
				  break;
			  }
			  pDData=(BYTE *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,nSfile+nSfile/16+64+3);
			  if(!pDData)
			  {
				  MessageBox(hwnd,"����Ŀ���ļ��ڴ�ռ�ʧ��","��ʾ",MB_OK);
				  break;
			  }
			  pcd=(P_COMP_DATA)pDData;
			  pcd->org_data_size=nSfile;//��дԴ�ļ���С
			  lzo_init();//��ʼ��minilzo��
			  if(Compress(pSData,nSfile,DATA(pDData),&size,wrkmem)!=LZO_E_OK)
			  {
				  MessageBox(hwnd,"ѹ��ʧ��","��ʾ",MB_OK);
				  break;
			  }
			  pcd->cmp_data_size=size;//��дѹ�������ݴ�С
			  if(!WriteFile(hFileD,pDData,size,&dwWrite,0))
			  {
				  MessageBox(hwnd,"д��Ŀ���ļ�ʧ��","��ʾ",MB_OK);
				  break;
			  }
			  CloseHandle(hFileS);CloseHandle(hFileD);
			  MessageBox(hwnd,"ѹ���ɹ�","��ʾ",MB_OK);
			  break;
		  case IDC_DECOMPRESS:
			  GetDlgItemText(hwnd,IDC_FILEPATH,szPath,MAX_PATH);
			  //��Դ�ļ�
			  hFileS=CreateFile(szPath,GENERIC_ALL,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			  if(hFileS==INVALID_HANDLE_VALUE)
			  {
				  MessageBox(hwnd,"�ļ���ʧ��","��ʾ",MB_OK);
				  return 1;
			  }
			  lstrcpy(str,szPath);
			  strcut(str,lstrlen(str)-4);
			  //��Ŀ���ļ�
			  hFileD=CreateFile(str,GENERIC_ALL,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			  if(hFileD==INVALID_HANDLE_VALUE)
			  {
				  MessageBox(hwnd,"�ļ�����ʧ��","��ʾ",MB_OK);
				  return 1;
			  }
			  //�����ڴ�ռ�
			  nSfile=GetFileSize(hFileS,NULL);
			  pSData=(BYTE *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,nSfile);
			  if(!pSData)
			  {
				  MessageBox(hwnd,"����Դ�ļ��ڴ�ռ�ʧ��","��ʾ",MB_OK);
				  break;
			  }
			  ReadFile(hFileS,pSData,nSfile,&dwRead,0);
			  pcd=(P_COMP_DATA)pSData;
			  pDData=(BYTE *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,pcd->org_data_size);
			  if(!pDData)
			  {
				  MessageBox(hwnd,"����Ŀ���ļ��ڴ�ռ�ʧ��","��ʾ",MB_OK);
				  break;
			  }
			  lzo_init();//��ʼ��minilzo��
			  size=pcd->org_data_size;
			  if(lzo1x_decompress_safe(DATA(pSData),pcd->cmp_data_size,pDData,&size,NULL)!=LZO_E_OK)
			  {
				  //MessageBox(hwnd,"��ѹ��ʧ��","��ʾ",MB_OK);
				  //break;
			  }
			  WriteFile(hFileD,pDData,size,&dwWrite,0);
			  CloseHandle(hFileS);CloseHandle(hFileD);
			  MessageBox(hwnd,"��ѹ���ɹ�","��ʾ",MB_OK);
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

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	DialogBoxParam(hInstance,(const char *)IDD_DLGMAIN,NULL,ProcDlgMain,0);
	return 0;
}



