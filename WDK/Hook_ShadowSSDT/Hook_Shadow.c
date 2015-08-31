#include "ntddk.h"
#include "winstring.h"

#define		IOCTL_INIT   (ULONG) CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_GETAWINDOW	(ULONG) CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_DECIDEOK		(ULONG) CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_DECIDENO		(ULONG) CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
//*************************************************************************
//��ؽṹ
//*************************************************************************
//������Ϣ
typedef struct window_info
{
	TCHAR window_name[64];
	TCHAR class_name[64];
	int height;
	int width;
}WINDOW_INFO,*PWINDOW_INFO;
WINDOW_INFO wndinfo;
//��ʼ����Ϣ
struct INITINFO
{
	PWINDOW_INFO p;
}init_info;

typedef struct _SERVICE_DESCRIPTOR_TABLE
{
	PVOID    ServiceTableBase;
	PULONG   ServiceCounterTableBase;
	ULONG    NumberOfServices;
	PULONG    ParamTableBase;
}SERVICE_DESCRIPTOR_TABLE, *PSERVICE_DESCRIPTOR_TABLE;

int (*old_fnCreateWindow)(int dwExStyle,PWINUNICODESTRING UnsafeClassName,PWINUNICODESTRING UnsafeWindowName,int dwUnknown1,int dwStyle,LONG x, LONG y,LONG nWidth,LONG nHeight,int hWndParent,int hMenu,int hInstance,int lpParam,int dwShowMode,int dwUnknown2);

//*************************************************************************
//ȫ�ֱ���
//*************************************************************************
ULONG *lpfnBaseInTable,fnOldAddress;
int bDecide;
int fnIndex;
KEVENT event_read,event_decide;
KEVENT event_wnd;//����ͬ����ͬ�߳�֮��Ĵ�������
				//��ͬ��������ԭ���� �¼��ܹ������Ľ���cpuʱ��
int bPass;
int window_handle;
KSPIN_LOCK kl;
int pShadow;
PSERVICE_DESCRIPTOR_TABLE KeServiceDescriptorTableShadow;
__declspec(dllimport)  SERVICE_DESCRIPTOR_TABLE KeServiceDescriptorTable; 
__declspec(dllimport) _stdcall KeAddSystemServiceTable(PVOID, PVOID, PVOID, PVOID, PVOID);



//HOOK��������
int _stdcall my_NtUserCreateWindowEx(int dwExStyle,PWINUNICODESTRING ClassName,PWINUNICODESTRING WindowName,int dwUnknown1,int dwStyle,LONG x, LONG y,LONG nWidth, LONG nHeight,int hWndParent,int hMenu,int hInstance,int lpParam,int dwShowMode,int dwUnknown2)
 { 
	//������ȴ�����˵���������߳����ڵ��ñ�����
	KeWaitForSingleObject(&event_wnd,Executive,KernelMode,FALSE,NULL);
	 //KeAcquireSpinLock(&kl,&irql);

	KdPrint(("There is a window being created!\n"));
	if(hWndParent==NULL)
	{
		_try
		{
		//���ݴ�����Ϣ
		KdPrint(("WindowName:%s\n",WindowName->buf));
		//_asm int 3
		CopyWinUnicodeString(wndinfo.window_name,WindowName->buf,64);
		CopyWinUnicodeString(wndinfo.class_name,ClassName->buf,64);
		wndinfo.height=nHeight;
		wndinfo.width=nWidth;
		}
		_except(STATUS_ACCESS_VIOLATION)
		{
			window_handle=old_fnCreateWindow(dwExStyle,ClassName,WindowName,dwUnknown1,dwStyle,x,y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam,dwShowMode,dwUnknown2); 
			goto exit;
		}
		//֪ͨ�жϳ����д��ڴ���
		KeSetEvent(&event_read,0,1);
		//�ȴ��ж�
		KeWaitForSingleObject(&event_decide,Executive,KernelMode,FALSE,NULL);
		if(bDecide)
		{
			window_handle=old_fnCreateWindow(dwExStyle,ClassName,WindowName,dwUnknown1,dwStyle,x,y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam,dwShowMode,dwUnknown2); 
		}
		else
		{
			window_handle=0;
		}
		KdPrint(("The window handle is %x\n",window_handle));
	}
exit:	
	KeSetEvent(&event_wnd,0,1);//�뿪ʱ��Ҫ���������¼����ź�
	//KeReleaseSpinLock(&kl,&irql);
	return window_handle;
 } 

