;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		.386
		.model flat, stdcall
		option casemap :none
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; Include �ļ�����
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
include		windows.inc
include		shell32.inc
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
EDIT			equ	1
NEW			equ	0

ADATE			equ	000h
AWEEK			equ	100h
NDLG			equ	200h

WM_ICON		equ	WM_USER+5

IDD_MAIN		equ	101
IDD_NEW		equ	102
IDD_SHOW		equ	103
IDI_ICON1	equ	104
IDC_LIST		equ	1000
IDC_TEXT		equ	1001
IDC_NEW		equ	1002
IDC_EDIT		equ	1003
IDC_DEL		equ	1004
IDC_ABOUT		equ	1005
IDC_ADATE		equ	1015
IDC_AWEEK		equ	1016
IDC_NAME		equ	1020
IDC_YEAR		equ	1024
IDC_MONTH		equ	1025
IDC_DAY		equ	1026
IDC_HOUR		equ	1027
IDC_MIN		equ	1028
IDC_CDSECONDS		equ	1032
IDC_FILEPATH		equ	1033
IDC_SELFILE		equ	1034
IDC_TskTEXT		equ	1035
IDC_CHECK7		equ	1057
IDC_CHECK1		equ	1051
IDC_CHECK2		equ	1052
IDC_CHECK3		equ	1053
IDC_CHECK4		equ	1054
IDC_CHECK5		equ	1055
IDC_CHECK6		equ	1056
IDC_ISDIALOG	equ	1043
IDC_TXTSHOW		equ	1008
IDC_NAMESHOW   equ   1044
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; ����ṹ��
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Task	struct
name1	db	22 dup(?)
text	db	202 dup(?)
appath	db	200 dup(?)
year	dd	?
month	dd	?
day	dd	?
hour	dd	?
min	dd	?
ttype	dd	?

Task	ends
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; ���ݶ�
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		.data?
hInstance	dd	?
hWinMain		dd	?
hRunKey		dd	?
hAppKey		dd	?
hAppKeyN		dd	?
hKeyList		dd	?
hKeyDate		dd	?
hKeyXingqi	dd	?
@opration	dd	?
lpOldProc	dd	?
szBuf			dd	200 dup(?)
task			Task		<>
		.data
bTest			dd	1b,10b,100b,1000b,10000b,100000b,1000000b
idTimer		dd 0
bShow			dd	0
		.const
szCaption		db	"Personal secretary ",0
szSubRunKey		db	"Software\Microsoft\Windows\CurrentVersion\Run",0
szKeyList		db	"Software\GoodMemory\list",0
szKeyDate		db	"Software\GoodMemory\index\date",0
szKeyXingqi		db	"Software\GoodMemory\index\xingqi",0
szEdit			db	"�޸�����",0

szName			db	"Name",0
szText			db	"Text",0
szYear			db	"Year",0
szMonth			db	"Month",0
szDay				db	"Day",0
szHour			db	"Hour",0
szMin				db	"Min",0
szType			db	"type",0
szPath			db	"path",0
szMsgCap			db	"��ʾ",0
szSameName		db	"����ͬ������",0
szError1			db	"����������С��10������!",0
szError2			db	"�������ݱ���С��100������!",0
szError3			db	"ע������ʧ��!",0
szError4			db	"����������Ϊ�գ�",0
szTimeErr		db	"ʱ���ʽ����",0
szAboutCap		db	"����",0
szAbout			db	"���ߣ�Code Soul",10,13,
						"�汾��V1.0",10,13,
						"2013.6.17",0
szFmt				db	"��������:%s",13,10,
						"�������ݣ�%s",13,10,
						"ִ��·����%s",13,10,
						"����ʱ�䣺",0
szFmtData		db	"%d��%d��%d��%dʱ%d��",0
szFmtTime		db	"%dʱ%d��",13,10,0
szFmtWeek		db	"	��һ",13,10,0,
						"	�ܶ�",13,10,0,
						"	����",13,10,0,
						"	����",13,10,0,
						"	����",13,10,0,
						"	����",13,10,0,
						"	����",0

