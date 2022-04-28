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

    self->pHandle_Table = (PCHAR)*(PULONG)((PCHAR)PsLookupProcessByProcessId + PSLOOKUPPROCESS_PSP_CID_TABLE_OFFSET_WIN7);
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

StatusCode PspCidTableTraversal_RecursiveTraversal(PspCidTableTraversal *self, PCHAR pCurrentTable)
{
    if (((ULONG)pCurrentTable & 3) == 0)
    {
        // for every 8B, untill all 4KB/8B items
        for (ULONG i = 0; i < 4 * 1024 / 8; i++)
        {
            // if is EPROCESS
            PVOID pCurrentObject = ((PCHAR)((ULONG)pCurrentTable & ~0x07) + i * 8);
            UCHAR currentObjectType = *(PUCHAR)((PCHAR)pCurrentObject - 0x18 + 0xC);
            if (currentObjectType == (UCHAR)7)
            {
                USHORT length;
                PCHAR buff;
                ProcessInfoPackager infoPackager;

                StatusCode tmp = ProcessInfoPackager_Init(&infoPackager, (PEPROCESS)((PCHAR)((ULONG)pCurrentTable & ~0x07) + i * 8));
                if (tmp != SUCCESS)
                {
                    ProcessInfoPackager_ClearAll(&infoPackager);
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
                KdPrint(("ProcessID:%u, ProcessPID:%u, ProcessName:%s, Process:%ws\n", infoPackager.Info.pid, infoPackager.Info.parentPid, infoPackager.Info.imageName, infoPackager.Info.path));
                ProcessInfoPackager_ClearAll(&infoPackager);
            }
            // if is ETHREAD
            else if (currentObjectType == (UCHAR)8)
            {
                USHORT length;
                PCHAR buff;
                ThreadInfoPackager infoPackager;

                StatusCode tmp = ThreadInfoPackager_Init(&infoPackager, (PETHREAD)((PCHAR)((ULONG)pCurrentTable & ~0x07) + i * 8));
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
                KdPrint(("ThreadID:%u, ThreadPID:%u\n", infoPackager.Info.tid, infoPackager.Info.parentPid));
                ThreadInfoPackager_ClearAll(&infoPackager);
            }
            else
            {
                KdPrint(("Object Type UNKNOWN, address is %u\n", (ULONG)pCurrentObject));
            }
        }

        return SUCCESS;
    }
    else
    {
        for (ULONG i = 0; i < 4 * 1024 / 4; i++)
        {
            if (pCurrentTable + i * 4 == NULL)
                continue;
            StatusCode tmp = PspCidTableTraversal_RecursiveTraversal(self, (PCHAR)((ULONG)pCurrentTable & ~0x7) + i * 4);
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