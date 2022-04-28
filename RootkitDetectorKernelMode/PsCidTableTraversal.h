#pragma once
#pragma warning(disable:4054 4055)

#include <ProcessAndThreadInterface.h>
#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>

#define PSP_CID_TABLE 0x82b59d94
#define PSLOOKUPPROCESS_PSP_CID_TABLE_OFFSET_WIN7 0x20

typedef struct _PspCidTableTraversal
{
    StructStatus Status;
    PCHAR pHandle_Table;
    //PCHAR pObjectTypeIndexTable;
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