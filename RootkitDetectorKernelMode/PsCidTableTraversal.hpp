#pragma once

#include <ProcessAndThreadInterface.h>

#define PSP_CID_TABLE 0x82b59d94

//_OBJECT_TYPE(*const ObGetObjectType) (PVOID pointer) = (_OBJECT_TYPE(*)(PVOID pointer))0x82cb0559;

class PspCidTableTraversal : public Detector
{
private:
    enum { NORMAL, DESTROYED } Status;
    PCHAR pHandle_Table;
    MemoryAllocator * pMemoryAllocator;

    _StatusCode RecursiveTraversal(PCHAR pCurrentTable);

public:
    PspCidTableTraversal() = default;
    ~PspCidTableTraversal() = default;
    _StatusCode Init(MemoryAllocator * pAllocator);
    _StatusCode ClearAll();
    _StatusCode Snapshot();
    _StatusCode GetInfos(
        PCHAR buffer,
        const ULONG bufferLength,
        ULONG &realReadLength);
    _StatusCode FreeupSnapshot();
};

_StatusCode PspCidTableTraversal::Init(MemoryAllocator * pAllocator)
{
    Status = DESTROYED;
    if (pAllocator == nullptr)
        return OUT_OF_RANGE;

    pHandle_Table = (PCHAR)PSP_CID_TABLE;
    pMemoryAllocator = pAllocator;

    Status = NORMAL;

    return SUCCESS;
}

_StatusCode PspCidTableTraversal::ClearAll()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    pMemoryAllocator->ResetBuff();

    Status = DESTROYED;
    return SUCCESS;
}

_StatusCode PspCidTableTraversal::RecursiveTraversal(PCHAR pCurrentTable)
{
    if (((ULONG)pCurrentTable & 3) == 0)
    {
        // for every 8B, untill all 4KB/8B items
        for (ULONG i = 0; i < 4 * 1024 / 8; i++)
        {
            // if is EPROCESS
            //if (*PsProcessType == ObGetObjectType((PVOID)((PCHAR)((ULONG)pCurrentTable & ~0x07) + i * 8)))
            if (true)
            {
                USHORT length;
                PCHAR buff;
                ProcessInfoPackager infoPackager;
                infoPackager.Init((PEPROCESS)((PCHAR)((ULONG)pCurrentTable & ~0x07) + i * 8));
                infoPackager.GetInfoLength(length);
                pMemoryAllocator->GetBuff(buff, length);
                infoPackager.WriteToBuff(buff);
                infoPackager.ClearAll();
            }
            else // if is ETHREAD
            {
                USHORT length;
                PCHAR buff;
                ThreadInfoPackager infoPackager;
                infoPackager.Init((PETHREAD)((PCHAR)((ULONG)pCurrentTable & ~0x07) + i * 8));
                infoPackager.GetInfoLength(length);
                pMemoryAllocator->GetBuff(buff, length);
                infoPackager.WriteToBuff(buff);
                infoPackager.ClearAll();
            }
        }
    }
    else
    {
        for (ULONG i = 0; i < 4 * 1024 / 4; i++)
        {
            if (pCurrentTable + i * 4 == nullptr)
                continue;
            RecursiveTraversal((PCHAR)((ULONG)pCurrentTable & ~0x7) + i * 4);
        }
    }
}

_StatusCode PspCidTableTraversal::Snapshot()
{
    // TODO: Haven't finished StatusCode check
    PCHAR pTableCode = (PCHAR)*(PLONGLONG)pHandle_Table;
    pMemoryAllocator->ResetBuff();
    RecursiveTraversal(pTableCode);

    return SUCCESS;
}

_StatusCode PspCidTableTraversal::GetInfos(
    PCHAR buffer,
    const ULONG bufferLength,
    ULONG &realReadLength)
{
    // TODO: Haven't finished StatusCode check

    pMemoryAllocator->ReadBuff(buffer, bufferLength, realReadLength);

    return SUCCESS;
}

_StatusCode PspCidTableTraversal::FreeupSnapshot()
{
    // TODO: Haven't finished StatusCode check

    pMemoryAllocator->ResetBuff();

    return SUCCESS;
}