szIcon			db	"˫����Personal secretary",0
szFmtKeyD		db	"%d\%d\%d\%d\%d",0
szFmtKeyZ		db	"%d\%d\%d",0
szFilter			db	"Ӧ�ó���(*.exe)",0,"*.exe",0,0
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; �����
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		.code
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;��ѯ��ֵto task�ṹ
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_QueryValue	proc	pszToQuery
	local lpcbData
	invoke RegOpenKeyEx,hKeyList,pszToQuery,0,KEY_ALL_ACCESS,offset hAppKeyN
	.if eax==ERROR_SUCCESS
		invoke RegQueryValueEx,hAppKeyN,offset szName,0,NULL,offset task.name1,addr lpcbData
		mov lpcbData,202
		invoke RegQueryValueEx,hAppKeyN,offset szText,0,NULL,offset task.text,addr lpcbData
		invoke RegQueryValueEx,hAppKeyN,offset szYear,0,NULL,offset task.year,addr lpcbData
		invoke RegQueryValueEx,hAppKeyN,offset szMonth,0,NULL,offset task.month,addr lpcbData
		invoke RegQueryValueEx,hAppKeyN,offset szDay,0,NULL,offset task.day,addr lpcbData
		invoke RegQueryValueEx,hAppKeyN,offset szHour,0,NULL,offset task.hour,addr lpcbData
		invoke RegQueryValueEx,hAppKeyN,offset szMin,0,NULL,offset task.min,addr lpcbData
		invoke RegQueryValueEx,hAppKeyN,offset szType,0,NULL,offset task.ttype,addr lpcbData
	.endif
	invoke CloseHandle,hAppKeyN
	ret
_QueryValue	endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;��ӵ�ע���
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_AddToRegistry  proc	uses eax
invoke RegCreateKeyEx,hKeyList,offset task.name1,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,offset hAppKeyN,0
invoke RegSetValueEx,hAppKeyN,offset szName,0,REG_SZ,offset task.name1,22
invoke RegSetValueEx,hAppKeyN,offset szText,0,REG_SZ,offset task.text,202
invoke RegSetValueEx,hAppKeyN,offset szPath,0,REG_SZ,offset task.appath,200
invoke RegSetValueEx,hAppKeyN,offset szYear,0,REG_DWORD,offset task.year,4
invoke RegSetValueEx,hAppKeyN,offset szMonth,0,REG_DWORD,offset task.month,4
invoke RegSetValueEx,hAppKeyN,offset szDay,0,REG_DWORD,offset task.day,4
invoke RegSetValueEx,hAppKeyN,offset szHour,0,REG_DWORD,offset task.hour,4
invoke RegSetValueEx,hAppKeyN,offset szMin,0,REG_DWORD,offset task.min,4
invoke RegSetValueEx,hAppKeyN,offset szType,0,REG_DWORD,offset task.ttype,4
invoke CloseHandle,hAppKeyN
ret
_AddToRegistry   endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; ����������
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_AutoRun  proc	uses eax
            local @szFileName[256]:BYTE
invoke RegOpenKeyEx,HKEY_LOCAL_MACHINE,offset szSubRunKey,0,KEY_ALL_ACCESS,offset hRunKey
invoke GetModuleFileName,0,addr @szFileName,sizeof @szFileName
inc eax
invoke RegSetValueEx,hRunKey,offset szCaption,0,REG_SZ,addr @szFileName,eax
invoke CloseHandle,hRunKey
ret
_AutoRun   endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;��ע���ɾ��ָ������
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_DelFromRegistry	proc	pszKeyName
	local @szBuf[100]:BYTE
	local @dwBuf

	invoke _QueryValue,pszKeyName
	test task.ttype,AWEEK
	.if ZERO?
		invoke wsprintf,addr @szBuf,offset szFmtKeyD,task.year,task.month,task.day,task.hour,task.min
		invoke RegOpenKeyEx,hKeyDate,addr @szBuf,NULL,KEY_ALL_ACCESS,offset hAppKeyN
		invoke RegDeleteKey,hAppKeyN,pszKeyName
		invoke RegDeleteKey,hKeyList,pszKeyName
	.else
		mov @dwBuf,0
		.while @dwBuf<=6
			mov eax,@dwBuf
			mov ecx,[eax*4+offset bTest]
			test task.ttype,ecx
			.if !ZERO?
				mov ebx,eax
				inc ebx
				invoke wsprintf,addr @szBuf,offset szFmtKeyZ,ebx,task.hour,task.min
				invoke RegOpenKeyEx,hKeyXingqi,addr @szBuf,NULL,KEY_ALL_ACCESS,offset hAppKeyN
				invoke RegDeleteKey,hAppKeyN,pszKeyName
			.endif
			inc @dwBuf
		.endw
		invoke RegDeleteKey,hKeyList,pszKeyName
	.endif
	invoke RegCloseKey,hAppKeyN
	ret
