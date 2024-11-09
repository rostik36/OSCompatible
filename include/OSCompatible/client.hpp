/**
* @file client.hpp
* @author Rostik
* @brief 
* @version 0.1
* @date 2024-11-08
* 
* @copyright Copyright (c) 2024
* 
*/
#ifndef _CLIENT__
#define _CLIENT__
// Or: #pragma once

// 1. Forward declarations (wherever possible)

// 2. Standard library headers

// 3. Third-party library headers

// 4. Other project headers (only if necessary)
#include "socket.hpp"


namespace OSCompatible
{


class client : public socket 
{
public:
    inline client(const std::string& serverIp, int port);
    inline ~client() override = default;

    client(const client& o_) = delete; // CCtor
    client& operator=(const client& o_) = delete; // Op=


    inline bool connectToServer(const std::string& serverIp, int port);
private:
    
};



client::client(const std::string& serverIp, int port)
{
    if (createSocket() && connectToServer(serverIp, port))
    {
        std::cout << "Connected to server at " << serverIp << ":" << port << std::endl;
    }
    else
    {
        std::cerr << "Failed to connect to server." << std::endl;
    }
}


bool client::connectToServer(const std::string& serverIp, int port)
{
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);

    return connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) != SOCKET_ERROR;
}

} // namespace OSCompatible


#endif //_CLIENT__