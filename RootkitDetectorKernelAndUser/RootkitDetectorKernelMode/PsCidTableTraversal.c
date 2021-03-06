#include <PsCidTableTraversal.h>

StatusCode PspCidTableTraversal_Init(PspCidTableTraversal *self, MemoryAllocator *pAllocator)
{
    self->Status = DESTROYED;
    if (pAllocator == NULL)
        return OUT_OF_RANGE;

    //UNICODE_STRING funcName;
    //RtlInitUnicodeString(&funcName, L"ObGetObjectType");
    //PVOID ObGetObjectType = MmGetSystemRoutineAddress(&funcName);
    //if (ObGetObjectType == NULL)
    //    return UNKNOWN;

    self->pHandle_Table = (PCHAR)*(PULONG)*(PULONG)((PCHAR)PsLookupProcessByProcessId + PSLOOKUPPROCESS_PSP_CID_TABLE_OFFSET_WIN7);
    //self->pObjectTypeIndexTable = (PCHAR)*(PULONG)((PCHAR)ObGetObjectType + 0xF);
    self->pMemoryAllocator = pAllocator;

    self->Status = NORMAL;

    return SUCCESS;
}

StatusCode PspCidTableTraversal_ClearAll(PspCidTableTraversal *self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    MemoryAllocator_ResetBuff(self->pMemoryAllocator);

    self->Status = DESTROYED;
    return SUCCESS;
}

