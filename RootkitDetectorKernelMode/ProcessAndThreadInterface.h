#pragma once

#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
#include "CommonHeader.h"
#include "MemAllocatorRootkit.h"

#define EPROCESS_LIST_OFFSET_WIN7 0x0b8
#define UNIQUE_PROCESS_ID_OFFSET_WIN7 0x0b4
#define IMAGE_FILE_NAME_OFFSET_WIN7 0x16c
#define PROCESS_CREATE_INFO_OFFSET_WIN7 0x1ec
#define INHERINT_PROCESS_ID_OFFSET_WIN7 0x140
#define ETHREAD_LIST_HEAD_IN_EPROCESS_OFFSET_WIN7 0x188

#define ETHREAD_LIST_HEAD_IN_ETHREAD_OFFSET_WIN7 0x268
#define THREAD_CLINET_OFFSET_WIN7 0x22c

typedef struct _ProcessInfoPackager
{
    ProcessInfoPackage Info;
    StructStatus Status;
} ProcessInfoPackager;

StatusCode ProcessInfoPackager_Init(ProcessInfoPackager *self, const PEPROCESS pInfoPosition);
StatusCode ProcessInfoPackager_ClearAll(ProcessInfoPackager *self);
StatusCode ProcessInfoPackager_GetInfoLength(ProcessInfoPackager *self, USHORT *pLength);
StatusCode ProcessInfoPackager_WriteToBuff(ProcessInfoPackager *self, PCHAR const buff);

typedef struct _ThreadInfoPackager
{
    ThreadInfoPackage Info;
    StructStatus Status;
} ThreadInfoPackager;

StatusCode ThreadInfoPackager_Init(ThreadInfoPackager *self, const PETHREAD pInfoPosition);
StatusCode ThreadInfoPackager_ClearAll(ThreadInfoPackager *self);
StatusCode ThreadInfoPackager_GetInfoLength(ThreadInfoPackager *self, USHORT *pLength);
StatusCode ThreadInfoPackager_WriteToBuff(ThreadInfoPackager *self, PCHAR const buff);