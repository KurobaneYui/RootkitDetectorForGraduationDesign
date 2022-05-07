#include "stdafx.h"
#include "ProcessTree.h"

#include <iostream>
#include <string>
using namespace std;

StatusCode ProcessTree::PrintInfos(PCHAR buff, DWORD bufferLength)
{
    DWORD pointer{ 0 };
    ProcessInfoPackage*infopackage = (ProcessInfoPackage*)(buff + pointer);
    ThreadInfoPackage*infopackageThread = (ThreadInfoPackage*)(buff + pointer);
    while (bufferLength > pointer)
    {
        if (bufferLength - pointer < *(unsigned short*)(buff + pointer))
            return UNKNOWN;

        switch (*((unsigned short*)(buff + pointer) + 1))
        {
        case 1:
            infopackage = (ProcessInfoPackage*)(buff + pointer);
            printf("ProcessID:%u, ParentID:%u, ImageName:%s, CreateInfo:%ls\n\n", infopackage->pid, infopackage->parentPid, infopackage->imageName, (wchar_t*)&infopackage->path);
            break;
        case 2:
            infopackageThread = (ThreadInfoPackage*)(buff + pointer);
            printf("ThreadID:%u, ParentID:%u\n\n", infopackageThread->tid, infopackageThread->parentPid);
            break;
        default:
            cerr << "Meet error" << endl << endl;
            break;
        }
        pointer += *(unsigned short*)(buff + pointer);
    }
    return SUCCESS;
}