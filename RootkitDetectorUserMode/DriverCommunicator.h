#pragma once
#include "stdafx.h"

class DriverCommunicator
{
private:
    UINT16 BufferLength;
    HANDLE DetectorDevice{};
    PCHAR Buffer{};
    DWORD ReadLength{};
    enum
    {
        DEVICE_OPENED,
        DEVICE_NOT_OPENED,
        ERROR_OCCUR
    } Status;

    StatusCode Append2ProcessTree(ProcessTree &PT);

public:
    DriverCommunicator(UINT16 bufferLength);
    ~DriverCommunicator();
    StatusCode SwitchDetector(UCHAR id);
    StatusCode Snapshot();
    StatusCode GetInfo(ProcessTree &PT);
};