_DelFromRegistry	endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;�����б��
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_UpdateListBox   proc 
	local dwSize
	local lpcbData
	local dwIndex
	local szTskNameBuf[22]:BYTE
	mov dwIndex,0
	mov lpcbData,256
	invoke SendDlgItemMessage,hWinMain,IDC_LIST,LB_RESETCONTENT,0,0			;����б��ԭ��������
   .while 1
   	mov dwSize,sizeof szTskNameBuf
   	invoke RegEnumKeyEx,hKeyList,dwIndex,addr szTskNameBuf,addr dwSize,0,0,0,0
   	.break .if eax==ERROR_NO_MORE_ITEMS
   	inc dwIndex
   	invoke SendDlgItemMessage,hWinMain,IDC_LIST,LB_ADDSTRING,0,addr szTskNameBuf
   .endw
	ret
_UpdateListBox   endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;�б�ؼ����໯  �µ�ַ
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_ProcList	proc	uses ebx edi esi hWnd,wMsg,wParam,lParam
		mov eax,wMsg
		.if eax==WM_KEYDOWN
			mov eax,wParam
			.if eax==VK_DELETE
				invoke SendMessage,hWinMain,WM_COMMAND,IDC_DEL,0
			.elseif eax==VK_RETURN
				invoke SendMessage,hWinMain,WM_COMMAND,IDC_EDIT,0
			.endif
			invoke CallWindowProc,lpOldProc,hWnd,wMsg,wParam,lParam
		.else
			invoke CallWindowProc,lpOldProc,hWnd,wMsg,wParam,lParam
		.endif
		ret
_ProcList	endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;����ģ��  ���ڹ���
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_ProcDlgShow	proc	uses ebx edi esi hWnd,wMsg,wParam,lParam
		local lpcbData
		local	stStartUp:STARTUPINFO
		local stProInfo:PROCESS_INFORMATION
		mov	eax,wMsg
		.if	eax == WM_INITDIALOG
			invoke RegOpenKeyEx,hKeyList,lParam,0,KEY_ALL_ACCESS,offset hAppKeyN
			invoke RegQueryValueEx,hAppKeyN,offset szType,0,NULL,offset task.ttype,addr lpcbData
			test task.ttype,NDLG
			.if ZERO?
				invoke EndDialog,hWnd,0
				jmp @f
			.endif
			invoke SetWindowPos,hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE				;���ö��㴰��
			invoke RegQueryValueEx,hAppKeyN,offset szName,0,NULL,offset task.name1,addr lpcbData
			invoke RegQueryValueEx,hAppKeyN,offset szText,0,NULL,offset task.text,addr lpcbData
			invoke SetDlgItemText,hWnd,IDC_NAMESHOW,offset task.name1
			invoke SetDlgItemText,hWnd,IDC_TXTSHOW,offset task.text
			invoke RegQueryValueEx,hAppKeyN,offset szPath,0,NULL,offset task.appath,addr lpcbData
			.if task.appath				;�򿪿�ִ���ļ�
				invoke GetStartupInfo,addr stStartUp
				invoke CreateProcess,NULL,offset task.appath,NULL,NULL,NULL,NORMAL_PRIORITY_CLASS,NULL,NULL,addr stStartUp,addr stProInfo
			.endif
		@@:invoke RegCloseKey,hAppKeyN
		.elseif	eax==WM_COMMAND
			invoke	EndDialog,hWnd,0
		.elseif eax==WM_CLOSE
			invoke	EndDialog,hWnd,0
		.else	;������Ϣ
			mov	eax,FALSE
			ret
		.endif
		mov	eax,TRUE
		ret
