#include <CommonHeader.h>
#include <MemAllocatorRootkit.h>
#include <ProcessAndThreadInterface.h>
#include <PsActiveProcessTraversal.h>
#include <PsCidTableTraversal.h>

#define DEVICE_NAME L"\\Device\\RootkitDetectorDevice" /*设备名称*/
#define SYMBOLIC_NAME L"\\??\\RootkitDetectorDevice" /*符号链接*/

typedef struct _GlobalVariables
{
    MemoryAllocator GlobalMemoryAllocatorForList;
    MemoryAllocator GlobalMemoryAllocatorForTable;
    PsActiveProcessTraversal psActiveProcessTraversal;
    PspCidTableTraversal psCidTableTraversal;
    CHAR togger;
} GlobalVariables;

void myUnload(_In_ PDRIVER_OBJECT DriverObject)
{
    KdPrint(("Driver Unload\n"));

    if (DriverObject->DeviceObject)
    {
        GlobalVariables *pGlobal = (GlobalVariables*)DriverObject->DeviceObject->DeviceExtension;
        if (pGlobal->togger != -1)
        {
            PsActiveProcessTraversal_ClearAll(&pGlobal->psActiveProcessTraversal);
            PspCidTableTraversal_ClearAll(&pGlobal->psCidTableTraversal);
            MemoryAllocator_CleanAll(&pGlobal->GlobalMemoryAllocatorForList);
            MemoryAllocator_CleanAll(&pGlobal->GlobalMemoryAllocatorForTable);
        }

        UNICODE_STRING symbolicName = RTL_CONSTANT_STRING(SYMBOLIC_NAME);
        IoDeleteSymbolicLink(&symbolicName);

        IoDeleteDevice(DriverObject->DeviceObject);
    }
    return;
}

