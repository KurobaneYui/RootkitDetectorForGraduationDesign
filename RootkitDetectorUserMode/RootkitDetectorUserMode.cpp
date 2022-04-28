// RootkitDetectorUserMode.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using std::cout;
using std::endl;

int main()
{
    ProcessTree tree = ProcessTree();
    DriverCommunicator Connector2Device = DriverCommunicator(1024 * 1024);

    Connector2Device.Snapshot();
    Connector2Device.SwitchDetector(1);
    Connector2Device.GetInfo(tree);

    system("pause");

    return 0;
}