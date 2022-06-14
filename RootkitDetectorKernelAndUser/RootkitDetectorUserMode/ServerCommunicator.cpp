#include "stdafx.h"
#include "ServerCommunicator.h"

ServerCommunicator::ServerCommunicator()
{
    Status = ERROR_OCCUR;

    // 启用WinSocket
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return;
    // 校验版本
    if (2 != HIBYTE(wsaData.wVersion) || 2 != LOBYTE(wsaData.wVersion))
        return;

    // 建立连接
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &ServerAddress.sin_addr.S_un.S_addr);
    ServerConnector = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ServerConnector == INVALID_SOCKET)
        return;
    // Set socket as block mode;
    u_long iMode = 0;
    if (ioctlsocket(ServerConnector, FIONBIO, &iMode) == SOCKET_ERROR)
    {
        closesocket(ServerConnector);
        return;
    }
    // Try to connect to the Server
    if (connect(ServerConnector, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
    {
        closesocket(ServerConnector);
        return;
    }

    Status = CONNECTION_ESTABLISHED;
    return;
}

ServerCommunicator::~ServerCommunicator()
{
    closesocket(ServerConnector);
    WSACleanup();
}

StatusCode ServerCommunicator::Authorize(UINT32 userID, UINT32 token, bool &status)
{
    if (Status != CONNECTION_ESTABLISHED)
    {
        status = false;
        return UNKNOWN;
    }

    char buff[14];

    userID = htonl(userID);
    token = htonl(token);
    UINT32 deviceID = htonl(DEVICE_ID);

    memcpy(buff, &userID, sizeof(userID));
    memcpy(buff + 5, &token, sizeof(token));
    memcpy(buff + 10, &deviceID, sizeof(deviceID));
    buff[4] = ':';
    buff[9] = ':';

    if (send(ServerConnector, buff, 14, 0) == SOCKET_ERROR)
    {
        Status = ERROR_OCCUR;
        closesocket(ServerConnector);
        return UNKNOWN;
    }
    if (recv(ServerConnector, buff, 1, 0) == SOCKET_ERROR)
    {
        Status = ERROR_OCCUR;
        closesocket(ServerConnector);
        return UNKNOWN;
    }

    status = (buff[0] == 1);
	if (status)
		Status = AUTHORIZED;
	else
		Status = CONNECTION_ESTABLISHED;
    return SUCCESS;
}

StatusCode ServerCommunicator::ReceiveControlSignal(ControlSignal &controlSignal)
{
    if (Status != AUTHORIZED)
    {
        controlSignal = CONTROL_UNKNOWN;
        return UNKNOWN;
    }

    UCHAR control;
    if (recv(ServerConnector, (PCHAR)&control, 1, 0) == SOCKET_ERROR)
    {
        Status = ERROR_OCCUR;
        closesocket(ServerConnector);
        return UNKNOWN;
    }

    controlSignal = (ControlSignal)control;
	char tmp = 1;
	SendData(&tmp, 1);
    return SUCCESS;
}

StatusCode ServerCommunicator::SendData(PVOID buffer, UINT bufferLength)
{
    if (Status != AUTHORIZED)
        return UNKNOWN;

    if (buffer == nullptr)
        return OUT_OF_RANGE;

    if (send(ServerConnector, (PCHAR)buffer, bufferLength, 0) == SOCKET_ERROR)
    {
        Status = ERROR_OCCUR;
        closesocket(ServerConnector);
        return UNKNOWN;
    }

    return SUCCESS;
}

StatusCode ServerCommunicator::ReceiveState(UINT &state)
{
	if (Status != AUTHORIZED)
		return UNKNOWN;

	recv(ServerConnector, (PCHAR)&state, 1, 0);

	return SUCCESS;
}