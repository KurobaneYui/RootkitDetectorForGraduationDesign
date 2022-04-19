#pragma once
// This header file is for Driver and User Application

#define IOCTL_SNAPSHOT CTL_CODE(FILE_DEVICE_UNKNOWN,0x9831,METHOD_BUFFERED,FILE_ANY_ACCESS)

// At least 12 bytes -- 2*sizeof(USHORT)+2*sizeof(LONG). If path is NULL, never copy to user layer
typedef struct _ProcessInfoPackage
{
    unsigned short length; // In byte
    unsigned short type; // process: 1
    unsigned long pid;
    unsigned long parentPid;
    // bellow should be WCHAR. WCHAR = wchar_t = unsigned short
    char* path; // At the end of string must be '\0', unless no charactor in it
} ProcessInfoPackage;

// 12 bytes -- 3*sizeof(LONG)
typedef struct _ThreadInfoPackage
{
    unsigned short length; // In bype
    unsigned short type; // thread: 2
    unsigned long tid;
    unsigned long parentPid;
} ThreadInfoPackage;

typedef enum _StatusCode
{
    SUCCESS,
    OUT_OF_RANGE,
    NO_MEMORY,
    HAVE_DESTROYED,
    UNKNOWN
} StatusCode;

typedef enum _StructStatus
{
    NORMAL,
    DESTROYED
} StructStatus;