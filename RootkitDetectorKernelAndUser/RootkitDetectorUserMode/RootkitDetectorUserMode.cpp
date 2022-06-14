// RootkitDetectorUserMode.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using std::cout;
using std::cerr;
using std::endl;

int main()
{
    setlocale(LC_ALL, "");
    bool status;
	ControlSignal signal{};

    DriverCommunicator Connector2Device = DriverCommunicator(1024 * 4);
    ServerCommunicator testConnector = ServerCommunicator();
    testConnector.Authorize(1322, 3456666, status);

	if (!status)
	{
		cerr << "Auth error" << endl;
		return 0;
	}

	while (testConnector.ReceiveControlSignal(signal) == SUCCESS)
	{
		if (signal == SNAPSHOT)
		{
			Connector2Device.SwitchDetector(0);
			Connector2Device.Snapshot();
			Connector2Device.GetInfo();
			Connector2Device.SwitchDetector(1);
			Connector2Device.Snapshot();
			Connector2Device.GetInfo();
			Connector2Device.SwitchDetector(2);
			Connector2Device.Snapshot();
			Connector2Device.GetInfo();

			ProcessTree::SendInfo(testConnector);
			ProcessTree::PrintInfos();
			cout << endl << endl;
			ProcessTree::ClearAll();
		}
		else if (signal == CLOSE_CONNECTION)
		{
			cout << "Connection closed by remote" << endl;
			break;
		}
		else
		{
			cerr << "Control Signal error" << endl;
			return 0;
		}
	}

    system("pause");

    return 0;
}