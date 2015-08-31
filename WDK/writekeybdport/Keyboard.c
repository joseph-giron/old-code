#include "ntddk.h"
//����ж������
VOID DriverUnload(PDRIVER_OBJECT driver)
{
	DbgPrint("���̶˿�����ж��ing\n");
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
		KdPrint(("Flags:DO_DIRECT_IO"));
	}
	if(DeviceObject->Flags & DO_BUFFERED_IO)
	{
		KdPrint(("Flags:DO_BUFFERED_IO"));
	}
	switch (pSP->MajorFunction)
	{
	case  IRP_MJ_READ:
		KdPrint(("MJ:IRP_MJ_READ"));
		break;
	case IRP_MJ_CREATE:
		KdPrint(("MJ:IRP_MJ_CREATE"));
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


	KdPrint(("�յ�irp write\n"));

		pBuffer = Irp->AssociatedIrp.SystemBuffer;
		KdPrint(("IRP���뻺����: %x\n",pBuffer));
		code=(UCHAR)(*pBuffer);

		KdPrint(("IRP_MJ_WRITE Write Len: %d\n",pSP->Parameters.Write.Length));
		//����̶˿�д������
		//WRITE_PORT_UCHAR(&port,(UCHAR)(*pBuffer));
		_asm
	{
			push eax
			mov al,0xd2
			out 0x64,al
			mov al,code
			out 0x60,al
			pop eax
		}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 1;

	IoCompleteRequest(Irp,IO_NO_INCREMENT);

	return status;
}

//������ں���
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING  RegistryPath)
{
	UNICODE_STRING DeviceName,LinkName;
	PDEVICE_OBJECT DeviceObject=NULL;
	NTSTATUS status;		int i;

	DriverObject->DriverUnload=DriverUnload;

	//���������Ϣ
	DbgPrint("���̶˿�������װing\n");
	//��ʼ���ַ���
	RtlInitUnicodeString(&DeviceName,L"\\Device\\wriKeyPort");
	RtlInitUnicodeString(&LinkName,L"\\??\\wriKeyPort");
	//ָ��IRP��������
	for(i=0;i<=IRP_MJ_MAXIMUM_FUNCTION;i++)
		DriverObject->MajorFunction[i]=fnIrpDefault;
	DriverObject->MajorFunction[IRP_MJ_WRITE]=fnIrpWrite;
	//�����豸
	status=IoCreateDevice(DriverObject,4, &DeviceName,FILE_DEVICE_UNKNOWN,0,0,&DeviceObject);

	if(!NT_SUCCESS(status))
	{
		DbgPrint("�豸����ʧ��\n");
		return status;
	}

	if(!DeviceObject)return STATUS_UNEXPECTED_IO_ERROR;

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