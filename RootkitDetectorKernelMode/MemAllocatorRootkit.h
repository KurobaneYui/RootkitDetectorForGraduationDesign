#pragma once

#include "CommonHeader.h"
#include <ntddk.h>

#define CAPICITY_OF_ONE_BUFF (1024*128)
#define NUM_OF_BUFF 40
#define ROOTKIT_DETECTOR_TAG 'detc'

// 负责驱动遍历进程、线程所需内存的分配和释放
class MemoryAllocator
{
private:
    PCHAR *PrimaryBuffIndex; // 内存块索引
    PULONG BuffAlreadyUsedSpace; // 内存块已用空间
    struct { USHORT index; ULONG offset; } ReadIndex; // 读取的索引标记，指向未读的第一个位置
    struct { USHORT index; ULONG offset; } WriteIndex; // 写入的索引标记，指向未写的第一个位置
    enum { NORMAL, DESTROYED } Status;

    _StatusCode FreeAllBuff(); // 释放所有内存块
    _StatusCode NewBuffBlock(); // 新开辟一块内存块

public:
    MemoryAllocator() = default;
    ~MemoryAllocator() = default;
    _StatusCode Init(); // 初始化
    _StatusCode CleanAll(); // 析构前清理空间
    _StatusCode GetBuff(PCHAR &buff, const ULONG needLength); // 获取一段空内存
    _StatusCode ReadBuff(PCHAR buff, const ULONG requireLength, ULONG &realReadLength); // 继续读取一段内存
    _StatusCode ResetBuff(); // 重置已用空间
};