#pragma once
#include "stdafx.h"
#include <map>
#include <string>
#include <set>

class ProcessTree
{
private:
    std::map<unsigned long, int> a;

public:
    static StatusCode PrintInfos(PCHAR buff, DWORD bufferLength);
};