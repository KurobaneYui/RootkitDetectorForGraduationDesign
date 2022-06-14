#include "stdafx.h"
#include "DriverCommunicator.h"

DriverCommunicator::DriverCommunicator(ULONG bufferLength)
{
    Status = DEVICE_NOT_OPENED;
    ReadLength = 0;
    BufferLength = bufferLength;
    Buffer = new CHAR[BufferLength];
    if (Buffer == nullptr)
        return;
    DetectorDevice = CreateFile((LPCWSTR)L"\\\\.\\RootkitDetectorDevice", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (DetectorDevice == INVALID_HANDLE_VALUE)
    {
        delete[] Buffer;
    }

    sourceID = 0;
    Status = DEVICE_OPENED;
    return;
}

DriverCommunicator::~DriverCommunicator()
{
    if (Status != DEVICE_NOT_OPENED)
    {
        CloseHandle(DetectorDevice);
        delete[] Buffer;
    }
    return;
}

StatusCode DriverCommunicator::Append2ProcessTree()
{
    ULONG pointer{ 0 };
    StatusCode getStatus{ SUCCESS };
    while (ReadLength - pointer > 2 && (ReadLength - pointer) >= ((ThreadInfoPackage *)(Buffer + pointer))->length)
    {
        if (((ThreadInfoPackage*)(Buffer + pointer))->type == 1)
            getStatus = ProcessTree::AddProcess((ProcessInfoPackage*)(Buffer + pointer), pointer, sourceID);
        else if (((ThreadInfoPackage*)(Buffer + pointer))->type == 2)
            getStatus = ProcessTree::AddThread((ThreadInfoPackage*)(Buffer + pointer), pointer);
        else
            return UNKNOWN;

        if (getStatus != SUCCESS)
            return getStatus;
    }

    memmove(Buffer, Buffer + pointer, ReadLength - pointer);
    ReadLength -= pointer;

    return SUCCESS;
}

StatusCode DriverCommunicator::Append2ProcessTree(PROCESSENTRY32 &proc)
{
    return ProcessTree::AddProcess(proc, sourceID);
}

StatusCode DriverCommunicator::CallAPI()
{
    HANDLE pHandle;
    PROCESSENTRY32 proc;
    DWORD procId;
    pHandle = CreateToolhelp32Snapshot(0x2, 0x0);
    if (pHandle == INVALID_HANDLE_VALUE) {
        return UNKNOWN;
    }
    proc.dwSize = sizeof(PROCESSENTRY32);
    while (Process32Next(pHandle, &proc)) {
        if (proc.th32ProcessID == 0)
            continue;
        Append2ProcessTree(proc);
    }
    CloseHandle(pHandle);
    return SUCCESS;
}

StatusCode DriverCommunicator::SwitchDetector(UCHAR id)
{
    if (Status != DEVICE_OPENED)
        return UNKNOWN;

    sourceID = id;
    if (sourceID == 0 || sourceID == 1)
    {
        if (DeviceIoControl(DetectorDevice, IOCTL_SWITCH, /*&input*/&id, sizeof(UCHAR), /*&output*/NULL, 4, &ReadLength, NULL))
            return SUCCESS;
        else
            return UNKNOWN;
    }
    else
    {
        return SUCCESS;
    }

}

StatusCode DriverCommunicator::Snapshot()
{
    if (Status != DEVICE_OPENED)
        return UNKNOWN;

    if (sourceID == 0 || sourceID == 1)
    {
        if (DeviceIoControl(DetectorDevice, IOCTL_SNAPSHOT, /*&input*/NULL, 4, /*&output*/NULL, 4, &ReadLength, NULL))
            return SUCCESS;
        else
            return UNKNOWN;
    }
    else
    {
        return SUCCESS;
    }
}

StatusCode DriverCommunicator::GetInfo()
{
    if (Status != DEVICE_OPENED)
        return UNKNOWN;
    
    if (sourceID == 0 || sourceID == 1)
    {
        BOOL readStatus{};
		ULONG oneReadLength{};
        do
        {
            readStatus = ReadFile(DetectorDevice, (PVOID)(Buffer + ReadLength), BufferLength - ReadLength, &oneReadLength, NULL);
            ReadLength += oneReadLength;
            if (!readStatus)
                break;
            Append2ProcessTree();
        } while (oneReadLength);

        if (readStatus)
            return SUCCESS;
        else
            return UNKNOWN;
    }
    else
    {
        return CallAPI();
    }
}