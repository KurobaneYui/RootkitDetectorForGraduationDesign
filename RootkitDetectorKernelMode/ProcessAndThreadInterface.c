#include "ProcessAndThreadInterface.h"

StatusCode ProcessInfoPackager_Init(ProcessInfoPackager *self, const PEPROCESS pInfoPosition)
{
    self->Status = DESTROYED;

    if (pInfoPosition == NULL)
        return OUT_OF_RANGE;

    PUNICODE_STRING pUnicodeString =
        (PUNICODE_STRING)((PCHAR)pInfoPosition + IMAGE_FILE_NAME_OFFSET_WIN7);

    self->Info.length = sizeof(ProcessInfoPackage)
        + (pUnicodeString->Length + 1) * sizeof(WCHAR);
    self->Info.type = 1;
    self->Info.pid = *(PULONG)((PCHAR)pInfoPosition + UNIQUE_PROCESS_ID_OFFSET_WIN7);
    self->Info.parentPid = *(PULONG)((PCHAR)pInfoPosition + INHERINT_PROCESS_ID_OFFSET_WIN7);
    self->Info.path = (PCHAR)ExAllocatePoolWithTag(PagedPool, (pUnicodeString->Length + 1) * sizeof(WCHAR), ROOTKIT_DETECTOR_TAG);
    if (self->Info.path == NULL)
        return NO_MEMORY;

    RtlZeroMemory(self->Info.path, (pUnicodeString->Length + 1) * sizeof(WCHAR));
    wcsncpy((PWCHAR)self->Info.path, pUnicodeString->Buffer, pUnicodeString->Length);

    self->Status = NORMAL;

    return SUCCESS;
}

StatusCode ProcessInfoPackager_ClearAll(ProcessInfoPackager *self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    if (self->Info.path != NULL)
        ExFreePoolWithTag(self->Info.path, ROOTKIT_DETECTOR_TAG);

    self->Status = DESTROYED;
    return SUCCESS;
}

StatusCode ProcessInfoPackager_GetInfoLength(ProcessInfoPackager *self, USHORT *pLength)
{
    if (self->Status != SUCCESS)
        return HAVE_DESTROYED;

    if (pLength == NULL)
        return OUT_OF_RANGE;

    *pLength = self->Info.length;
    return SUCCESS;
}

StatusCode ProcessInfoPackager_WriteToBuff(ProcessInfoPackager *self, PCHAR const buff)
{
    if (buff == NULL)
        return OUT_OF_RANGE;

    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    memcpy(buff, &self->Info, self->Info.length);

    return SUCCESS;
}


StatusCode ThreadInfoPackager_Init(ThreadInfoPackager *self, const PETHREAD pInfoPosition)
{
    self->Status = DESTROYED;

    if (pInfoPosition == NULL)
        return OUT_OF_RANGE;

    PCLIENT_ID pClientID = (PCLIENT_ID)((PCHAR)pInfoPosition + THREAD_CLINET_OFFSET_WIN7);
    self->Info.length = sizeof(ThreadInfoPackage);
    self->Info.type = 2;
    self->Info.tid = *(PULONG)pClientID->UniqueThread;
    self->Info.parentPid = *(PULONG)pClientID->UniqueProcess;

    self->Status = NORMAL;

    return SUCCESS;
}

StatusCode ThreadInfoPackager_ClearAll(ThreadInfoPackager *self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    self->Status = DESTROYED;
    return SUCCESS;
}

StatusCode ThreadInfoPackager_GetInfoLength(ThreadInfoPackager *self, USHORT *pLength)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    if (pLength == NULL)
        return OUT_OF_RANGE;

    *pLength = self->Info.length;
    return SUCCESS;
}

StatusCode ThreadInfoPackager_WriteToBuff(ThreadInfoPackager *self, PCHAR const buff)
{
    if (buff == NULL)
        return OUT_OF_RANGE;

    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    memcpy(buff, &self->Info, self->Info.length);

    return SUCCESS;
}