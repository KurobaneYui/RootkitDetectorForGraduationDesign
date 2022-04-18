#include "MemAllocatorRootkit.h"

_StatusCode MemoryAllocator::Init()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    Status = NORMAL;

    PrimaryBuffIndex = (PCHAR *)ExAllocatePoolWithTag(
        PagedPool,
        NUM_OF_BUFF * sizeof(PCHAR),
        ROOTKIT_DETECTOR_TAG);
    if (PrimaryBuffIndex == nullptr)
    {
        Status = DESTROYED;
        return NO_MEMORY;
    }

    BuffAlreadyUsedSpace = (PULONG)ExAllocatePoolWithTag(
        PagedPool,
        NUM_OF_BUFF * sizeof(ULONG),
        ROOTKIT_DETECTOR_TAG);

    if (BuffAlreadyUsedSpace == nullptr)
    {
        ExFreePoolWithTag(PrimaryBuffIndex, ROOTKIT_DETECTOR_TAG);
        Status = DESTROYED;
        return NO_MEMORY;
    }

    for (ULONG i = 0; i < NUM_OF_BUFF; i++)
    {
        BuffAlreadyUsedSpace[i] = 0;
        PrimaryBuffIndex[i] = nullptr;
    }

    _StatusCode tmp = NewBuffBlock();
    if (tmp != SUCCESS)
    {
        ExFreePoolWithTag(PrimaryBuffIndex, ROOTKIT_DETECTOR_TAG);
        ExFreePoolWithTag(BuffAlreadyUsedSpace, ROOTKIT_DETECTOR_TAG);
        Status = DESTROYED;
        return tmp;
    }

    return SUCCESS;
}

_StatusCode MemoryAllocator::CleanAll()
{
    if (Status == DESTROYED)
        return SUCCESS;

    FreeAllBuff();
    ExFreePoolWithTag(PrimaryBuffIndex, ROOTKIT_DETECTOR_TAG);
    ExFreePoolWithTag(BuffAlreadyUsedSpace, ROOTKIT_DETECTOR_TAG);
    Status = DESTROYED;
    return SUCCESS;
}

_StatusCode MemoryAllocator::GetBuff(PCHAR &buff, const ULONG needLength)
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    if (buff == nullptr || needLength > CAPICITY_OF_ONE_BUFF)
        return OUT_OF_RANGE;

    if (WriteIndex.offset == CAPICITY_OF_ONE_BUFF
        || CAPICITY_OF_ONE_BUFF - WriteIndex.offset < needLength)
    {
        if (PrimaryBuffIndex[++WriteIndex.index] == nullptr)
        {
            _StatusCode tmp = NewBuffBlock();

            if (tmp != SUCCESS)
                WriteIndex.index--;
            return NO_MEMORY;
        }
        WriteIndex.offset = 0;
    }

    buff = PrimaryBuffIndex[WriteIndex.index] + WriteIndex.offset;
    WriteIndex.offset += needLength;
    BuffAlreadyUsedSpace[WriteIndex.index] += needLength;

    return SUCCESS;
}

_StatusCode MemoryAllocator::ReadBuff(PCHAR buff, const ULONG requireLength, ULONG &realReadLength)
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    if (buff == nullptr)
        return OUT_OF_RANGE;

    if (PrimaryBuffIndex[ReadIndex.index] == nullptr)
    {
        realReadLength = 0;
        return SUCCESS;
    }

    ULONG accumulate = 0;
    ULONG kernelLeftLength = 0;
    ULONG userLeftLength = requireLength;

    while (PrimaryBuffIndex[ReadIndex.index] != nullptr)
    {
        kernelLeftLength = BuffAlreadyUsedSpace[ReadIndex.index] - ReadIndex.offset;
        if (kernelLeftLength <= userLeftLength)
        {
            memcpy(buff + accumulate, PrimaryBuffIndex[ReadIndex.index] + ReadIndex.offset, kernelLeftLength);
            accumulate += kernelLeftLength;
            userLeftLength -= kernelLeftLength;
            ReadIndex.index++;
            ReadIndex.offset = 0;
        }
        else
        {
            memcpy(buff + accumulate, PrimaryBuffIndex[ReadIndex.index] + ReadIndex.offset, userLeftLength);
            accumulate += userLeftLength;
            userLeftLength = 0;
            ReadIndex.offset += userLeftLength;
            if (ReadIndex.offset == BuffAlreadyUsedSpace[ReadIndex.index])
            {
                ReadIndex.index++;
                ReadIndex.offset = 0;
            }
            break;
        }
    }

    realReadLength = accumulate;

    return SUCCESS;
}

_StatusCode MemoryAllocator::ResetBuff()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    WriteIndex = { 0,0 };
    ReadIndex = { 0,0 };
    BuffAlreadyUsedSpace[0] = 0;
    for (int i = 1; i < NUM_OF_BUFF; i++)
    {
        if (PrimaryBuffIndex[i] != nullptr)
            ExFreePoolWithTag(PrimaryBuffIndex[i], ROOTKIT_DETECTOR_TAG);
        BuffAlreadyUsedSpace[i] = 0;
    }
    return SUCCESS;
}

_StatusCode MemoryAllocator::FreeAllBuff()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    for (ULONG i = 0; i < NUM_OF_BUFF; i++)
    {
        BuffAlreadyUsedSpace[i] = 0;
        if (PrimaryBuffIndex[i] != nullptr)
        {
            ExFreePoolWithTag(PrimaryBuffIndex[i], ROOTKIT_DETECTOR_TAG);
            PrimaryBuffIndex[i] = nullptr;
        }
    }

    Status = NORMAL;
    ReadIndex = { 0,0 };
    WriteIndex = { 0,0 };

    return SUCCESS;
}

_StatusCode MemoryAllocator::NewBuffBlock()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    for (int i = 0; i < NUM_OF_BUFF; i++)
    {
        if (PrimaryBuffIndex[i] == nullptr)
        {
            PrimaryBuffIndex[i] =
                (PCHAR)ExAllocatePoolWithTag(
                    PagedPool,
                    CAPICITY_OF_ONE_BUFF,
                    ROOTKIT_DETECTOR_TAG
                );
            if (PrimaryBuffIndex[i] == nullptr)
                return NO_MEMORY;

            return SUCCESS;
        }

        if (i == NUM_OF_BUFF - 1)
            return NO_MEMORY;
    }

    return UNKNOWN;
}