_ProcDlgShow	endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;�̺߳���
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ThreadProc		proc	uses ebx esi edi	lParam
	invoke DialogBoxParam,hInstance,IDD_SHOW,NULL,offset _ProcDlgShow,lParam
	ret
ThreadProc		endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; ��ʱ�����̣��ػ�
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_ProcTimerGuard	proc	uses ebx edi esi hWnd,uMsg,idEvent,dwTime
	local @second
	local @dwSize
	local @index
	local @buf[100]:BYTE
	local @systime:SYSTEMTIME
	invoke KillTimer,hWinMain,idTimer	;ɱ���ϸ�timer
	invoke GetLocalTime,addr @systime
	mov eax,60
	movzx ebx,@systime.wSecond
	sub eax,ebx
	mov @second,eax
	inc @second
	mov eax,1000
	mul @second
	invoke SetTimer,hWinMain,idTimer,eax,offset _ProcTimerGuard		;�����µ�timer
	pushad
	movzx eax,@systime.wYear
	movzx ebx,@systime.wMonth
	movzx ecx,@systime.wDay
	movzx edx,@systime.wHour
	movzx esi,@systime.wMinute
	invoke wsprintf,addr @buf,offset szFmtKeyD,eax,ebx,ecx,edx,esi
	popad
	invoke RegOpenKeyEx,hKeyDate,addr @buf,0,KEY_ALL_ACCESS,offset hAppKeyN				;�����ػ�
	.if eax==ERROR_SUCCESS
		mov @index,0
		.while 1
			mov @dwSize,sizeof @dwSize
			invoke RegEnumKeyEx,hKeyList,@index,addr @buf,addr @dwSize,0,0,0,0
			.break .if eax==ERROR_NO_MORE_ITEMS
			invoke CreateThread,NULL,NULL,offset ThreadProc,addr @buf,0,NULL
			;invoke DialogBoxParam,hInstance,IDD_SHOW,NULL,offset _ProcDlgShow,addr @buf
			inc @index
		.endw
	.endif
	
	movzx eax,@systime.wDayOfWeek
	movzx ebx,@systime.wHour
	movzx ecx,@systime.wMinute
	invoke wsprintf,addr @buf,offset szFmtKeyZ,eax,ebx,ecx
	invoke RegOpenKeyEx,hKeyXingqi,addr @buf,0,KEY_ALL_ACCESS,offset hAppKeyN				;�����ػ�
	.if eax==ERROR_SUCCESS
		mov @index,0
		.while 1
			mov @dwSize,sizeof @dwSize
			invoke RegEnumKeyEx,hKeyList,@index,addr @buf,addr @dwSize,0,0,0,0
			.break .if eax==ERROR_NO_MORE_ITEMS
			invoke CreateThread,NULL,NULL,offset ThreadProc,addr @buf,0,NULL
			.if !eax
				invoke MessageBox,NULL,offset szError1,offset szCaption,MB_OK
			.endif
			;invoke DialogBoxParam,hInstance,IDD_SHOW,NULL,offset _ProcDlgShow,addr @buf
			inc @index
		.endw
	.endif
	invoke RegCloseKey,hAppKeyN
	ret