//*************************************************************************
//ʵ��HOOK��غ���
//*************************************************************************
//�õ�ShadowSSDT�ĵ�ַ(�ѹ������뷨)
unsigned int getAddressOfShadowTable()
{
    unsigned int i;
    unsigned char *p;
    unsigned int intatbyte;

    p = (unsigned char*) KeAddSystemServiceTable;

    for(i = 0; i < 4096; i++, p++)
    {
        __try
        {
            intatbyte = *(unsigned int*)p;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return 0;
        }

        if(MmIsAddressValid((PVOID)intatbyte))
        {
            if(memcmp((PVOID)intatbyte, &KeServiceDescriptorTable, 16) == 0)
            {
                if((PVOID)intatbyte == &KeServiceDescriptorTable)
                {
                    continue;
                }

                return intatbyte;
            }
        }
    }

    return 0;
}
//��PETHREAD->ServiceTable�ֶη���
ULONG GetShadowAddrFromEthread()
{
	int i;
	PETHREAD pEthread=PsGetCurrentThread();
	KdPrint(("ETHREAD�ṹ��ַ��%x\n",pEthread));
	for(i=0;i<256*4;i++)
	{
		if(*(pEthread+i)==KeServiceDescriptorTableShadow)
		{
			KdPrint(("ServiceTable�ֶε�ƫ�ƣ�%d\n",i));
			return *(pEthread+i);
		}
	}
	//_asm int 3
	return 5555;//0xdcΪServiceTable�ֶε�ƫ�ƣ�ETHREAD�ṹ��͸����
}

ULONG getShadowTable()
{
    KeServiceDescriptorTableShadow =getAddressOfShadowTable();
	
	KdPrint(("���������룺%x\n",KeServiceDescriptorTableShadow));
	
	KdPrint(("ServiceTable�ֶΣ�%x\n",GetShadowAddrFromEthread()));

    if(KeServiceDescriptorTableShadow == NULL)
    {
        DbgPrint("Couldnt find shadowtable!\n");
        
        return FALSE;
    }
    else
    {
        DbgPrint("Shadowtable has been found!\n");
        
        DbgPrint("Shadowtable entries: %d\n", KeServiceDescriptorTableShadow[1].NumberOfServices);
        return TRUE;
    }
} 

//���ݲ���ϵͳ��ȷ�����庯���ķ���� 
VOID InitCallNumber()
{
	ULONG majorVersion, minorVersion;
	PsGetVersion( &majorVersion, &minorVersion, NULL, NULL );
    if ( majorVersion == 5 && minorVersion == 2 )
    {
	  DbgPrint("comint32: Running on Windows 2003\n");

	}
	else if ( majorVersion == 5 && minorVersion == 1 )
	{
	  DbgPrint("comint32: Running on Windows XP\n");
      fnIndex=343;
	}
	else if ( majorVersion == 5 && minorVersion == 0 )
	{
	  DbgPrint("comint32: Running on Windows 2000\n");
	}
}

// xp��NtUserCreateWindowEx������Ϊ343
//param1��������shadow�е�������  param2���º����ĵ�ַ  
//����ֵ���ɺ����ĵ�ַ
ULONG Hook_ShadowSSDT(ULONG fnID,PVOID fnNew)
{
	PMDL pShadow;
	int size;
	int base;
	int i=0;
	
	_asm
	  {
		CLI                    //dissable interrupt
		MOV    EAX, CR0        //move CR0 register into EAX
		AND EAX, NOT 10000H //disable WP bit 
		MOV    CR0, EAX        //write register back
	  }
	
	getShadowTable();
	size=KeServiceDescriptorTableShadow[1].NumberOfServices*4;
	base=KeServiceDescriptorTableShadow[1].ServiceTableBase;
	
	KdPrint(("ShadowSSDT��ַΪ��%x\n",base));

	lpfnBaseInTable = (ULONG *)(base+343*4);
	fnOldAddress=(ULONG)*lpfnBaseInTable;
	
	KdPrint(("NuUserCreateWindowExԭ������ַ��%x\n",fnOldAddress));
	KdPrint(("NuUserCreateWindowEx�º�����ַ��%x\n",fnNew));
	
	InterlockedExchange(lpfnBaseInTable,fnNew);
	
	_asm 
	  {
		MOV    EAX, CR0        //move CR0 register into EAX
		OR     EAX, 10000H        //enable WP bit     
		MOV    CR0, EAX        //write register back        
		STI                    //enable interrupt
	  }
	
	KdPrint(("Hook �ɹ�\n"));
	return fnOldAddress;
}

//*************************************************************************
//�������
//*************************************************************************
VOID DriverUnload(PDRIVER_OBJECT driver)
{
	DbgPrint("Driver Unload!\n");
	//�ָ�hook
	InterlockedExchange(lpfnBaseInTable,fnOldAddress);
}

