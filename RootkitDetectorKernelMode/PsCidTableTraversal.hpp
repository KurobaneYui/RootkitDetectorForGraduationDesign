#pragma once

#include <ProcessAndThreadInterface.h>

#define PSP_CID_TABLE 0x82b59d94

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
            UNICODE_STRING funcName;
            RtlInitUnicodeString(&funcName, L"ObGetObjectType");
            POBJECT_TYPE(*ObGetObjectType) (PVOID);
            ObGetObjectType = (POBJECT_TYPE(*)(PVOID))MmGetSystemRoutineAddress(&funcName);
            if (ObGetObjectType == nullptr)
                return UNKNOWN;
            int cmp = memcmp(PsProcessType, (PVOID)ObGetObjectType((PVOID)((PCHAR)((ULONG)pCurrentTable & ~0x07) + i * 8)), 0x2E0);
            if (cmp == 0)
            {
                USHORT length;
                PCHAR buff;
                ProcessInfoPackager infoPackager;

                _StatusCode tmp = infoPackager.Init((PEPROCESS)((PCHAR)((ULONG)pCurrentTable & ~0x07) + i * 8));
                if (tmp != SUCCESS)
                {
                    infoPackager.ClearAll();
                    return tmp;
                }
                infoPackager.GetInfoLength(length);
                if (pMemoryAllocator->GetBuff(buff, length) != SUCCESS)
                {
                    infoPackager.ClearAll();
                    return NO_MEMORY;
                }
                tmp = infoPackager.WriteToBuff(buff);
                if (tmp != SUCCESS)
                {
                    infoPackager.ClearAll();
                    return tmp;
                }
                infoPackager.ClearAll();
            }
            else // if is ETHREAD
            {
                USHORT length;
                PCHAR buff;
                ThreadInfoPackager infoPackager;

                _StatusCode tmp = infoPackager.Init((PETHREAD)((PCHAR)((ULONG)pCurrentTable & ~0x07) + i * 8));
                if (tmp != SUCCESS)
                {
                    infoPackager.ClearAll();
                    return tmp;
                }
                infoPackager.GetInfoLength(length);
                if (pMemoryAllocator->GetBuff(buff, length) != SUCCESS)
                {
                    infoPackager.ClearAll();
                    return NO_MEMORY;
                }
                tmp = infoPackager.WriteToBuff(buff);
                if (tmp != SUCCESS)
                {
                    infoPackager.ClearAll();
                    return tmp;
                }
                infoPackager.ClearAll();
            }
        }

        return SUCCESS;
    }
    else
    {
        for (ULONG i = 0; i < 4 * 1024 / 4; i++)
        {
            if (pCurrentTable + i * 4 == nullptr)
                continue;
            _StatusCode tmp = RecursiveTraversal((PCHAR)((ULONG)pCurrentTable & ~0x7) + i * 4);
            if (tmp != SUCCESS)
            {
                return tmp;
            }
        }

        return SUCCESS;
    }
}

_StatusCode PspCidTableTraversal::Snapshot()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    PCHAR pTableCode = (PCHAR)*(PLONGLONG)pHandle_Table;
    pMemoryAllocator->ResetBuff();
    return RecursiveTraversal(pTableCode);
}

_StatusCode PspCidTableTraversal::GetInfos(
    PCHAR buffer,
    const ULONG bufferLength,
    ULONG &realReadLength)
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    if (buffer == nullptr || bufferLength == 0)
        return OUT_OF_RANGE;

    return pMemoryAllocator->ReadBuff(buffer, bufferLength, realReadLength);
}

_StatusCode PspCidTableTraversal::FreeupSnapshot()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    pMemoryAllocator->ResetBuff();

    return SUCCESS;
}