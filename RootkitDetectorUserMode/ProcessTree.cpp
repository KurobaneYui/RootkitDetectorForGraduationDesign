#include "stdafx.h"
#include "ProcessTree.h"

using namespace std;

std::map<ULONG, ProcessTree*> ProcessTree::ProcessRecords = std::map<ULONG, ProcessTree*>();

ProcessTree::ProcessTree(ULONG pid)
{
    Status = NOT_ACCOMPLISHED;
    ThreadRecord.clear();
    ProcessRecords.insert(pair<ULONG, ProcessTree *>(pid, this));
}

ProcessTree::~ProcessTree()
{
    if (Status != NOT_ACCOMPLISHED)
        for(auto i : Info)
            if(i.path != nullptr)
                delete[] i.path;
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

StatusCode ProcessTree::AddProcess(ProcessInfoPackage *buffer, ULONG &pointer, int sourceID)
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
        ProcessTree *newProcess = new ProcessTree(buffer->pid);
        newProcess->ComplishProcessTree(buffer, sourceID);
    }
    else if (process->second->IsNotAccomplished(sourceID))
    {
        // complish process
        process->second->ComplishProcessTree(buffer, sourceID);
    }

    return SUCCESS;
}

StatusCode ProcessTree::AddProcess(PROCESSENTRY32 &proc, int sourceID)
{
    // search process
    auto process = ProcessRecords.find(proc.th32ProcessID);
    if (process == ProcessRecords.end())
    {
        // create new process
        ProcessTree *newProcess = new ProcessTree(proc.th32ProcessID);
        newProcess->ComplishProcessTree(proc, sourceID);
    }
    else if (process->second->IsNotAccomplished(sourceID))
    {
        // complish process
        process->second->ComplishProcessTree(proc, sourceID);
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

StatusCode ProcessTree::PrintInfos()
{
    for (auto i : ProcessRecords)
    {
        if (i.second->PrintSelf(i.first) != SUCCESS)
            return UNKNOWN;
    }

    return SUCCESS;
}

bool ProcessTree::IsNotAccomplished(int sourceID)
{
    return source[sourceID] == false;
}

bool ProcessTree::IsNotAccomplished()
{
    return Status == NOT_ACCOMPLISHED;
}

StatusCode ProcessTree::ComplishProcessTree(ProcessInfoPackage *buffer, int sourceID)
{
    if (buffer == nullptr || sourceID < 0 || sourceID > 3)
        return OUT_OF_RANGE;

    if (source[sourceID])
        return SUCCESS;

    Info[sourceID] = *buffer;
    USHORT pathLengthTmp = (Info[sourceID].length - sizeof(ProcessInfoPackage) + sizeof(char16_t *)) / sizeof(char16_t);
    char16_t *tmp = new char16_t[pathLengthTmp];
    Info[sourceID].path = tmp;
    memcpy(Info[sourceID].path, &buffer->path, pathLengthTmp * sizeof(char16_t));
    Status = NORMAL;
    source[sourceID] = true;

    return SUCCESS;
}

StatusCode ProcessTree::ComplishProcessTree(PROCESSENTRY32 &proc, int sourceID)
{
    if (sourceID < 0 || sourceID > 3)
        return OUT_OF_RANGE;

    if (source[sourceID])
        return SUCCESS;

    Info[sourceID].pid = proc.th32ProcessID;
    Info[sourceID].parentPid = proc.th32ParentProcessID;
    Info[sourceID].type = 1;
    char16_t *tmp = new char16_t[wcslen(proc.szExeFile) + 1];
    Info[sourceID].path = tmp;
	memset(tmp, 0, (wcslen(proc.szExeFile) + 1) * sizeof(char16_t));
	memcpy(Info[sourceID].path, proc.szExeFile, wcslen(proc.szExeFile) * sizeof(char16_t));
    memset(Info[sourceID].imageName, 0, sizeof(16));
    Info[sourceID].length = (USHORT)(sizeof(ProcessInfoPackage) - sizeof(char16_t *) + (wcslen(proc.szExeFile) + 1) * sizeof(char16_t));
    Status = NORMAL;
    source[sourceID] = true;

    return SUCCESS;
}

StatusCode ProcessTree::AddThread(ThreadInfoPackage &package)
{
    if (Status == ERROR_OCCURRED)
        return UNKNOWN;

    if (ThreadRecord.find(package) == ThreadRecord.end())
        ThreadRecord.insert(package);

    return SUCCESS;
}

StatusCode ProcessTree::SendSelf(ServerCommunicator &serverCommunicator)
{
    if (Status != NORMAL)
        return UNKNOWN;

    if (source[0] && source[1] && source[2])
        return SUCCESS;
    
    for (int i = 0; i < 3; i++)
    {
        if(source[i])
        {
            htonProcessInfoPackage(Info[i]);
            serverCommunicator.SendData(&Info[i], sizeof(Info[i]) - sizeof(char16_t *));
            serverCommunicator.SendData(Info[i].path, Info[i].length - sizeof(Info[i]) + sizeof(char16_t *));
            ntohProcessInfoPackage(Info[i]);
            break;
        }
    }
    //for (auto i : ThreadRecord)
    //{
    //    htonThreadInfoPackage(i);
    //    serverCommunicator.SendData(&i, i.length);
    //    ntohThreadInfoPackage(i);
    //}

    return SUCCESS;
}

StatusCode ProcessTree::PrintSelf(ULONG pid)
{
    if (Status != NORMAL)
        return UNKNOWN;

	for (int i = 0; i < 3; i++)
	{
		if (source[i])
			cout << "0-";
		else
			cout << "x-";
	}
    for (int i = 0; i < 3;i++)
    {
        if (source[i])
        {
            cout << "ProcessID: " << Info[i].pid << "    ParentProcessID: " << Info[i].parentPid
                 << "    Name: " << Info[i].imageName << "    CreatePath: ";
            printf("%ls\n", (wchar_t*)Info[i].path);
			break;
        }
    }
	if (!source[0] && !source[1] && !source[2])
		cout << "ProcessID: " << pid << " Not Found" << endl;
    //for(auto i : ThreadRecord)
    //{
    //    cout << "\tThreadID: " << i.tid << "    ParentProcessID: " << i.parentPid << endl;
    //}

    return SUCCESS;
}