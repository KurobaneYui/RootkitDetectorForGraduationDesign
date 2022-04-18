#include <ntddk.h>
#include <wdm.h>
#include <CommonHeader.h>
#include <MemAllocatorRootkit.h>
#include <ProcessAndThreadInterface.h>
#include <PsActiveProcessTraversal.hpp>
#include <PsCidTableTraversal.hpp>

#define DEVICE_NAME L"\\Device\\RootkitDetectorDevice" /*设备名称*/
#define SYMBOLIC_NAME L"\\??\\RootkitDetectorDevice" /*符号链接*/

MemoryAllocator GlobalMemoryAllocator;
Detector * pDetector;

void* __cdecl operator new(size_t size)
{
    return ExAllocatePoolWithTag(PagedPool, size, ROOTKIT_DETECTOR_TAG);
}

void __cdecl operator delete(void*p, size_t size)
{
    UNREFERENCED_PARAMETER(size);
    if (p != NULL)
    {
        ExFreePoolWithTag(p, ROOTKIT_DETECTOR_TAG);
    }
}

void myUnload(_In_ PDRIVER_OBJECT DriverObject)
{
    DbgPrint("Driver Unload\n");

    if (DriverObject->DeviceObject)
    {
        IoDeleteDevice(DriverObject->DeviceObject);

        UNICODE_STRING symbolicName = RTL_CONSTANT_STRING(SYMBOLIC_NAME);

        IoDeleteSymbolicLink(&symbolicName);

        if (pDetector != nullptr)
        {
            pDetector->ClearAll();
            delete pDetector;
        }
        GlobalMemoryAllocator.CleanAll();
    }
    return;
}

NTSTATUS DeviceCreate(PDEVICE_OBJECT Device_Object, PIRP pirp)
{
    UNREFERENCED_PARAMETER(Device_Object);

    NTSTATUS status = STATUS_SUCCESS;

    pirp->IoStatus.Status = status;

    pirp->IoStatus.Information = 0;

    IoCompleteRequest(pirp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS DeviceCleanup(PDEVICE_OBJECT Device_Object, PIRP pirp)
{
    UNREFERENCED_PARAMETER(Device_Object);

    NTSTATUS status = STATUS_SUCCESS;

    pDetector->FreeupSnapshot(); // 清理进线程快照的内存

    pirp->IoStatus.Status = status;

    pirp->IoStatus.Information = 0;

    IoCompleteRequest(pirp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS DeviceClose(PDEVICE_OBJECT Device_Object, PIRP pirp)
{
    UNREFERENCED_PARAMETER(Device_Object);

    NTSTATUS status = STATUS_SUCCESS;

    pirp->IoStatus.Status = status;

    pirp->IoStatus.Information = 0;

    IoCompleteRequest(pirp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS DeviceRead(PDEVICE_OBJECT Device_Object, PIRP pirp)
{
    UNREFERENCED_PARAMETER(Device_Object);

    NTSTATUS status = STATUS_SUCCESS;

    PIO_STACK_LOCATION pstack = IoGetCurrentIrpStackLocation(pirp);

    ULONG readsize = pstack->Parameters.Read.Length;

    PCHAR readbuffer = (PCHAR)(pirp->AssociatedIrp.SystemBuffer);

    ULONG realLength;
    RtlZeroMemory(readbuffer, readsize);
    pDetector->GetInfos(readbuffer, readsize, realLength);

    pirp->IoStatus.Status = status;

    pirp->IoStatus.Information = realLength;

    IoCompleteRequest(pirp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS DeviceControl(PDEVICE_OBJECT Device_Object, PIRP pirp)
{
    UNREFERENCED_PARAMETER(Device_Object);

    NTSTATUS status = STATUS_SUCCESS;

    PIO_STACK_LOCATION pstack = IoGetCurrentIrpStackLocation(pirp);

    ULONG iocode = pstack->Parameters.DeviceIoControl.IoControlCode;
    ULONG information = 0;

    switch (iocode)
    {
    case IOCTL_SNAPSHOT:
        DbgPrint("Togger Device Control and togger right control code!\n");

        pDetector->Snapshot();

        break;
    default:
        status = STATUS_UNSUCCESSFUL;
        information = 0;
        DbgPrint("Togger Device Control, BUT!!! NOT togger right control code!\n");
        break;
    }

    pirp->IoStatus.Status = status;

    pirp->IoStatus.Information = information;

    IoCompleteRequest(pirp, IO_NO_INCREMENT);

    return status;
}

extern "C"
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS status = STATUS_SUCCESS;

    // 注册驱动卸载函数
    DriverObject->DriverUnload = myUnload;

    UNICODE_STRING deviceName = RTL_CONSTANT_STRING(DEVICE_NAME);
    UNICODE_STRING symbolicName = RTL_CONSTANT_STRING(SYMBOLIC_NAME);
    PDEVICE_OBJECT pdevice = NULL;

    // 尝试创建设备
    status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pdevice);
    if (!NT_SUCCESS(status))
    {
        return status;
    }
    pdevice->Flags |= DO_BUFFERED_IO;
    // 设备创建成功

    // 创建符号链接
    status = IoCreateSymbolicLink(&symbolicName, &deviceName);
    if (!NT_SUCCESS(status))
    {
        IoDeleteDevice(pdevice); /*创建失败则设备无用，应清理设备*/

        return status;
    }
    // 符号链接创建成功

    // 注册对设备的操作
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DeviceCreate;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = DeviceCleanup;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DeviceClose;
    DriverObject->MajorFunction[IRP_MJ_READ] = DeviceRead;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;

    KdBreakPoint();
    GlobalMemoryAllocator.Init();
    KdBreakPoint();
    pDetector = new PsActiveProcessTraversal();
    //pDetector = new PsCidTableTraversal();
    pDetector->Init(&GlobalMemoryAllocator);

    return STATUS_SUCCESS;
}