#pragma once
#include "stdafx.h"

class DriverCommunicator
{
private:
    ULONG BufferLength;
    HANDLE DetectorDevice{};
    PCHAR Buffer{};
    ULONG ReadLength{};
	int sourceID;
    enum
    {
        DEVICE_OPENED,
        DEVICE_NOT_OPENED,
        ERROR_OCCUR
    } Status;

    StatusCode Append2ProcessTree();
    StatusCode Append2ProcessTree(PROCESSENTRY32 &proc);
	StatusCode CallAPI();

public:
    DriverCommunicator(ULONG bufferLength);
    ~DriverCommunicator();
    StatusCode SwitchDetector(UCHAR id);
    StatusCode Snapshot();
    StatusCode GetInfo();
};