// 递归遍历句柄表
StatusCode PspCidTableTraversal_RecursiveTraversal(PspCidTableTraversal *self, PCHAR pCurrentTable)
{
    // 如果指针低三位是000，则为末级索引，指针指向某个4k大小的句柄表
    if (((ULONG)pCurrentTable & 3) == 0)
    {
        // 搜索句柄表的每一个句柄项（长度为8 Bytes）
        // for every 8 Bytes, untill all 4KB/8B items
        USHORT i = 0;
        for (i = 0; i < 4 * 1024 / 8; i++)
        {
            // 句柄项第一个成员是指向句柄对应进程或线程结构体的指针
            PVOID pCurrentObject = (PVOID)*(PULONG)(pCurrentTable + i * 8);
            if (pCurrentObject == NULL)
                continue;
            // 指针第二位是标志位，需要置0，最高位同理
            pCurrentObject = (PVOID)(((ULONG)pCurrentObject & 0xFFFFFFF8) | 0x80000000);
            // 结构体头部前有OBJECT_HEADER结构体，存有结构体的类型，可用于判断结构体是进程对象还是线程对象
            UCHAR currentObjectType = *(PUCHAR)((PCHAR)pCurrentObject - 0x18 + 0xC);
            // if is EPROCESS
            if (currentObjectType == (UCHAR)7)
            {
                USHORT length;
                PCHAR buff;
                ProcessInfoPackager infoPackager;

                // 完成如下流程：初始化（获取有关信息）->获取信息长度->向内存分配器申请内存->写入内存
                PEPROCESS currentProcess = (PEPROCESS)((ULONG)pCurrentObject & ~0x07);
                StatusCode tmp = ProcessInfoPackager_Init(&infoPackager, currentProcess);
                if (tmp != SUCCESS)
                {
                    ProcessInfoPackager_ClearAll(&infoPackager);
                    if (tmp == OUT_OF_RANGE && ((PCHAR)((ULONG)pCurrentObject & ~0x07) + i * 8) != 0)
                        continue;
                    return tmp;
                }
                ProcessInfoPackager_GetInfoLength(&infoPackager, &length);
                if (MemoryAllocator_GetBuff(self->pMemoryAllocator, &buff, length) != SUCCESS)
                {
                    ProcessInfoPackager_ClearAll(&infoPackager);
                    return NO_MEMORY;
                }
                tmp = ProcessInfoPackager_WriteToBuff(&infoPackager, buff);
                if (tmp != SUCCESS)
                {
                    ProcessInfoPackager_ClearAll(&infoPackager);
                    return tmp;
                }
                // 打印内核信息便于调试
                KdPrint(("ProcessID:%u, ProcessPID:%u, ProcessName:%s, Process:%ws\n", infoPackager.Info.pid, infoPackager.Info.parentPid, infoPackager.Info.imageName, infoPackager.Info.path));
                ProcessInfoPackager_ClearAll(&infoPackager);
            }
            // if is ETHREAD
            else if (currentObjectType == (UCHAR)8)
            {
                USHORT length;
                PCHAR buff;
                ThreadInfoPackager infoPackager;

                // 完成如下流程：初始化（获取有关信息）->获取信息长度->向内存分配器申请内存->写入内存
                PETHREAD currentThread = (PETHREAD)((ULONG)pCurrentObject & ~0x07);
                StatusCode tmp = ThreadInfoPackager_Init(&infoPackager, currentThread);
                if (tmp != SUCCESS)
                {
                    ThreadInfoPackager_ClearAll(&infoPackager);
                    return tmp;
                }
                ThreadInfoPackager_GetInfoLength(&infoPackager, &length);
                if (MemoryAllocator_GetBuff(self->pMemoryAllocator, &buff, length) != SUCCESS)
                {
                    ThreadInfoPackager_ClearAll(&infoPackager);
                    return NO_MEMORY;
                }
                tmp = ThreadInfoPackager_WriteToBuff(&infoPackager, buff);
                if (tmp != SUCCESS)
                {
                    ThreadInfoPackager_ClearAll(&infoPackager);
                    return tmp;
                }
                // 打印内核信息，便于调试
                KdPrint(("ThreadID:%u, ThreadPID:%u\n", infoPackager.Info.tid, infoPackager.Info.parentPid));
                ThreadInfoPackager_ClearAll(&infoPackager);
            }
            // 如果不是进程也不是线程，忽略
            // else
            // {
            //     KdPrint(("Object Type UNKNOWN, address is %u\n", (ULONG)pCurrentObject));
            // }
        }

        return SUCCESS;
    }
    // 如果指针低三位不是000，则为非末级索引，指针指向下一级4k大小的索引表
    else
    {
        // 指针低三位是标志位，置0后才为真实地址
        pCurrentTable = (PCHAR)((ULONG)pCurrentTable & (~3));
        USHORT j = 0;
        // 遍历下级索引表的每一项，递归搜索
        for (j = 0; j < 4 * 1024 / 4; j++)
        {
            PCHAR pNextTable = (PCHAR)*(PULONG)(pCurrentTable + j * 4);
            if (pNextTable == NULL)
                continue;
            StatusCode tmp = PspCidTableTraversal_RecursiveTraversal(self, pNextTable);
            if (tmp != SUCCESS)
            {
                return tmp;
            }
        }

        return SUCCESS;
    }
}

StatusCode PspCidTableTraversal_Snapshot(PspCidTableTraversal *self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    PCHAR pTableCode = (PCHAR) * (PLONG)(self->pHandle_Table);
    MemoryAllocator_ResetBuff(self->pMemoryAllocator);
    return PspCidTableTraversal_RecursiveTraversal(self, pTableCode);
}

StatusCode PspCidTableTraversal_GetInfos(
    PspCidTableTraversal *self,
    PCHAR buffer,
    const ULONG bufferLength,
    PULONG pRealReadLength)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    if (buffer == NULL || pRealReadLength == NULL || bufferLength == 0)
        return OUT_OF_RANGE;

    return MemoryAllocator_ReadBuff(self->pMemoryAllocator, buffer, bufferLength, pRealReadLength);
}

StatusCode PspCidTableTraversal_FreeupSnapshot(PspCidTableTraversal *self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    MemoryAllocator_ResetBuff(self->pMemoryAllocator);

    return SUCCESS;
}