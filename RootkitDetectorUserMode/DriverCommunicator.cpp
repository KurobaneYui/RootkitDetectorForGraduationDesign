#include "stdafx.h"
#include "DriverCommunicator.h"

DriverCommunicator::DriverCommunicator(UINT bufferLength)
{
    Status = DEVICE_NOT_OPENED;
    BufferLength = bufferLength;
    Buffer = new CHAR[BufferLength];
    if (Buffer == nullptr)
        return;
    DetectorDevice = CreateFile((LPCWSTR)L"\\\\.\\RootkitDetectorDevice", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (DetectorDevice == INVALID_HANDLE_VALUE)
    {
        delete[] Buffer;
    }

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

StatusCode DriverCommunicator::Append2ProcessTree(ProcessTree &PT)
{
    ULONG pointer{ 0 };
    StatusCode getStatus{ SUCCESS };
    while (pointer < ReadLength)
    {
        if (((ThreadInfoPackage*)(Buffer + pointer))->type == 1)
            getStatus = ProcessTree::AddProcess((ProcessInfoPackage*)(Buffer + pointer), pointer);
        else if (((ThreadInfoPackage*)(Buffer + pointer))->type == 2)
            getStatus = ProcessTree::AddThread((ThreadInfoPackage*)(Buffer + pointer), pointer);
        else
            return UNKNOWN;

        if (getStatus != SUCCESS)
            return getStatus;
    }

    return SUCCESS;
}

StatusCode DriverCommunicator::SwitchDetector(UCHAR id)
{
    if (Status != DEVICE_OPENED)
        return UNKNOWN;

    if (DeviceIoControl(DetectorDevice, IOCTL_SWITCH, /*&input*/&id, sizeof(UCHAR), /*&output*/NULL, 4, &ReadLength, NULL))
        return SUCCESS;
    else
        return UNKNOWN;
}

StatusCode DriverCommunicator::Snapshot()
{
    if (Status != DEVICE_OPENED)
        return UNKNOWN;

    if (DeviceIoControl(DetectorDevice, IOCTL_SNAPSHOT, /*&input*/NULL, 4, /*&output*/NULL, 4, &ReadLength, NULL))
        return SUCCESS;
    else
        return UNKNOWN;
}

StatusCode DriverCommunicator::GetInfo(ProcessTree &PT)
{
    if (Status != DEVICE_OPENED)
        return UNKNOWN;

    BOOL readStatus{};
    do
    {
        readStatus = ReadFile(DetectorDevice, (PVOID)Buffer, BufferLength, &ReadLength, NULL);
        if (!readStatus)
            break;
        Append2ProcessTree(PT);
    } while (ReadLength);

    if (readStatus)
        return SUCCESS;
    else
        return UNKNOWN;
}