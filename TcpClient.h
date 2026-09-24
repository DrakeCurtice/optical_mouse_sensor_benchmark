#pragma once

#include <string>

class TcpClient
{
public:
    bool sendJson(
        const std::string& json,
        const std::string& host = "127.0.0.1",
        unsigned short port = 9000
    ) const;
};
