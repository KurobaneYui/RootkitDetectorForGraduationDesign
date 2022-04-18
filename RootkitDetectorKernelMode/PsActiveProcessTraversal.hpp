#pragma once

#include <ProcessAndThreadInterface.h>

#define PS_ACTIVE_PROCESS_HEAD 0x82b59d70

class PsActiveProcessTraversal : public Detector
{
private:
    PLIST_ENTRY ListHead;
    MemoryAllocator * pMemoryAllocator;
    enum { NORMAL, DESTROYED } Status;
    _StatusCode Traversal();

public:
    PsActiveProcessTraversal() = default;
    ~PsActiveProcessTraversal() = default;
    _StatusCode Init(MemoryAllocator * pAllocator);
    _StatusCode ClearAll();
    _StatusCode Snapshot();
    _StatusCode GetInfos(
        PCHAR buffer,
        const ULONG bufferLength,
        ULONG &realReadLength);
    _StatusCode FreeupSnapshot();
};

class PsActiveThreadTraversal
{
private:
    PLIST_ENTRY ListHead;
    MemoryAllocator * pMemoryAllocator;
    enum { NORMAL, DESTROYED } Status;
    _StatusCode Traversal();

public:
    PsActiveThreadTraversal() = default;
    ~PsActiveThreadTraversal() = default;
    _StatusCode Init(PLIST_ENTRY pHead, MemoryAllocator * pAllocator);
    _StatusCode Snapshot();
};

_StatusCode PsActiveProcessTraversal::Init(MemoryAllocator * pAllocator)
{
    Status = DESTROYED;

    if (pAllocator == nullptr)
        return OUT_OF_RANGE;

    ListHead = (PLIST_ENTRY)PS_ACTIVE_PROCESS_HEAD;
    pMemoryAllocator = pAllocator;

    Status = NORMAL;

    return SUCCESS;
}

_StatusCode PsActiveProcessTraversal::ClearAll()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    pMemoryAllocator->ResetBuff();

    Status = DESTROYED;
    return SUCCESS;
}

_StatusCode PsActiveProcessTraversal::Traversal()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    PLIST_ENTRY pCurrentList = ListHead->Flink;

    while (pCurrentList != ListHead)
    {
        USHORT length;
        PCHAR buff;
        PEPROCESS pCurrentProcess = (PEPROCESS)((PCHAR)pCurrentList - EPROCESS_LIST_OFFSET_WIN7);
        ProcessInfoPackager infoPackager;

        _StatusCode tmp = infoPackager.Init(pCurrentProcess);
        if (tmp != SUCCESS)
        {
            infoPackager.ClearAll();
            return tmp;
        }
        infoPackager.GetInfoLength(length);
        if (pMemoryAllocator->GetBuff(buff, length) != SUCCESS)
        {
            infoPackager.ClearAll();
            return NO_MEMORY;
        }
        tmp = infoPackager.WriteToBuff(buff);
        if (tmp != SUCCESS)
        {
            infoPackager.ClearAll();
            return tmp;
        }
        infoPackager.ClearAll();

        PsActiveThreadTraversal threadTraversal;
        tmp = threadTraversal.Init((PLIST_ENTRY)((PCHAR)pCurrentProcess + ETHREAD_LIST_HEAD_IN_EPROCESS_OFFSET_WIN7), pMemoryAllocator);
        if (tmp != SUCCESS)
            return tmp;
        tmp = threadTraversal.Snapshot();
        if (tmp != SUCCESS)
            return tmp;

        pCurrentList = pCurrentList->Flink;

        return SUCCESS;
    }

    return SUCCESS;
}

_StatusCode PsActiveProcessTraversal::Snapshot()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    pMemoryAllocator->ResetBuff();
    return Traversal();
}

