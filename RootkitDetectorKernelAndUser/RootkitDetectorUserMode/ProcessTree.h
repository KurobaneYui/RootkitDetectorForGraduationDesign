#pragma once

class ProcessTree
{
private:
    static std::map<ULONG, ProcessTree*> ProcessRecords;
    ProcessInfoPackage Info[3]{};
    std::set<ThreadInfoPackage> ThreadRecord{};
    bool source[3]{false, false, false};
    enum
    {
        NORMAL,
        NOT_ACCOMPLISHED,
        ERROR_OCCURRED
    } Status;

public:
    ProcessTree(ULONG pid);
    ~ProcessTree();
    static void ClearAll();
    static ProcessTree* GetRoot();
    static StatusCode AddProcess(ProcessInfoPackage *buffer, ULONG &pointer, int sourceID);
    static StatusCode AddProcess(PROCESSENTRY32 &proc, int sourceID);
    static StatusCode AddThread(ThreadInfoPackage *buffer, ULONG &pointer);
    static StatusCode SendInfo(ServerCommunicator &serverCommunicator);
    static StatusCode PrintInfos();
    bool IsNotAccomplished(int sourceID);
    bool IsNotAccomplished();
    StatusCode ComplishProcessTree(ProcessInfoPackage *buffer, int sourceID);
    StatusCode ComplishProcessTree(PROCESSENTRY32 &proc, int sourceID);
    StatusCode AddThread(ThreadInfoPackage &package);
    StatusCode SendSelf(ServerCommunicator &serverCommunicator);
    StatusCode PrintSelf(ULONG pid);
};