_ProcTimerGuard	endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;���½����񡱴��ڹ���
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_ProcDlgNew	proc	uses ebx ecx edi esi hWnd,wMsg,wParam,lParam
		;****�ֲ�����****
		local szCurSelName[22]:BYTE
		local @szBuf[200]:BYTE
		local @index
		local @systime:SYSTEMTIME
		local @stOF:OPENFILENAME
		local @dwSize
		local @sf			;SUCCESS or fail

		mov	eax,wMsg
		.if	eax == WM_CLOSE
			;***�˳�ʱ����***
			invoke EndDialog,hWnd,0
		.elseif	eax == WM_INITDIALOG
			;***��ʼ������***
			.if @opration==EDIT
				;������ѡ��
				invoke SetWindowText,hWnd,offset szEdit
				invoke SendDlgItemMessage,hWinMain,IDC_LIST,LB_GETCURSEL,0,0	;�õ���ǰ����ѡ��
				mov @index,eax
				invoke SendDlgItemMessage,hWinMain,IDC_LIST,LB_GETTEXT,@index,addr szCurSelName
				invoke _QueryValue,addr szCurSelName
				test task.ttype,AWEEK														;�Ƿ�����Ϣ��ʾ
				.if ZERO?																		;������
					invoke CheckRadioButton,hWnd,IDC_ADATE,IDC_AWEEK,IDC_ADATE;���û���ĵ�ѡť
					invoke SendMessage,hWnd,WM_COMMAND,IDC_ADATE,0
					invoke SetDlgItemInt,hWnd,IDC_YEAR,task.year,0
					invoke SetDlgItemInt,hWnd,IDC_MONTH,task.month,0
					invoke SetDlgItemInt,hWnd,IDC_DAY,task.day,0
				.else																				;����
					invoke CheckRadioButton,hWnd,IDC_ADATE,IDC_AWEEK,IDC_AWEEK
					invoke SendMessage,hWnd,WM_COMMAND,IDC_ADATE,0
					mov @dwSize,0
					.while @dwSize<=6
						mov eax,@dwSize
						mov ecx,[eax*4+offset bTest]
						test task.ttype,ecx
						.if !ZERO?			;�ܲ���ͨ��
							mov edi,IDC_CHECK1
							add edi,eax
							invoke CheckDlgButton,hWnd,edi,BST_CHECKED
						.endif
						inc @dwSize
					.endw
				.endif
				test task.ttype,NDLG															;�Ƿ�Ի�������
				.if !ZERO?
					invoke CheckDlgButton,hWnd,IDC_ISDIALOG,BST_CHECKED
				.endif
				invoke SetDlgItemText,hWnd,IDC_NAME,offset task.name1
				invoke SetDlgItemText,hWnd,IDC_TskTEXT,offset task.text
				invoke SetDlgItemText,hWnd,IDC_FILEPATH,offset task.appath
				invoke SetDlgItemInt,hWnd,IDC_HOUR,task.hour,0
				invoke SetDlgItemInt,hWnd,IDC_MIN,task.min,0
			.else
				invoke CheckRadioButton,hWnd,IDC_ADATE,IDC_AWEEK,IDC_ADATE
				;���ص�ǰʱ��
				invoke GetLocalTime,addr @systime
				movzx eax,@systime.wYear
				invoke SetDlgItemInt,hWnd,IDC_YEAR,eax,0
				movzx eax,@systime.wMonth
				invoke SetDlgItemInt,hWnd,IDC_MONTH,eax,0
				movzx eax,@systime.wDay
				invoke SetDlgItemInt,hWnd,IDC_DAY,eax,0
				movzx eax,@systime.wHour
				invoke SetDlgItemInt,hWnd,IDC_HOUR,eax,0
				movzx eax,@systime.wMinute
				invoke SetDlgItemInt,hWnd,IDC_MIN,eax,0
				
				invoke CheckDlgButton,hWnd,IDC_ISDIALOG,BST_CHECKED			;��ʼ����ѡ��
				mov @dwSize,IDC_CHECK1
				.while @dwSize<=IDC_CHECK7
					invoke GetDlgItem,hWnd,@dwSize
					invoke EnableWindow,eax,0
					inc @dwSize
				.endw
			.endif
		.elseif	eax == WM_COMMAND
			;***�Ӵ�������***
			mov	eax,wParam
			.if	ax ==	IDCANCEL
				invoke EndDialog,hWnd,0
			.elseif	ax==IDC_SELFILE													;ѡ���ļ�
				invoke RtlZeroMemory,addr @stOF,sizeof @stOF
				invoke RtlZeroMemory,addr @szBuf,sizeof @szBuf
				mov @stOF.lStructSize,sizeof @stOF
				push hWnd
				pop  @stOF.hwndOwner
				mov  @stOF.lpstrFilter,offset szFilter								;�����ļ�
				lea  eax,@szBuf
				mov  @stOF.lpstrFile,eax												;�ļ�·����
				mov  @stOF.nMaxFile,MAX_PATH
				mov  @stOF.lpstrFileTitle,NULL										;�ļ���
				mov  @stOF.Flags,OFN_FILEMUSTEXIST  or OFN_PATHMUSTEXIST
				invoke GetOpenFileName,addr @stOF
				invoke SetDlgItemText,hWnd,IDC_FILEPATH,addr @szBuf
			.elseif	ax ==	IDOK
				.if @opration==EDIT;ɾ������
					invoke SendDlgItemMessage,hWinMain,IDC_LIST,LB_GETCURSEL,0,0
					mov ebx,eax
					invoke SendDlgItemMessage,hWinMain,IDC_LIST,LB_GETTEXT,ebx,addr szCurSelName
					invoke _DelFromRegistry,addr szCurSelName
				.endif
				;�ַ�����Խ����
				invoke GetDlgItemText,hWnd,IDC_NAME,offset task.name1,100
				.if eax>20
					invoke MessageBox,hWnd,offset szError1,offset szMsgCap,MB_OK
					jmp @f
				.elseif eax==0																			;�����������
					invoke MessageBox,hWnd,offset szError4,offset szMsgCap,MB_OK
					jmp @f
				.endif
				invoke GetDlgItemText,hWnd,IDC_TskTEXT,offset task.text,1024
				.if eax>200
					invoke MessageBox,hWnd,offset szError2,offset szMsgCap,MB_OK
					jmp @f
				.endif
				;����ͬ�����
				mov @index,0
				 .while 1
					mov @dwSize,sizeof szCurSelName
					invoke RegEnumKeyEx,hKeyList,@index,addr szCurSelName,addr @dwSize,0,0,0,0
					.break .if eax==ERROR_NO_MORE_ITEMS
					invoke lstrcmp,addr szCurSelName,offset task.name1
					.if eax==0
						invoke MessageBox,hWnd,offset szSameName,offset szMsgCap,MB_OK
						jmp @f
					.endif
					inc @index
				.endw
				;ִ�г���·��
				invoke GetDlgItemText,hWnd,IDC_FILEPATH,offset task.appath,200
				mov task.ttype,0
				;ʱ����  ʱ  ��
				invoke GetDlgItemInt,hWnd,IDC_HOUR,addr @sf,0
				mov task.hour,eax
				invoke GetDlgItemInt,hWnd,IDC_MIN,addr @sf,0
				mov task.min,eax
				;������
				invoke	IsDlgButtonChecked,hWnd,IDC_ADATE
				.if eax==BST_CHECKED
					or task.ttype,ADATE
					;���Task�ṹ
					invoke GetDlgItemInt,hWnd,IDC_YEAR,addr @sf,0
					mov task.year,eax
					invoke GetDlgItemInt,hWnd,IDC_MONTH,addr @sf,0
					mov task.month,eax
					invoke GetDlgItemInt,hWnd,IDC_DAY,addr @sf,0
					mov task.day,eax
					;����������
					invoke wsprintf,addr @szBuf,offset szFmtKeyD,task.year,task.month,task.day,task.hour,task.min
					invoke RegCreateKeyEx,hKeyDate,addr @szBuf,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,offset hAppKeyN,0
					invoke RegCreateKeyEx,hAppKeyN,offset task.name1,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,offset hAppKeyN,0
					invoke RegCloseKey,hAppKeyN
				.endif
				;������
				invoke	IsDlgButtonChecked,hWnd,IDC_AWEEK
				.if eax==BST_CHECKED
					or task.ttype,AWEEK
					;�����  ��Ϣ
					mov @dwSize,1051
					.while @dwSize<=1057
						invoke	IsDlgButtonChecked,hWnd,@dwSize
						.if eax==BST_CHECKED
							mov eax,@dwSize
							sub eax,1050
							dec eax
							mov ecx,[eax*4+offset bTest]
							or task.ttype,ecx
							mov ebx,eax
							inc ebx
							;����������
							invoke wsprintf,addr @szBuf,offset szFmtKeyZ,ebx,task.hour,task.min
							invoke RegCreateKeyEx,hKeyXingqi,addr @szBuf,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,offset hAppKeyN,0
						.endif
						inc @dwSize
					.endw
					invoke RegCloseKey,hAppKeyN
				.endif
				;�Ƿ��öԻ�����ʾ
				invoke	IsDlgButtonChecked,hWnd,IDC_ISDIALOG
				.if eax==BST_CHECKED
					or task.ttype,NDLG
				.endif
				;��ӵ�list��
				invoke _AddToRegistry
				invoke EndDialog,hWnd,0
				;ѡ����������ʱ��Ч��
			.elseif ax==IDC_ADATE
				mov @dwSize,IDC_YEAR
				.while @dwSize<=IDC_DAY
					invoke GetDlgItem,hWnd,@dwSize
					invoke EnableWindow,eax,1
					inc @dwSize
				.endw
				
				mov @dwSize,IDC_CHECK1
				.while @dwSize<=IDC_CHECK7
					invoke GetDlgItem,hWnd,@dwSize
					invoke EnableWindow,eax,0
					inc @dwSize
				.endw
			.elseif ax==IDC_AWEEK
				mov @dwSize,IDC_CHECK1
				.while @dwSize<=IDC_CHECK7
					invoke GetDlgItem,hWnd,@dwSize
					invoke EnableWindow,eax,1
					inc @dwSize
				.endw
				
				mov @dwSize,IDC_YEAR
				.while @dwSize<=IDC_DAY
					invoke GetDlgItem,hWnd,@dwSize
					invoke EnableWindow,eax,0
					inc @dwSize
				.endw
			.endif

		.else	;������Ϣ
			mov	eax,FALSE
			ret
		.endif
		@@:mov	eax,TRUE
		ret

