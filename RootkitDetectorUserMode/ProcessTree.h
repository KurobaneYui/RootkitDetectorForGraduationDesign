#pragma once
#include "stdafx.h"
#include <map>
#include <string>
#include <set>
#include <vector>
#include <iostream>

class ProcessTree
{
private:
    static std::map<ULONG, ProcessTree*> ProcessRecords;
    ProcessInfoPackage Info;
    std::set<ThreadInfoPackage> ThreadRecord;
    enum
    {
        NORMAL,
        NOT_ACCOMPLISHED,
        ERROR_OCCURRED
    } Status;

public:
    ProcessTree(ULONG pid);
    ProcessTree(ProcessInfoPackage* buffer);
    ~ProcessTree();
    static void ClearAll();
    static ProcessTree* GetRoot();
    static StatusCode AddProcess(ProcessInfoPackage *buffer, ULONG &pointer);
    static StatusCode AddThread(ThreadInfoPackage *buffer, ULONG &pointer);
    static StatusCode SendInfo(ServerCommunicator &serverCommunicator);
    //static StatusCode PrintInfos();
    bool IsNotAccomplished();
    StatusCode ComplishProcessTree(ProcessInfoPackage *buffer);
    StatusCode AddThread(ThreadInfoPackage &package);
    StatusCode SendSelf(ServerCommunicator &serverCommunicator);
};