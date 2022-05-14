#include <PsActiveProcessTraversal.h>

StatusCode PsActiveProcessTraversal_Init(PsActiveProcessTraversal *self, MemoryAllocator * pAllocator)
{
    self->Status = DESTROYED;

    if (pAllocator == NULL)
        return OUT_OF_RANGE;

    self->ListHead = (PLIST_ENTRY)((PCHAR)PsInitialSystemProcess + EPROCESS_LIST_OFFSET_WIN7);
    self->ListHead = self->ListHead->Blink;
    self->pMemoryAllocator = pAllocator;

    self->Status = NORMAL;

    return SUCCESS;
}

StatusCode PsActiveProcessTraversal_ClearAll(PsActiveProcessTraversal *self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    MemoryAllocator_ResetBuff(self->pMemoryAllocator);

    self->Status = DESTROYED;
    return SUCCESS;
}

StatusCode PsActiveProcessTraversal_Traversal(PsActiveProcessTraversal *self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    PLIST_ENTRY pCurrentList = self->ListHead->Flink;

    while (pCurrentList != self->ListHead)
    {
        USHORT length;
        PCHAR buff;
        PEPROCESS pCurrentProcess = (PEPROCESS)((PCHAR)pCurrentList - EPROCESS_LIST_OFFSET_WIN7);
        ProcessInfoPackager infoPackager;

        StatusCode tmp = ProcessInfoPackager_Init(&infoPackager, pCurrentProcess);
        if (tmp != SUCCESS)
        {
            ProcessInfoPackager_ClearAll(&infoPackager);
            return tmp;
        }
        ProcessInfoPackager_GetInfoLength(&infoPackager, &length);
        if (MemoryAllocator_GetBuff(self->pMemoryAllocator, &buff, length) != SUCCESS)
        {
            ProcessInfoPackager_ClearAll(&infoPackager);
            return NO_MEMORY;
        }
        tmp = ProcessInfoPackager_WriteToBuff(&infoPackager, buff);
        if (tmp != SUCCESS)
        {
            ProcessInfoPackager_ClearAll(&infoPackager);
            return tmp;
        }
        KdPrint(("ProcessID:%u, ProcessPID:%u, ProcessName:%s, Process:%ws\n", infoPackager.Info.pid, infoPackager.Info.parentPid, infoPackager.Info.imageName, infoPackager.Info.path));
        ProcessInfoPackager_ClearAll(&infoPackager);

        PsActiveThreadTraversal threadTraversal;
        tmp = PsActiveThreadTraversal_Init(
            &threadTraversal,
            (PLIST_ENTRY)((PCHAR)pCurrentProcess + ETHREAD_LIST_HEAD_IN_EPROCESS_OFFSET_WIN7),
            self->pMemoryAllocator);
        if (tmp != SUCCESS)
            return tmp;
        tmp = PsActiveThreadTraversal_Snapshot(&threadTraversal);
        if (tmp != SUCCESS)
            return tmp;

        pCurrentList = pCurrentList->Flink;
    }

    return SUCCESS;
}

StatusCode PsActiveProcessTraversal_Snapshot(PsActiveProcessTraversal *self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    MemoryAllocator_ResetBuff(self->pMemoryAllocator);
    return PsActiveProcessTraversal_Traversal(self);
}

StatusCode PsActiveProcessTraversal_GetInfos(
    PsActiveProcessTraversal *self,
    PCHAR buffer,
    const ULONG bufferLength,
    PULONG pRealReadLength)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    if (buffer == NULL || pRealReadLength == NULL || bufferLength == 0)
        return OUT_OF_RANGE;

    return MemoryAllocator_ReadBuff(self->pMemoryAllocator, buffer, bufferLength, pRealReadLength);
}

StatusCode PsActiveProcessTraversal_FreeupSnapshot(PsActiveProcessTraversal *self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    MemoryAllocator_ResetBuff(self->pMemoryAllocator);

    return SUCCESS;
}

StatusCode PsActiveThreadTraversal_Init(PsActiveThreadTraversal *self, PLIST_ENTRY pHead, MemoryAllocator * pAllocator)
{
    self->Status = DESTROYED;

    if (pHead == NULL || pAllocator == NULL)
        return OUT_OF_RANGE;

    self->ListHead = pHead;
    self->pMemoryAllocator = pAllocator;

    self->Status = NORMAL;

    return SUCCESS;
}

StatusCode PsActiveThreadTraversal_Traversal(PsActiveThreadTraversal *self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    PLIST_ENTRY pCurrentList = self->ListHead->Flink;

    while (pCurrentList != self->ListHead)
    {
        USHORT length;
        PCHAR buff;
        PETHREAD pCurrentThread = (PETHREAD)((PCHAR)pCurrentList - ETHREAD_LIST_HEAD_IN_ETHREAD_OFFSET_WIN7);
        ThreadInfoPackager infoPackager;

        StatusCode tmp = ThreadInfoPackager_Init(&infoPackager, pCurrentThread);
        if (tmp != SUCCESS)
        {
            ThreadInfoPackager_ClearAll(&infoPackager);
            return tmp;
        }
        ThreadInfoPackager_GetInfoLength(&infoPackager, &length);
        if (MemoryAllocator_GetBuff(self->pMemoryAllocator, &buff, length) != SUCCESS)
        {
            ThreadInfoPackager_ClearAll(&infoPackager);
            return NO_MEMORY;
        }
        tmp = ThreadInfoPackager_WriteToBuff(&infoPackager, buff);
        if (tmp != SUCCESS)
        {
            ThreadInfoPackager_ClearAll(&infoPackager);
            return tmp;
        }
        KdPrint(("ThreadID:%u, ThreadPID:%u\n", infoPackager.Info.tid, infoPackager.Info.parentPid));
        ThreadInfoPackager_ClearAll(&infoPackager);

        pCurrentList = pCurrentList->Flink;
    }

    return SUCCESS;
}

StatusCode PsActiveThreadTraversal_Snapshot(PsActiveThreadTraversal *self)
{
    if (self->Status == DESTROYED)
        return HAVE_DESTROYED;

    return PsActiveThreadTraversal_Traversal(self);
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