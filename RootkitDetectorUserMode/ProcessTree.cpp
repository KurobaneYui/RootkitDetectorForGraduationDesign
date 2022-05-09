#include "stdafx.h"
#include "ProcessTree.h"

using namespace std;

ProcessTree::ProcessTree(ULONG pid)
{
    Status = NOT_ACCOMPLISHED;
    ThreadRecord.clear();
    ProcessRecords.insert(pair<ULONG, ProcessTree *>(pid, this));
}

ProcessTree::ProcessTree(ProcessInfoPackage *buffer)
{
    Info = *buffer;
    USHORT pathLengthTmp = (Info.length - sizeof(ProcessInfoPackage) + sizeof(char16_t *)) / sizeof(char16_t);
    char16_t *tmp = new char16_t[pathLengthTmp];
    Info.path = tmp;
    memcpy(Info.path, &buffer->path, pathLengthTmp * sizeof(char16_t));
    Status = NORMAL;
    ProcessRecords.insert(pair<ULONG, ProcessTree *>(Info.pid, this));
}

ProcessTree::~ProcessTree()
{
    if (Status != NOT_ACCOMPLISHED)
        delete[] Info.path;
}

void ProcessTree::ClearAll()
{
    for (auto i : ProcessRecords)
    {
        delete i.second;
    }
    ProcessRecords.clear();
}

ProcessTree *ProcessTree::GetRoot()
{
    if (ProcessRecords.find(4) != ProcessRecords.end())
        return ProcessRecords[4];

    return nullptr;
}

StatusCode ProcessTree::AddProcess(ProcessInfoPackage *buffer, ULONG &pointer)
{
    // check parameters
    if (buffer == nullptr)
        return OUT_OF_RANGE;

    // move pointer
    pointer += buffer->length;

    // search process
    auto process = ProcessRecords.find(buffer->pid);
    if (process == ProcessRecords.end())
    {
        // create new process
        ProcessTree *newProcess = new ProcessTree(buffer);
    }
    else if (process->second->IsNotAccomplished())
    {
        // complish process
        process->second->ComplishProcessTree(buffer);
    }

    return SUCCESS;
}

StatusCode ProcessTree::AddThread(ThreadInfoPackage *buffer, ULONG &pointer)
{
    // check parameters
    if (buffer == nullptr)
        return UNKNOWN;

    // get package
    ThreadInfoPackage pack = *buffer;

    // move pointer
    pointer += pack.length;

    // search parent process and add thread
    auto parentProcess = ProcessRecords.find(pack.parentPid);
    if (parentProcess == ProcessRecords.end())
    {
        // create new process and add thread
        ProcessTree *pProcess = new ProcessTree(pack.parentPid);
        return pProcess->AddThread(pack);
    }
    else
    {
        // add thread
        return parentProcess->second->AddThread(pack);
    }
}

StatusCode ProcessTree::SendInfo(ServerCommunicator &serverCommunicator)
{
    for (auto i : ProcessRecords)
    {
        if (i.second->IsNotAccomplished())
            return UNKNOWN;

        if (i.second->SendSelf(serverCommunicator) != SUCCESS)
            return UNKNOWN;
    }

    return SUCCESS;
}

bool ProcessTree::IsNotAccomplished()
{
    return Status == NOT_ACCOMPLISHED;
}

StatusCode ProcessTree::ComplishProcessTree(ProcessInfoPackage *buffer)
{
    if (buffer == nullptr)
        return OUT_OF_RANGE;

    if (Status != NOT_ACCOMPLISHED)
        return UNKNOWN;

    Info = *buffer;
    USHORT pathLengthTmp = (Info.length - sizeof(ProcessInfoPackage) + sizeof(char16_t *)) / sizeof(char16_t);
    char16_t *tmp = new char16_t[pathLengthTmp];
    Info.path = tmp;
    memcpy(Info.path, &buffer->path, pathLengthTmp * sizeof(char16_t));
    Status = NORMAL;

    return SUCCESS;
}

StatusCode ProcessTree::AddThread(ThreadInfoPackage &package)
{
    if (Status == ERROR_OCCURRED)
        return UNKNOWN;

    if (Status == NORMAL && package.parentPid != Info.pid)
        return UNKNOWN;

    if (ThreadRecord.find(package) == ThreadRecord.end())
        ThreadRecord.insert(package);

    return SUCCESS;
}

StatusCode ProcessTree::SendSelf(ServerCommunicator &serverCommunicator)
{// TODO: change data from host byte order to network byte order
    if (Status != NORMAL)
        return UNKNOWN;

    serverCommunicator.SendData(&Info, sizeof(Info) - sizeof(char16_t*));
    serverCommunicator.SendData(Info.path, Info.length - sizeof(Info) + sizeof(char16_t*));

    for (auto i : ThreadRecord)
    {
        serverCommunicator.SendData(&i, i.length);
    }

    return SUCCESS;
}

// StatusCode ProcessTree::PrintInfos(PCHAR buff, DWORD bufferLength)
// {
//     DWORD pointer{ 0 };
//     ProcessInfoPackage*infopackage = (ProcessInfoPackage*)(buff + pointer);
//     ThreadInfoPackage*infopackageThread = (ThreadInfoPackage*)(buff + pointer);
//     while (bufferLength > pointer)
//     {
//         if (bufferLength - pointer < *(unsigned short*)(buff + pointer))
//             return UNKNOWN;

//         switch (*((unsigned short*)(buff + pointer) + 1))
//         {
//         case 1:
//             infopackage = (ProcessInfoPackage*)(buff + pointer);
//             printf("ProcessID:%u, ParentID:%u, ImageName:%s, CreateInfo:%ls\n\n", infopackage->pid, infopackage->parentPid, infopackage->imageName, (wchar_t*)&infopackage->path);
//             break;
//         case 2:
//             infopackageThread = (ThreadInfoPackage*)(buff + pointer);
//             printf("ThreadID:%u, ParentID:%u\n\n", infopackageThread->tid, infopackageThread->parentPid);
//             break;
//         default:
//             cerr << "Meet error" << endl << endl;
//             break;
//         }
//         pointer += *(unsigned short*)(buff + pointer);
//     }
//     return SUCCESS;
// }
