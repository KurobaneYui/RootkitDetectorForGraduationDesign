#pragma once

#include <ProcessAndThreadInterface.h>

#define PSP_CID_TABLE 0x82b59d94

typedef struct _PspCidTableTraversal
{
    StructStatus Status;
    PCHAR pHandle_Table;
    MemoryAllocator *pMemoryAllocator;
} PspCidTableTraversal;

StatusCode PspCidTableTraversal_RecursiveTraversal(PspCidTableTraversal *self, PCHAR pCurrentTable);
StatusCode PspCidTableTraversal_Init(PspCidTableTraversal *self, MemoryAllocator *pAllocator);
StatusCode PspCidTableTraversal_ClearAll(PspCidTableTraversal *self);
StatusCode PspCidTableTraversal_Snapshot(PspCidTableTraversal *self);
StatusCode PspCidTableTraversal_GetInfos(
    PspCidTableTraversal *self,
    PCHAR buffer,
    const ULONG bufferLength,
    PULONG pRealReadLength);
StatusCode PspCidTableTraversal_FreeupSnapshot(PspCidTableTraversal *self);