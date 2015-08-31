//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TKeyboard *Keyboard;
//---------------------------------------------------------------------------
__fastcall TKeyboard::TKeyboard(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
HANDLE hDevice;
unsigned char Key_code='a';
class key
{
  public:
    unsigned char scan;
    void key_down(char code)
    {
      char ch=code;
      ch&=0x7f;
      FileWrite((int)hDevice,(const void *)&ch,1);
    }
    void key_up(char code)
    {
      char ch=code;
      ch&=0x80;
      FileWrite((int)hDevice,(const void *)&ch,1);
    }
}Keys;
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

void __fastcall TKeyboard::FormCreate(TObject *Sender)
{
   char str[260];
   char szSys[260];
   //���һЩ��ʼ������
   //-------------------------
   //��װ����
   GetCurrentDirectory(260,str);
   wsprintf(szSys,"%s\\Keyboard.sys",str);
   if(!InstallService("mykey",szSys,1)
   {
     ShowMessage("��������ʧ��");
     ExitProcess(0);
   }
   //ȫ���豸���ļ������   ��ʼ��
   //hDevice=(HANDLE)FileOpen(L"\\.\\\\wriKeyPort",fmOpenReadWrite);
   hDevice=CreateFileW(L"\\\\.\\wriKeyPort",GENERIC_READ|GENERIC_WRITE,0,
                        NULL,OPEN_EXISTING,0,NULL);
   if(hDevice==(HANDLE)INVALID_HANDLE_VALUE)
   {
     wsprintf(str,"�豸��ʧ�ܣ�������룺%d",GetLastError());
     ShowMessage(str);
     ExitProcess(0);
   }
}
//---------------------------------------------------------------------------

void __fastcall TKeyboard::StartClick(TObject *Sender)
{
   Timer1->Interval=Interval->Text.ToInt();
   Timer1->Enabled=true;
}
//---------------------------------------------------------------------------
void __fastcall TKeyboard::Timer1Timer(TObject *Sender)
{
  if(down->Checked)
    Keys.key_down(Key_code);
  if(up->Checked)
    Keys.key_up(Key_code);
}
//---------------------------------------------------------------------------
void __fastcall TKeyboard::EndClick(TObject *Sender)
{
  Timer1->Enabled=false;
}
//---------------------------------------------------------------------------

void __fastcall TKeyboard::KeyCodeKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
   char str[20];
   Key_code=MapVirtualKey(Key,0);
   wsprintf(str,"ɨ���룺0x%x",Key);
   KeyCode->Text=str;
}
//---------------------------------------------------------------------------

