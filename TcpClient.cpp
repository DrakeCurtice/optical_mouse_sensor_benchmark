#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <winsock2.h>
#include <ws2tcpip.h>

#include "TcpClient.h"

#pragma comment(lib, "Ws2_32.lib")

bool TcpClient::sendJson(
    const std::string& json,
    const std::string& host,
    unsigned short port) const
{
    WSADATA wsaData{};

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return false;
    }

    SOCKET socketHandle =
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (socketHandle == INVALID_SOCKET)
    {
        WSACleanup();
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(
        AF_INET,
        host.c_str(),
        &address.sin_addr) != 1)
    {
        closesocket(socketHandle);
        WSACleanup();
        return false;
    }

    if (connect(
        socketHandle,
        reinterpret_cast<sockaddr*>(&address),
        sizeof(address)) == SOCKET_ERROR)
    {
        closesocket(socketHandle);
        WSACleanup();
        return false;
    }

    const std::string message = json + "\n";
    std::size_t totalSent = 0;

    while (totalSent < message.size())
    {
        const int sent = send(
            socketHandle,
            message.data() + totalSent,
            static_cast<int>(message.size() - totalSent),
            0
        );

        if (sent <= 0)
        {
            closesocket(socketHandle);
            WSACleanup();
            return false;
        }

        totalSent += static_cast<std::size_t>(sent);
    }

    closesocket(socketHandle);
    WSACleanup();
    return true;
}
