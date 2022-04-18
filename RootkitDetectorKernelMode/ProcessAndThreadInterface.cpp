#include "ProcessAndThreadInterface.h"

_StatusCode ProcessInfoPackager::Init(const PEPROCESS const pInfoPosition)
{
    Status=DESTROYED;

    if (pInfoPosition == nullptr)
        return OUT_OF_RANGE;

    PUNICODE_STRING pUnicodeString =
        (PUNICODE_STRING)((PCHAR)pInfoPosition + IMAGE_FILE_NAME_OFFSET_WIN7);

    Info.length = sizeof(ProcessInfoPackage)
        - sizeof(PWCHAR)
        + (pUnicodeString->Length + 1) * sizeof(WCHAR);
    Info.type = 1;
    Info.pid = *(PULONG)((PCHAR)pInfoPosition + UNIQUE_PROCESS_ID_OFFSET_WIN7);
    Info.parentPid = *(PULONG)((PCHAR)pInfoPosition + INHERINT_PROCESS_ID_OFFSET_WIN7);
    Info.path = (PCHAR)ExAllocatePoolWithTag(PagedPool, (pUnicodeString->Length + 1) * sizeof(WCHAR), ROOTKIT_DETECTOR_TAG);
    if (Info.path == nullptr)
        return NO_MEMORY;

    RtlZeroMemory(Info.path, (pUnicodeString->Length + 1) * sizeof(WCHAR));
    wcsncpy((PWCHAR)Info.path, pUnicodeString->Buffer, pUnicodeString->Length);

    Status = NORMAL;

    return SUCCESS;
}

_StatusCode ProcessInfoPackager::ClearAll()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    if (Info.path != nullptr)
        ExFreePoolWithTag(Info.path, ROOTKIT_DETECTOR_TAG);

    Status = DESTROYED;
    return SUCCESS;
}

_StatusCode ProcessInfoPackager::GetInfoLength(USHORT &length)
{
    if (Status != SUCCESS)
        return HAVE_DESTROYED;

    length = Info.length;
    return SUCCESS;
}

_StatusCode ProcessInfoPackager::WriteToBuff(PCHAR const buff)
{
    if (buff == nullptr)
        return OUT_OF_RANGE;

    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    memcpy(buff, &Info, Info.length);

    return SUCCESS;
}


_StatusCode ThreadInfoPackager::Init(const PETHREAD const pInfoPosition)
{
    Status=DESTROYED;

    if (pInfoPosition == nullptr)
        return OUT_OF_RANGE;

    PCLIENT_ID pClientID = (PCLIENT_ID)((PCHAR)pInfoPosition + THREAD_CLINET_OFFSET_WIN7);
    Info.length = sizeof(ThreadInfoPackage);
    Info.type = 2;
    Info.tid = *(PULONG)pClientID->UniqueThread;
    Info.parentPid = *(PULONG)pClientID->UniqueProcess;

    Status=NORMAL;

    return SUCCESS;
}

_StatusCode ThreadInfoPackager::ClearAll()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    Status = DESTROYED;
    return SUCCESS;
}

_StatusCode ThreadInfoPackager::GetInfoLength(USHORT &length)
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    length = Info.length;
    return SUCCESS;
}

_StatusCode ThreadInfoPackager::WriteToBuff(PCHAR const buff)
{
    if (buff == nullptr)
        return OUT_OF_RANGE;

    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    memcpy(buff, &Info, Info.length);

    return SUCCESS;
}