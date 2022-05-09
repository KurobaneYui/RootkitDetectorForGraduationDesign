#include <MemAllocatorRootkit.h>

StatusCode MemoryAllocator_Init(MemoryAllocator* self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    self->Status = NORMAL;

    self->PrimaryBuffIndex = (PCHAR *)ExAllocatePoolWithTag(
        PagedPool,
        NUM_OF_BUFF * sizeof(PCHAR),
        ROOTKIT_DETECTOR_TAG);
    if (self->PrimaryBuffIndex == NULL)
    {
        self->Status = DESTROYED;
        return NO_MEMORY;
    }

    self->BuffAlreadyUsedSpace = (PULONG)ExAllocatePoolWithTag(
        PagedPool,
        NUM_OF_BUFF * sizeof(ULONG),
        ROOTKIT_DETECTOR_TAG);

    if (self->BuffAlreadyUsedSpace == NULL)
    {
        ExFreePoolWithTag(self->PrimaryBuffIndex, ROOTKIT_DETECTOR_TAG);
        self->Status = DESTROYED;
        return NO_MEMORY;
    }

    for (ULONG i = 0; i < NUM_OF_BUFF; i++)
    {
        self->BuffAlreadyUsedSpace[i] = 0;
        self->PrimaryBuffIndex[i] = NULL;
    }

    StatusCode tmp = MemoryAllocator_NewBuffBlock(self);
    if (tmp != SUCCESS)
    {
        ExFreePoolWithTag(self->PrimaryBuffIndex, ROOTKIT_DETECTOR_TAG);
        ExFreePoolWithTag(self->BuffAlreadyUsedSpace, ROOTKIT_DETECTOR_TAG);
        self->Status = DESTROYED;
        return tmp;
    }

    return SUCCESS;
}

StatusCode MemoryAllocator_CleanAll(MemoryAllocator* self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    MemoryAllocator_FreeAllBuff(self);
    ExFreePoolWithTag(self->PrimaryBuffIndex, ROOTKIT_DETECTOR_TAG);
    ExFreePoolWithTag(self->BuffAlreadyUsedSpace, ROOTKIT_DETECTOR_TAG);
    self->Status = DESTROYED;
    return SUCCESS;
}

StatusCode MemoryAllocator_GetBuff(MemoryAllocator* self, PCHAR *pBuff, const ULONG needLength)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    if (pBuff == NULL || needLength > CAPICITY_OF_ONE_BUFF)
        return OUT_OF_RANGE;

    if (self->WriteIndex.offset == CAPICITY_OF_ONE_BUFF
        || CAPICITY_OF_ONE_BUFF - self->WriteIndex.offset < needLength)
    {
        if (self->PrimaryBuffIndex[++self->WriteIndex.index] == NULL)
        {
            StatusCode tmp = MemoryAllocator_NewBuffBlock(self);

            if (tmp != SUCCESS)
                self->WriteIndex.index--;
            return NO_MEMORY;
        }
        self->WriteIndex.offset = 0;
    }

    *pBuff = self->PrimaryBuffIndex[self->WriteIndex.index] + self->WriteIndex.offset;
    self->WriteIndex.offset += needLength;
    self->BuffAlreadyUsedSpace[self->WriteIndex.index] += needLength;

    return SUCCESS;
}

StatusCode MemoryAllocator_ReadBuff(MemoryAllocator* self, PCHAR buff, const ULONG requireLength, PULONG pRealReadLength)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    if (buff == NULL || pRealReadLength == NULL || requireLength == 0)
        return OUT_OF_RANGE;

    if (self->PrimaryBuffIndex[self->ReadIndex.index] == NULL)
    {
        *pRealReadLength = 0;
        return SUCCESS;
    }

    ULONG accumulate = 0;
    ULONG kernelLeftLength = 0;
    ULONG userLeftLength = requireLength;

    while (self->PrimaryBuffIndex[self->ReadIndex.index] != NULL)
    {
        kernelLeftLength = self->BuffAlreadyUsedSpace[self->ReadIndex.index] - self->ReadIndex.offset;
        if (kernelLeftLength <= userLeftLength)
        {
            memcpy(buff + accumulate, self->PrimaryBuffIndex[self->ReadIndex.index] + self->ReadIndex.offset, kernelLeftLength);
            accumulate += kernelLeftLength;
            userLeftLength -= kernelLeftLength;
            self->ReadIndex.index++;
            self->ReadIndex.offset = 0;
        }
        else
        {
            memcpy(buff + accumulate, self->PrimaryBuffIndex[self->ReadIndex.index] + self->ReadIndex.offset, userLeftLength);
            accumulate += userLeftLength;
            userLeftLength = 0;
            self->ReadIndex.offset += userLeftLength;
            if (self->ReadIndex.offset == self->BuffAlreadyUsedSpace[self->ReadIndex.index])
            {
                self->ReadIndex.index++;
                self->ReadIndex.offset = 0;
            }
            break;
        }
    }

    *pRealReadLength = accumulate;

    return SUCCESS;
}

StatusCode MemoryAllocator_ResetBuff(MemoryAllocator* self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    self->WriteIndex.index = 0, self->WriteIndex.offset = 0;
    self->ReadIndex.index = 0, self->ReadIndex.offset = 0;
    self->BuffAlreadyUsedSpace[0] = 0;
    for (int i = 1; i < NUM_OF_BUFF; i++)
    {
        if (self->PrimaryBuffIndex[i] != NULL)
            ExFreePoolWithTag(self->PrimaryBuffIndex[i], ROOTKIT_DETECTOR_TAG);
        self->BuffAlreadyUsedSpace[i] = 0;
    }
    return SUCCESS;
}

StatusCode MemoryAllocator_FreeAllBuff(MemoryAllocator* self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    for (ULONG i = 0; i < NUM_OF_BUFF; i++)
    {
        self->BuffAlreadyUsedSpace[i] = 0;
        if (self->PrimaryBuffIndex[i] != NULL)
        {
            ExFreePoolWithTag(self->PrimaryBuffIndex[i], ROOTKIT_DETECTOR_TAG);
            self->PrimaryBuffIndex[i] = NULL;
        }
    }

    self->Status = NORMAL;
    self->WriteIndex.index = 0, self->WriteIndex.offset = 0;
    self->ReadIndex.index = 0, self->ReadIndex.offset = 0;

    return SUCCESS;
}

StatusCode MemoryAllocator_NewBuffBlock(MemoryAllocator* self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    for (int i = 0; i < NUM_OF_BUFF; i++)
    {
        if (self->PrimaryBuffIndex[i] == NULL)
        {
            self->PrimaryBuffIndex[i] =
                (PCHAR)ExAllocatePoolWithTag(
                    PagedPool,
                    CAPICITY_OF_ONE_BUFF,
                    ROOTKIT_DETECTOR_TAG
                );
            if (self->PrimaryBuffIndex[i] == NULL)
                return NO_MEMORY;

            return SUCCESS;
        }

        if (i == NUM_OF_BUFF - 1)
            return NO_MEMORY;
    }

    return UNKNOWN;
}