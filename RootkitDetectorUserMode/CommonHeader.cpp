#include "stdafx.h"
#include "CommonHeader.h"

bool operator< (const ThreadInfoPackage &a, const ThreadInfoPackage &b) {
    return a.tid < b.tid;
}

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