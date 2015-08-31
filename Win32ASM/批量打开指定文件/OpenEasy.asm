;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		.386
		.model flat, stdcall
		option casemap :none
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; Include �ļ�����
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
include		windows.inc
include 		shell32.inc
includelib	shell32.lib
include		user32.inc
includelib	user32.lib
include		kernel32.inc
includelib	kernel32.lib
include		Advapi32.inc
includelib	Advapi32.lib
include		Comdlg32.inc
includelib	Comdlg32.lib
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; Equ ��ֵ����
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IDD_DIALOG1		equ	101
IDC_LIST		equ	1000
IDC_INFO		equ	1001
IDC_IMM		equ	1002
IDC_CHECK1		equ	1003
IDC_OPEN		equ	1004
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; ���ݶ�
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		.data?

hInstance	dd	?
hWinMain	dd	?
hSubKey	dd	?
hKey		dd	?
		.const
szDian	db	".",0
szSubKey	db	"SoftWare\OpenEasy",0
szFilter	db	"�����ļ�",0,"*.*",0,0
szOprea	db	"open",0
szExe		db	"open.exe",0
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; �����
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		.code
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; Take the filename
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_TakeFileName		proc	lpszFile,lpszFileName
		pushad
		mov ebx,lpszFile											;�ַ�������ַ
		xor eax,eax
		.while 1														;�ַ�������
			mov cl,[ebx+eax]
			.break .if cl==0
			inc eax
		.endw
		.while 1														;�ҵ�'\'��ƫ����
			mov cl,[ebx+eax]
			.break .if cl=='\'
			dec eax
			.if eax==0												;��û�У��򷵻�0
				ret
			.endif
		.endw
		inc eax
		mov esi,lpszFileName
		xor edx,edx
		.while 1														;����'\'������ַ���
			mov cl,[ebx+eax]
			mov [esi+edx],cl
			.break .if cl==0
			inc eax
			inc edx
		.endw
		popad
		mov eax,1
		ret
_TakeFileName		endp

;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; ͨ�öԻ���  ���ļ�
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_OpenFile		proc	lpszFilter,lpszFile
		local @stOF:OPENFILENAME
		
		invoke RtlZeroMemory,addr @stOF,sizeof @stOF
		invoke RtlZeroMemory,lpszFile,MAX_PATH
		mov @stOF.lStructSize,sizeof @stOF
		push hWinMain
		pop  @stOF.hwndOwner
		push lpszFilter
		pop  @stOF.lpstrFilter													;�����ļ�
		mov eax,lpszFile
		mov @stOF.lpstrFile,eax													;�ļ�·����
		mov  @stOF.nMaxFile,MAX_PATH
		mov  @stOF.lpstrFileTitle,NULL										;�ļ���
		mov  @stOF.Flags,OFN_FILEMUSTEXIST  or OFN_PATHMUSTEXIST
		invoke GetOpenFileName,addr @stOF
		ret
_OpenFile		endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;��ӵ�ע���
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_AddToRegistry  proc			lpszFile
local hAppKeyN
local dwIndex
local dwSize
local szFileName[MAX_PATH]:BYTE
local szBuf[MAX_PATH]:BYTE

invoke _TakeFileName,lpszFile,addr szFileName
	mov dwIndex,0
	 .while 1
		mov dwSize,sizeof szBuf
		invoke RegEnumKeyEx,hKey,dwIndex,addr szBuf,addr dwSize,0,0,0,0
		.break .if eax==ERROR_NO_MORE_ITEMS
		invoke lstrcmp,addr szFileName,addr szBuf											;ͬ����鴦��
		.if eax==0
			invoke lstrcat,addr szFileName,offset szDian
		.endif
		inc dwIndex
   .endw