_ProcDlgNew	endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
; �����ڹ���
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
_ProcDlgMain	proc	uses ebx edi esi hWnd,wMsg,wParam,lParam
		;****�ֲ�����****
		local dwSize
		local szCurSelName[22]:BYTE
		local @szBuf[200]:BYTE
		local @szBuf2[100]:BYTE
		local hIcon
		local ntico:NOTIFYICONDATA
		mov	eax,wMsg
		.if	eax == WM_CLOSE
			;***�˳�ʱ����***
			invoke ShowWindow,hWnd,SW_HIDE
			;invoke Shell_NotifyIcon,NIM_DELETE,addr ntico
			;invoke EndDialog,hWnd,0
		.elseif	eax == WM_INITDIALOG
			;***��ʼ������***
			invoke SetWindowText,hWnd,offset szCaption
			invoke LoadIcon,hInstance,IDI_ICON1
			mov hIcon,eax
			invoke SendMessage,hWnd,WM_SETICON,ICON_BIG,eax
			
			;ע���ȼ�
			invoke RegisterHotKey,hWnd,0,MOD_ALT,VK_P
			
			;���½�ͼ��
			mov ntico.cbSize,sizeof NOTIFYICONDATA
			push hWnd
			pop ntico.hwnd
			mov ntico.uID,0
			mov ntico.uFlags,NIF_MESSAGE OR NIF_ICON OR NIF_TIP
			mov ntico.uCallbackMessage,WM_ICON
			push hIcon
			pop ntico.hIcon
			mov ebx,offset szIcon
			lea eax,ntico.szTip
			mov esi,eax
			mov eax,0
			.while eax<=6
			push [ebx]
			pop  [esi]
			add ebx,4
			add esi,4
			inc eax
			.endw
			push 0
			pop [esi]
			invoke Shell_NotifyIcon,NIM_ADD,addr ntico
			
			push hWnd
			pop hWinMain
			invoke GetDlgItem,hWnd,IDC_LIST
			invoke SetWindowLong,eax,GWL_WNDPROC,offset _ProcList
			mov lpOldProc,eax
			invoke RegCreateKeyEx,HKEY_LOCAL_MACHINE,\	;��Ӧ�ó���ʹ�õ�hKey��hAppKey��������ʹ��
										offset szKeyList,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,offset hKeyList,0
			invoke RegCreateKeyEx,HKEY_LOCAL_MACHINE,\	;��Ӧ�ó���ʹ�õ�hKey��hAppKey��������ʹ��
										offset szKeyDate,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,offset hKeyDate,0
			invoke RegCreateKeyEx,HKEY_LOCAL_MACHINE,\	;��Ӧ�ó���ʹ�õ�hKey��hAppKey��������ʹ��
										offset szKeyXingqi,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,offset hKeyXingqi,0
			invoke _UpdateListBox
			;���ö�ʱ���ػ�����
			invoke SetTimer,hWinMain,idTimer,0,offset _ProcTimerGuard
			invoke GetDlgItem,hWnd,IDC_NEW
			invoke SetFocus,eax
		.elseif	eax == WM_COMMAND
			;***�Ӵ�������***
			mov	eax,wParam
			.if	ax==IDC_NEW	;�����Ӵ�������
				mov @opration,NEW
				invoke DialogBoxParam,hInstance,IDD_NEW,hWnd,offset _ProcDlgNew,0
				invoke _UpdateListBox
			.elseif	ax==IDC_EDIT
				mov @opration,EDIT
				invoke DialogBoxParam,hInstance,IDD_NEW,hWnd,offset _ProcDlgNew,0
				invoke _UpdateListBox
			.elseif	ax==IDC_DEL
				invoke SendDlgItemMessage,hWnd,IDC_LIST,LB_GETCURSEL,0,0
				mov ebx,eax
				invoke SendDlgItemMessage,hWinMain,IDC_LIST,LB_GETTEXT,ebx,addr szCurSelName
				invoke _DelFromRegistry,addr szCurSelName
				invoke _UpdateListBox
			.elseif	ax==IDC_ABOUT
				invoke MessageBox,hWnd,offset szAbout,offset szAboutCap,MB_OK
			.elseif	ax==IDC_LIST
				shr	eax,16
				.if ax==LBN_SELCHANGE
					invoke SendDlgItemMessage,hWnd,IDC_LIST,LB_GETCURSEL,0,0
					mov ebx,eax
					invoke SendDlgItemMessage,hWnd,IDC_LIST,LB_GETTEXT,ebx,addr szCurSelName
					invoke _QueryValue,addr szCurSelName
					;��ʾ�������ƺ�����
					invoke wsprintf,addr @szBuf,offset szFmt,offset task.name1,offset task.text,offset task.appath
					;���ܻ�������ʾ��������
					test task.ttype,AWEEK
					.if ZERO?;adata
						invoke wsprintf,addr @szBuf2,offset szFmtData,task.year,task.month,task.day,task.hour,task.min
						invoke lstrcat,addr @szBuf,addr @szBuf2
					.else		;aweek
						invoke wsprintf,addr @szBuf2,offset szFmtTime,task.hour,task.min
						invoke lstrcat,addr @szBuf,addr @szBuf2
						mov dwSize,0
						.while dwSize<=6
							mov eax,dwSize
							mov ecx,[eax*4+offset bTest]
							test task.ttype,ecx
							.if !ZERO?			;�ܲ���ͨ��
								mov eax,dwSize
								shl eax,3		;��8
								mov edi,offset szFmtWeek
								add edi,eax
								invoke lstrcat,addr @szBuf,edi
							.endif
							inc dwSize
						.endw
					.endif
					invoke GetDlgItem,hWnd,IDC_TEXT
					mov ebx,eax
					invoke SetWindowText,ebx,addr @szBuf
				.endif
			.endif
		.elseif eax==WM_ICON
			.if wParam==0
				.if lParam==WM_LBUTTONDBLCLK
					invoke ShowWindow,hWinMain,SW_RESTORE
					invoke GetDlgItem,hWnd,IDC_NEW
					invoke SetFocus,eax
				.endif
			.endif
		.elseif eax==WM_HOTKEY
			.if bShow
				invoke ShowWindow,hWinMain,SW_RESTORE
				invoke GetDlgItem,hWnd,IDC_NEW
				invoke SetFocus,eax
			.else
				invoke ShowWindow,hWinMain,SW_HIDE
			.endif
			not bShow
		.else	;������Ϣ
			mov	eax,FALSE
			ret
		.endif
	@@:mov	eax,TRUE
		ret

_ProcDlgMain	endp
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
start:
		invoke FindWindow,NULL,offset szCaption
		.if	eax
			invoke ShowWindow,eax,SW_RESTORE
			invoke GetDlgItem,hWinMain,IDC_NEW
			invoke SetFocus,eax
			invoke	ExitProcess,NULL
		.endif
		invoke	GetModuleHandle,NULL
		mov	hInstance,eax
		invoke _AutoRun
		invoke	DialogBoxParam,hInstance,IDD_MAIN,NULL,offset _ProcDlgMain,NULL
		invoke	ExitProcess,NULL
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		end	start
