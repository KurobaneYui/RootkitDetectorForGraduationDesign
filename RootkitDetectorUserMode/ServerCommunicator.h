#pragma once
#include "stdafx.h"
#include "CommonHeader.h"

#define SERVER_IP "152.136.254.151"
#define SERVER_PORT 5555
#define DEVICE_ID 0xF8523679

enum ControlSignal
{
    SNAPSHOT,
    CLOSE_CONNECTION,
    CONTROL_UNKNOWN
};

class ServerCommunicator
{
public:
    ServerCommunicator();
    ~ServerCommunicator();
    StatusCode Authorize(UINT32 userID, UINT32 token, bool &status);
    StatusCode ReceiveControlSignal(ControlSignal &controlSignal);
    StatusCode SendData(PVOID buffer, UINT bufferLength);

private:
    WSADATA wsaData;
    SOCKET ServerConnector;
    SOCKADDR_IN ServerAddress;
    UINT32 UserID;
    UINT32 Token;
    UINT32 DeviceID;
    enum
    {
        CONNECTION_ESTABLISHED,
        AUTHORIZED,
        NEED_CLOSE,
        ERROR_OCCUR
    } Status;
};