invoke RegCreateKeyEx,hKey,addr szFileName,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,addr hAppKeyN,0
invoke RegSetValueEx,hAppKeyN,0,0,REG_SZ,lpszFile,MAX_PATH
invoke CloseHandle,hAppKeyN
ret
_AddToRegistry   endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;�����б��
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_UpdateListBox   proc 
	local dwSize
	local lpcbData
	local dwIndex
	local hAppKeyN
	local szBuf[MAX_PATH]:BYTE
	mov dwIndex,0
	mov lpcbData,256
	invoke SendDlgItemMessage,hWinMain,IDC_LIST,LB_RESETCONTENT,0,0			;����б��ԭ��������
   .while 1
   	mov dwSize,sizeof szBuf
   	invoke RegEnumKeyEx,hKey,dwIndex,addr szBuf,addr dwSize,0,0,0,0
   	.break .if eax==ERROR_NO_MORE_ITEMS
   	inc dwIndex
   	invoke SendDlgItemMessage,hWinMain,IDC_LIST,LB_ADDSTRING,0,addr szBuf
   .endw
	ret
_UpdateListBox   endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_ProcDlgMain	proc	uses ebx edi esi hWnd,wMsg,wParam,lParam
		;****�ֲ�����****
		local szFile[MAX_PATH]:BYTE
		local szListText[MAX_PATH]:BYTE
		local dwData
		local hAppKeyN
		
		mov	eax,wMsg
		.if	eax == WM_CLOSE
			;***�˳�ʱ����***
			invoke EndDialog,hWnd,0
		.elseif	eax == WM_INITDIALOG
			;***��ʼ������***
			push hWnd
			pop hWinMain
			;��ע�����Ӧ�Ӽ�
			invoke RegCreateKeyEx,HKEY_LOCAL_MACHINE,offset szSubKey,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,offset hKey,0
			invoke _UpdateListBox
		.elseif	eax == WM_COMMAND
			;***�Ӵ�������***
			mov	eax,wParam
			.if	ax ==	IDCANCEL
				invoke SendDlgItemMessage,hWnd,IDC_LIST,LB_GETCURSEL,0,0
				mov ebx,eax
				invoke SendDlgItemMessage,hWnd,IDC_LIST,LB_GETTEXT,ebx,addr szListText
				invoke RegDeleteKey,hKey,addr szListText
				invoke _UpdateListBox
			.elseif	ax ==	IDOK
				invoke _OpenFile,offset szFilter,addr szFile
				.if eax==0
					jmp @f
				.endif
				invoke _AddToRegistry,addr szFile
				invoke _UpdateListBox
			.elseif 	ax==IDC_OPEN
				invoke SendDlgItemMessage,hWnd,IDC_LIST,LB_GETCURSEL,0,0
				mov ebx,eax
				invoke SendDlgItemMessage,hWnd,IDC_LIST,LB_GETTEXT,ebx,addr szListText
				invoke RegOpenKeyEx,hKey,addr szListText,0,KEY_ALL_ACCESS,addr hAppKeyN
				mov dwData,MAX_PATH
				invoke RegQueryValueEx,hAppKeyN,NULL,0,NULL,addr szFile,addr dwData
				invoke RegCloseKey,hAppKeyN
				invoke ShellExecute,hWnd,offset szOprea,addr szFile,0,0,SW_NORMAL
			.elseif ax==IDC_IMM
				invoke ShellExecute,hWnd,offset szOprea,offset szExe,0,0,SW_NORMAL
			.elseif ax==IDC_LIST
				shr	eax,16
				.if ax==LBN_SELCHANGE
					invoke SendDlgItemMessage,hWnd,IDC_LIST,LB_GETCURSEL,0,0
					mov ebx,eax
					invoke SendDlgItemMessage,hWnd,IDC_LIST,LB_GETTEXT,ebx,addr szListText
					invoke RegOpenKeyEx,hKey,addr szListText,0,KEY_ALL_ACCESS,addr hAppKeyN
					mov dwData,MAX_PATH
					invoke RegQueryValueEx,hAppKeyN,NULL,0,NULL,addr szFile,addr dwData
					invoke RegCloseKey,hAppKeyN
					invoke SetDlgItemText,hWnd,IDC_INFO,addr szFile
				.endif
			.endif
		.else
			mov	eax,FALSE
			ret
		.endif
	@@:mov	eax,TRUE
		ret

_ProcDlgMain	endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
start:
		invoke	GetModuleHandle,NULL
		mov	hInstance,eax
		invoke	DialogBoxParam,hInstance,IDD_DIALOG1,NULL,offset _ProcDlgMain,NULL
		invoke	ExitProcess,NULL
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		end	start
