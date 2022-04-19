#pragma once

#include "CommonHeader.h"
#include <ntddk.h>

#define CAPICITY_OF_ONE_BUFF (1024*128)
#define NUM_OF_BUFF 40
#define ROOTKIT_DETECTOR_TAG 'detc'

// 负责驱动遍历进程、线程所需内存的分配和释放
typedef struct _MemoryAllocator
{
    PCHAR *PrimaryBuffIndex; // 内存块索引
    PULONG BuffAlreadyUsedSpace; // 内存块已用空间
    struct { USHORT index; ULONG offset; } ReadIndex; // 读取的索引标记，指向未读的第一个位置
    struct { USHORT index; ULONG offset; } WriteIndex; // 写入的索引标记，指向未写的第一个位置
    StructStatus Status;
} MemoryAllocator;

StatusCode MemoryAllocator_FreeAllBuff(MemoryAllocator* self); // 释放所有内存块
StatusCode MemoryAllocator_NewBuffBlock(MemoryAllocator* self); // 新开辟一块内存块

StatusCode MemoryAllocator_Init(MemoryAllocator* self); // 初始化
StatusCode MemoryAllocator_CleanAll(MemoryAllocator* self); // 析构前清理空间
StatusCode MemoryAllocator_GetBuff(MemoryAllocator* self, PCHAR *buff, const ULONG needLength); // 获取一段空内存
StatusCode MemoryAllocator_ReadBuff(MemoryAllocator* self, PCHAR buff, const ULONG requireLength, PULONG pRealReadLength); // 继续读取一段内存
StatusCode MemoryAllocator_ResetBuff(MemoryAllocator* self); // 重置已用空间