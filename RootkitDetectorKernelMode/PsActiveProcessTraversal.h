#pragma once
#pragma warning(disable:4055)

#include <ProcessAndThreadInterface.h>

#define PS_ACTIVE_PROCESS_HEAD 0x82b59d70

typedef struct _PsActiveProcessTraversal
{
    PLIST_ENTRY ListHead;
    MemoryAllocator * pMemoryAllocator;
    StructStatus Status;
} PsActiveProcessTraversal;

StatusCode PsActiveProcessTraversal_Traversal(PsActiveProcessTraversal *self);
StatusCode PsActiveProcessTraversal_Init(PsActiveProcessTraversal *self, MemoryAllocator * pAllocator);
StatusCode PsActiveProcessTraversal_ClearAll(PsActiveProcessTraversal *self);
StatusCode PsActiveProcessTraversal_Snapshot(PsActiveProcessTraversal *self);
StatusCode PsActiveProcessTraversal_GetInfos(
    PsActiveProcessTraversal *self,
    PCHAR buffer,
    const ULONG bufferLength,
    PULONG pRealReadLength);
StatusCode PsActiveProcessTraversal_FreeupSnapshot(PsActiveProcessTraversal *self);


typedef struct _PsActiveThreadTraversal
{
    PLIST_ENTRY ListHead;
    MemoryAllocator * pMemoryAllocator;
    StructStatus Status;
} PsActiveThreadTraversal;

StatusCode PsActiveThreadTraversal_Traversal(PsActiveThreadTraversal *self);
StatusCode PsActiveThreadTraversal_Init(PsActiveThreadTraversal *self, PLIST_ENTRY pHead, MemoryAllocator * pAllocator);
StatusCode PsActiveThreadTraversal_Snapshot(PsActiveThreadTraversal *self);