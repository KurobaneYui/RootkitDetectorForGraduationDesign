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
            cout << "ProcessID:" << infopackage->pid << ' ' << "ParentID:" << infopackage->parentPid << ' ' << "imageName:" << string((char*)infopackage->imageName) << ' ' << "CreateInfo:" << infopackage->path << endl << endl;
            break;
        case 2:
            infopackageThread = (ThreadInfoPackage*)(buff + pointer);
            cout << "ThreadID:" << infopackageThread->tid << ' ' << "ParentID:" << infopackageThread->parentPid << endl << endl;
            break;
        default:
            cerr << "Meet error" << endl << endl;
            break;
        }
        pointer += *(unsigned short*)(buff + pointer);
    }
}