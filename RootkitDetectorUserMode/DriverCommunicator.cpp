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
{// TODO: Change temp code to real one
    ProcessTree::PrintInfos(Buffer, ReadLength);
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