_StatusCode PsActiveProcessTraversal::GetInfos(
    PCHAR buffer,
    const ULONG bufferLength,
    ULONG &realReadLength)
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    if (buffer == nullptr || bufferLength == 0)
        return OUT_OF_RANGE;

    return pMemoryAllocator->ReadBuff(buffer, bufferLength, realReadLength);
}

_StatusCode PsActiveProcessTraversal::FreeupSnapshot()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    pMemoryAllocator->ResetBuff();

    return SUCCESS;
}

_StatusCode PsActiveThreadTraversal::Init(PLIST_ENTRY pHead, MemoryAllocator * pAllocator)
{
    Status = DESTROYED;

    if (pHead == nullptr || pAllocator == nullptr)
        return OUT_OF_RANGE;

    ListHead = pHead;
    pMemoryAllocator = pAllocator;

    Status = NORMAL;

    return SUCCESS;
}

_StatusCode PsActiveThreadTraversal::Traversal()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    PLIST_ENTRY pCurrentList = ListHead->Flink;

    while (pCurrentList != ListHead)
    {
        USHORT length;
        PCHAR buff;
        PETHREAD pCurrentThread = (PETHREAD)((PCHAR)pCurrentList - ETHREAD_LIST_HEAD_IN_ETHREAD_OFFSET_WIN7);
        ThreadInfoPackager infoPackager;

        _StatusCode tmp = infoPackager.Init(pCurrentThread);
        if (tmp != SUCCESS)
        {
            infoPackager.ClearAll();
            return tmp;
        }
        infoPackager.GetInfoLength(length);
        if (pMemoryAllocator->GetBuff(buff, length) != SUCCESS)
        {
            infoPackager.ClearAll();
            return NO_MEMORY;
        }
        tmp = infoPackager.WriteToBuff(buff);
        if (tmp != SUCCESS)
        {
            infoPackager.ClearAll();
            return tmp;
        }
        infoPackager.ClearAll();

        pCurrentList = pCurrentList->Flink;

        return SUCCESS;
    }

    return SUCCESS;
}

_StatusCode PsActiveThreadTraversal::Snapshot()
{
    if (Status == DESTROYED)
        return HAVE_DESTROYED;

    return Traversal();
}



