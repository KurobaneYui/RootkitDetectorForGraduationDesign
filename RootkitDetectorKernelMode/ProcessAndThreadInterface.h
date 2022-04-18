#pragma once

#include <ntddk.h>
#include <wdm.h>
#include "CommonHeader.h"
#include "MemAllocatorRootkit.h"

#define EPROCESS_LIST_OFFSET_WIN7 0x0b8
#define UNIQUE_PROCESS_ID_OFFSET_WIN7 0x0b4
#define IMAGE_FILE_NAME_OFFSET_WIN7 0x1ec
#define INHERINT_PROCESS_ID_OFFSET_WIN7 0x140
#define ETHREAD_LIST_HEAD_IN_EPROCESS_OFFSET_WIN7 0x188

#define ETHREAD_LIST_HEAD_IN_ETHREAD_OFFSET_WIN7 0x268
#define THREAD_CLINET_OFFSET_WIN7 0x22c

class ProcessInfoPackager
{
private:
    ProcessInfoPackage Info;
    enum { NORMAL, DESTROYED } Status;

public:
    ProcessInfoPackager() = default;
    ~ProcessInfoPackager() = default;
    _StatusCode Init(const PEPROCESS pInfoPosition);
    _StatusCode ClearAll();
    _StatusCode GetInfoLength(USHORT &length);
    _StatusCode WriteToBuff(PCHAR const buff);
};

class ThreadInfoPackager
{
private:
    ThreadInfoPackage Info;
    enum { NORMAL, DESTROYED } Status;

public:
    ThreadInfoPackager() = default;
    ~ThreadInfoPackager() = default;
    _StatusCode Init(const PETHREAD pInfoPosition);
    _StatusCode ClearAll();
    _StatusCode GetInfoLength(USHORT &length);
    _StatusCode WriteToBuff(PCHAR const buff);
};

__interface Detector
{
public:
    virtual _StatusCode Init(MemoryAllocator * pAllocator) = 0;
    virtual _StatusCode ClearAll() = 0;
    virtual _StatusCode Snapshot() = 0;
    virtual _StatusCode GetInfos(
        PCHAR buffer,
        const ULONG bufferLength,
        ULONG &realReadLength) = 0;
    virtual _StatusCode FreeupSnapshot() = 0;
};