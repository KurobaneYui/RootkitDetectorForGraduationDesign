// RootkitDetectorUserMode.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using std::cout;
using std::endl;

int main()
{
    HANDLE HDevice = NULL;
    CHAR buff[1024] = { 0 };
    DWORD readlen = 0;

    system("pause");

    HDevice = CreateFile((LPCWSTR)L"\\\\.\\RootkitDetectorDevice", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (HDevice == INVALID_HANDLE_VALUE)
    {
        printf("Create Handle Failure");

        system("pause");
        return -1;
    }
    printf("Handle Create successfully");
    system("pause");

    ReadFile(HDevice, (PVOID)buff, 1024, &readlen, NULL);
    cout << buff << ' ' << readlen << endl;
    system("pause");

    DeviceIoControl(HDevice, IOCTL_SNAPSHOT, /*&input*/NULL, 4, /*&output*/NULL, 4, &readlen, NULL);
    system("pause");

    CloseHandle(HDevice);
    system("pause");

    return 0;
}