//class EThreadTraversal
//{
//private:
//    _StatusCode StatusCode = SUCCESS;
//    PLIST_ENTRY pEThreadListHead = { 0 };
//    ULONG numOfThread = { 0 };
//    MemoryAllocator* pMemoryAllocator = { 0 };
//
//    _StatusCode Traversal()
//    {
//        PLIST_ENTRY ctList = pEThreadListHead->Flink;
//        while (ctList != pEThreadListHead)
//        {
//            PETHREAD ct = (PETHREAD)((PCHAR)ctList - ETHREAD_LIST_HEAD_IN_ETHREAD_OFFSET_WIN7);
//            USHORT needBuffLength;
//            PCHAR pBuff;
//
//            ThreadInfoPackager::GetNeededBuffLength(&needBuffLength, ct);
//            pMemoryAllocator->GetBuff(&pBuff, needBuffLength);
//            ThreadInfoPackager::CopyToProvidedBuff(pBuff, needBuffLength, ct);
//
//            numOfThread++;
//
//            ctList = ctList->Flink;
//        }
//
//        return StatusCode;
//    }
//
//public:
//    EThreadTraversal(const PLIST_ENTRY pEThreadList, const MemoryAllocator* pMemoryAllocator)
//    {
//        DbgPrint("EThreadTraversal Construct\n");
//        StatusCode = SUCCESS;
//        if (pMemoryAllocator == NULL || pEThreadList == NULL)
//            StatusCode = OUT_OF_RANGE;
//        pEThreadListHead = pEThreadList;
//        numOfThread = 0;
//    }
//
//    ~EThreadTraversal() = default;
//
//    _StatusCode GetThreadNum(PULONG pNum)
//    {
//        if (StatusCode != SUCCESS)
//            return StatusCode;
//
//        if (pNum == NULL)
//            return OUT_OF_RANGE;
//
//        *pNum = numOfThread;
//        return StatusCode;
//    }
//
//    _StatusCode SnapshotThread()
//    {
//        Traversal();
//        return StatusCode;
//    }
//};
//
//class EProcessTraversal : public ProcessTraversal
//{
//private:
//    _StatusCode StatusCode = SUCCESS;
//    PLIST_ENTRY pEProcessListHead = { 0 };
//    ULONG numOfProcess = { 0 }, numOfThread = { 0 };
//    MemoryAllocator* pMemoryAllocator = { 0 };
//
//    _StatusCode Traversal()
//    {
//        PLIST_ENTRY cpList = pEProcessListHead->Flink;
//        while (cpList != pEProcessListHead)
//        {
//            PEPROCESS cp = (PEPROCESS)((PCHAR)cpList - EPROCESS_LIST_OFFSET_WIN7);
//            ULONG threadNum = 0;
//            USHORT needBuffLength;
//            PCHAR pBuff;
//
//            ProcessInfoPackager::GetNeededBuffLength(&needBuffLength, cp);
//            pMemoryAllocator->GetBuff(&pBuff, needBuffLength);
//            ProcessInfoPackager::CopyToProvidedBuff(pBuff, needBuffLength, cp);
//            numOfProcess++;
//
//            EThreadTraversal tt = EThreadTraversal(
//                (PLIST_ENTRY)((PCHAR)cp + ETHREAD_LIST_HEAD_IN_EPROCESS_OFFSET_WIN7),
//                pMemoryAllocator);
//            tt.SnapshotThread();
//
//            ProcessInfoPackager::AddThreadNumToProvidedBuff(pBuff, threadNum);
//
//            tt.GetThreadNum(&threadNum);
//            numOfThread += threadNum;
//            cpList = cpList->Flink;
//        }
//
//        return StatusCode;
//    }
//
//public:
//    EProcessTraversal(const MemoryAllocator* pMemoryAllocator)
//    {
//        DbgPrint("EProcessTraversal Construct\n");
//        StatusCode = SUCCESS;
//        if (pMemoryAllocator == NULL)
//            StatusCode = OUT_OF_RANGE;
//        pEProcessListHead = (PLIST_ENTRY)PS_ACTIVE_PROCESS_HEAD;
//        numOfProcess = numOfThread = 0;
//    }
//
//    ~EProcessTraversal() = default;
//
//    _StatusCode SnapshotProcessAndThread()
//    {
//        if (pMemoryAllocator->ResetBuff() != SUCCESS)
//            return StatusCode = NO_MEMORY;
//
//        Traversal();
//        return StatusCode;
//    }
//
//    _StatusCode GetProcessAndThreadInfo(
//        PCHAR buffer,
//        const ULONG bufferLength,
//        PULONG pRealLength)
//    {
//        _StatusCode status = SUCCESS;
//        status = pMemoryAllocator->ReadBuff(buffer, bufferLength, pRealLength);
//        return StatusCode = status;
//    }
//
//    _StatusCode FreeupSnapshot()
//    {
//        if (pMemoryAllocator->ResetBuff() != SUCCESS)
//            return StatusCode = NO_MEMORY;
//
//        numOfProcess = numOfThread = 0;
//
//        return StatusCode = SUCCESS;
//    }
//
//    _StatusCode GetProcessNum(PULONG pNum)
//    {
//        if (StatusCode != SUCCESS)
//            return StatusCode;
//
//        if (pNum == NULL)
//            return OUT_OF_RANGE;
//
//        *pNum = numOfProcess;
//        return StatusCode;
//    }
//
//    _StatusCode GetThreadNum(PULONG pNum)
//    {
//        if (StatusCode != SUCCESS)
//            return StatusCode;
//
//        if (pNum == NULL)
//            return OUT_OF_RANGE;
//
//        *pNum = numOfThread;
//        return StatusCode;
//    }
//};