NTSTATUS DeviceCreate(PDEVICE_OBJECT Device_Object, PIRP pirp)
{
    UNREFERENCED_PARAMETER(Device_Object);

    NTSTATUS status = STATUS_SUCCESS;
    StatusCode myFuncStatus = SUCCESS;

    GlobalVariables *pGlobal = (GlobalVariables*)Device_Object->DeviceExtension;

    pGlobal->togger = 0;
    myFuncStatus = MemoryAllocator_Init(&pGlobal->GlobalMemoryAllocatorForList);
    if (myFuncStatus != SUCCESS)
        status = STATUS_UNSUCCESSFUL;
    myFuncStatus = MemoryAllocator_Init(&pGlobal->GlobalMemoryAllocatorForTable);
    if (myFuncStatus != SUCCESS)
        status = STATUS_UNSUCCESSFUL;

    myFuncStatus = PsActiveProcessTraversal_Init(&pGlobal->psActiveProcessTraversal, &pGlobal->GlobalMemoryAllocatorForList);
    if (myFuncStatus != SUCCESS)
        status = STATUS_UNSUCCESSFUL; 
    myFuncStatus = PspCidTableTraversal_Init(&pGlobal->psCidTableTraversal, &pGlobal->GlobalMemoryAllocatorForTable);
    if (myFuncStatus != SUCCESS)
        status = STATUS_UNSUCCESSFUL;

    pirp->IoStatus.Status = status;

    pirp->IoStatus.Information = 0;

    IoCompleteRequest(pirp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS DeviceCleanup(PDEVICE_OBJECT Device_Object, PIRP pirp)
{
    UNREFERENCED_PARAMETER(Device_Object);

    NTSTATUS status = STATUS_SUCCESS;
    StatusCode myFuncStatus = SUCCESS;

    GlobalVariables *pGlobal = (GlobalVariables*)Device_Object->DeviceExtension;

    myFuncStatus = PsActiveProcessTraversal_FreeupSnapshot(&pGlobal->psActiveProcessTraversal);
    if (myFuncStatus != SUCCESS)
        status = STATUS_UNSUCCESSFUL;
    myFuncStatus = PspCidTableTraversal_FreeupSnapshot(&pGlobal->psCidTableTraversal);
    if (myFuncStatus != SUCCESS)
        status = STATUS_UNSUCCESSFUL;

    pirp->IoStatus.Status = status;

    pirp->IoStatus.Information = 0;

    IoCompleteRequest(pirp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS DeviceClose(PDEVICE_OBJECT Device_Object, PIRP pirp)
{
    UNREFERENCED_PARAMETER(Device_Object);

    NTSTATUS status = STATUS_SUCCESS;
    StatusCode myFuncStatus = SUCCESS;

    GlobalVariables *pGlobal = (GlobalVariables*)Device_Object->DeviceExtension;

    myFuncStatus = PsActiveProcessTraversal_FreeupSnapshot(&pGlobal->psActiveProcessTraversal);
    if (myFuncStatus != SUCCESS)
        status = STATUS_UNSUCCESSFUL;
    myFuncStatus = PspCidTableTraversal_FreeupSnapshot(&pGlobal->psCidTableTraversal);
    if (myFuncStatus != SUCCESS)
        status = STATUS_UNSUCCESSFUL;
    myFuncStatus = MemoryAllocator_FreeAllBuff(&pGlobal->GlobalMemoryAllocatorForList);
    if (myFuncStatus != SUCCESS)
        status = STATUS_UNSUCCESSFUL;
    myFuncStatus = MemoryAllocator_FreeAllBuff(&pGlobal->GlobalMemoryAllocatorForTable);
    if (myFuncStatus != SUCCESS)
        status = STATUS_UNSUCCESSFUL;

    pirp->IoStatus.Status = status;

    pirp->IoStatus.Information = 0;

    IoCompleteRequest(pirp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS DeviceRead(PDEVICE_OBJECT Device_Object, PIRP pirp)
{
    UNREFERENCED_PARAMETER(Device_Object);

    NTSTATUS status = STATUS_SUCCESS;
    StatusCode myFuncStatus = SUCCESS;

    GlobalVariables *pGlobal = (GlobalVariables*)Device_Object->DeviceExtension;

    PIO_STACK_LOCATION pstack = IoGetCurrentIrpStackLocation(pirp);

    ULONG readsize = pstack->Parameters.Read.Length;

    PCHAR readbuffer = (PCHAR)(pirp->AssociatedIrp.SystemBuffer);

    ULONG realLength;
    RtlZeroMemory(readbuffer, readsize);
    if (pGlobal->togger == 0)
        myFuncStatus = PsActiveProcessTraversal_GetInfos(&pGlobal->psActiveProcessTraversal, readbuffer, readsize, &realLength);
    else
        myFuncStatus = PspCidTableTraversal_GetInfos(&pGlobal->psCidTableTraversal, readbuffer, readsize, &realLength);
    if (myFuncStatus != SUCCESS)
        status = STATUS_UNSUCCESSFUL;

    pirp->IoStatus.Status = status;

    pirp->IoStatus.Information = realLength;

    IoCompleteRequest(pirp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS DeviceControl(PDEVICE_OBJECT Device_Object, PIRP pirp)
{
    UNREFERENCED_PARAMETER(Device_Object);

    NTSTATUS status = STATUS_SUCCESS;
    StatusCode myFuncStatus = SUCCESS;

    GlobalVariables *pGlobal = (GlobalVariables*)Device_Object->DeviceExtension;

    PIO_STACK_LOCATION pstack = IoGetCurrentIrpStackLocation(pirp);

    ULONG iocode = pstack->Parameters.DeviceIoControl.IoControlCode;
    ULONG information = 0;

    switch (iocode)
    {
    case IOCTL_SNAPSHOT:
        KdPrint(("Togger Device Control to SNAPSHOT!\n"));

        if (pGlobal->togger == 0)
            myFuncStatus = PsActiveProcessTraversal_Snapshot(&pGlobal->psActiveProcessTraversal);
        else
            myFuncStatus = PspCidTableTraversal_Snapshot(&pGlobal->psCidTableTraversal);

        break;
    case IOCTL_SWITCH:
        KdPrint(("Togger Device Control to SWITCH!\n"));
        ((GlobalVariables*)Device_Object->DeviceExtension)->togger = ((GlobalVariables*)Device_Object->DeviceExtension)->togger == 0 ? 1 : 0;
        if (pGlobal->togger == 1)
        {
            myFuncStatus = PsActiveProcessTraversal_FreeupSnapshot(&pGlobal->psActiveProcessTraversal);
            KdPrint(("\tCid Table Traversal\n"));
        }
        else
        {
            myFuncStatus = PspCidTableTraversal_FreeupSnapshot(&pGlobal->psCidTableTraversal);
            KdPrint(("\tActive Process Link Traversal\n"));
        }
        break;
    default:
        status = STATUS_UNSUCCESSFUL;
        information = 0;
        KdPrint(("Togger Device Control, BUT!!! NOT togger right control code!\n"));
        break;
    }

    if (myFuncStatus != SUCCESS)
        status = STATUS_UNSUCCESSFUL;

    pirp->IoStatus.Status = status;

    pirp->IoStatus.Information = information;

    IoCompleteRequest(pirp, IO_NO_INCREMENT);

    return status;
}

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
    status = IoCreateDevice(DriverObject, sizeof(GlobalVariables), &deviceName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pdevice);
    if (!NT_SUCCESS(status))
    {
        return status;
    }
    ((GlobalVariables*)DriverObject->DeviceObject->DeviceExtension)->togger = -1;
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

    return STATUS_SUCCESS;
}