//Ĭ�ϵ�IRP��������
//
NTSTATUS fnIrpDefault(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PIO_STACK_LOCATION pSP=IoGetCurrentIrpStackLocation(Irp);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	if(DeviceObject->Flags & DO_DIRECT_IO)
	{
		KdPrint(("Flags:DO_DIRECT_IO\n"));
	}
	if(DeviceObject->Flags & DO_BUFFERED_IO)
	{
		KdPrint(("Flags:DO_BUFFERED_IO\n"));
	}
	switch (pSP->MajorFunction)
	{
	case  IRP_MJ_READ:
		KdPrint(("MJ:IRP_MJ_READ\n"));
		break;
	case IRP_MJ_CREATE:
		KdPrint(("MJ:IRP_MJ_CREATE\n"));
		break;	
	}
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

//IRPд����������
NTSTATUS fnIrpWrite(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pSP = IoGetCurrentIrpStackLocation(Irp);
	PUCHAR pBuffer = NULL;
	UCHAR  port=0x64;
	UCHAR code;
	
	KdPrint(("д����\n"));

	pBuffer = Irp->AssociatedIrp.SystemBuffer;
	KdPrint(("IRP���뻺����: %x\n",pBuffer));

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 1;

	IoCompleteRequest(Irp,IO_NO_INCREMENT);

	return status;
}

NTSTATUS fnIrpDeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pSP = IoGetCurrentIrpStackLocation(Irp);
	PUCHAR pBuffer = NULL;
	ULONG code=pSP->Parameters.DeviceIoControl.IoControlCode;
	pBuffer = Irp->AssociatedIrp.SystemBuffer;
	KdPrint(("��������\n"));
	KdPrint(("IRP���뻺����: %x\n",pBuffer));

	switch(code)
	{
		case IOCTL_INIT:
		KeSetEvent(&event_wnd,0,1);
		//Hook Shadow
		KdPrint(("Hook Shadow ...\n",pBuffer));
		old_fnCreateWindow=Hook_ShadowSSDT(fnIndex,my_NtUserCreateWindowEx);
		RtlCopyMemory(&init_info.p,pBuffer,4);

		//KdPrint(("����Ļ�������ַ��%x\n",init_info.p));
		//_asm int 3
		break;
		case IOCTL_GETAWINDOW:
		KeWaitForSingleObject(&event_read,UserRequest,KernelMode,FALSE,NULL);
		//_asm  int 3
		RtlCopyMemory(init_info.p,&wndinfo,sizeof(wndinfo));
		KdPrint(("Got a window!"));
		break;
		case IOCTL_DECIDEOK://������
		bDecide=1;
		KeSetEvent(&event_decide,0,1);
		break;
		case IOCTL_DECIDENO://��������
		bDecide=0;
		KeSetEvent(&event_decide,0,1);
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 1;

	IoCompleteRequest(Irp,IO_NO_INCREMENT);

	return status;
}

//IRP������������
NTSTATUS fnIrpRead(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pSP = IoGetCurrentIrpStackLocation(Irp);
	PUCHAR pBuffer = NULL;
	UCHAR code;


	KdPrint(("������\n"));

	pBuffer = Irp->AssociatedIrp.SystemBuffer;
	KdPrint(("IRP���������: %x\n",pBuffer));
	
	KeWaitForSingleObject(&event_read,Executive,KernelMode,0,0);
	KdPrint(("�д��ڴ��������	��%x\n",window_handle));

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 1;

	IoCompleteRequest(Irp,IO_NO_INCREMENT);

	return status;
}

//������ڵ�
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING  RegistryPath)
{
	PDEVICE_OBJECT DeviceObject;
	UNICODE_STRING DeviceName,LinkName;
	NTSTATUS status;
	int i;
	
	//�ж�����ϵͳ�汾
	InitCallNumber();
	
	//��ʼ���¼�����
	KeInitializeEvent(&event_read,SynchronizationEvent,0);
	KeInitializeEvent(&event_decide,SynchronizationEvent,0);
	KeInitializeEvent(&event_wnd,SynchronizationEvent,0);

	//KeInitializeSpinLock(&kl);

	//��ʼ���ַ���
	RtlInitUnicodeString(&DeviceName,L"\\Device\\FilterWindow");
	RtlInitUnicodeString(&LinkName,L"\\??\\FilterWindow");
	
	//ָ��IRP��������
	for(i=0;i<=IRP_MJ_MAXIMUM_FUNCTION;i++)
		DriverObject->MajorFunction[i]=fnIrpDefault;
	DriverObject->MajorFunction[IRP_MJ_WRITE]=fnIrpWrite;
	DriverObject->MajorFunction[IRP_MJ_READ]=fnIrpRead;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]=fnIrpDeviceControl;
	//�����豸
	status=IoCreateDevice(DriverObject,0, &DeviceName,FILE_DEVICE_UNKNOWN,0,0,&DeviceObject);

	if(!NT_SUCCESS(status))
	{
		DbgPrint("�豸����ʧ��\n");
		return status;
	}
	
	//�����豸�Ķ�д��ʽ
	DeviceObject->Flags |=DO_BUFFERED_IO;
	//������������
	status=IoCreateSymbolicLink(&LinkName,&DeviceName);

	DeviceObject->Flags&=~DO_DEVICE_INITIALIZING;//��ʼ����ɱ��

	if(!NT_SUCCESS(status))
	{
		DbgPrint("�������Ӵ���ʧ��\n");
		return status;
	}
	DbgPrint("�������Ӵ����ɹ�\n");
	
	return STATUS_SUCCESS;
}






