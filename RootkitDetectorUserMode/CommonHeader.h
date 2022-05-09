#pragma once
// This header file is for Driver and User Application
#include <cuchar>

#define IOCTL_SNAPSHOT CTL_CODE(FILE_DEVICE_UNKNOWN,0x9831,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SWITCH CTL_CODE(FILE_DEVICE_UNKNOWN,0x9832,METHOD_BUFFERED,FILE_ANY_ACCESS)

// At least 30 bytes -- 2*sizeof(USHORT)+2*sizeof(LONG)+16*sizeof(unsigned char)+sizeof(char16_t). If path is NULL, never copy to user layer
typedef struct _ProcessInfoPackage
{
    unsigned short length; // In byte
    unsigned short type; // process: 1
    unsigned long pid;
    unsigned long parentPid;
    unsigned char imageName[16];
    char16_t *path; // At the end of string must be '\0'. it's '\0', if no charactor in it
} ProcessInfoPackage;

// 12 bytes -- 3*sizeof(LONG)
typedef struct _ThreadInfoPackage
{
    unsigned short length; // In bype
    unsigned short type; // thread: 2
    unsigned long tid;
    unsigned long parentPid;
} ThreadInfoPackage;

void htonProcessInfoPackage(ProcessInfoPackage &package)
{
    package.length = htons(package.length);
    package.type = htons(package.type);
    package.pid = htonl(package.pid);
    package.parentPid = htonl(package.parentPid);
    USHORT length = package.length - sizeof(ProcessInfoPackage) + sizeof(char16_t*);
    for (int i = 0; i < length / 2; i++)
    {
        *(package.path + i) = htons(*(package.path + i));
    }
}

void ntohProcessInfoPackage(ProcessInfoPackage &package)
{
    package.length = ntohs(package.length);
    package.type = ntohs(package.type);
    package.pid = ntohl(package.pid);
    package.parentPid = ntohl(package.parentPid);
    USHORT length = package.length - sizeof(ProcessInfoPackage) + sizeof(char16_t*);
    for (int i = 0; i < length / 2; i++)
    {
        *(package.path + i) = ntohs(*(package.path + i));
    }
}

void htonThreadInfoPackage(ThreadInfoPackage &package)
{
    package.length = htons(package.length);
    package.type = htons(package.type);
    package.tid = htonl(package.tid);
    package.parentPid = htonl(package.parentPid);
}

void ntohThreadInfoPackage(ThreadInfoPackage &package)
{
    package.length = ntohs(package.length);
    package.type = ntohs(package.type);
    package.tid = ntohl(package.tid);
    package.parentPid = ntohl(package.parentPid);
}

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