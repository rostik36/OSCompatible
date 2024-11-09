/**
* @file server.hpp
* @author Rostik
* @brief 
* @version 0.1
* @date 2024-11-08
* 
* @copyright Copyright (c) 2024
* 
*/
#ifndef _SERVER__
#define _SERVER__
// Or: #pragma once

// 1. Forward declarations (wherever possible)

// 2. Standard library headers

// 3. Third-party library headers

// 4. Other project headers (only if necessary)
#include "socket.hpp"
#include "client.hpp"


namespace OSCompatible
{


class server : public socket
{
public:
    inline server(int port);

    inline ~server() override = default;

    server(const server& o_) = delete; // CCtor
    server& operator=(const server& o_) = delete; // Op=

    inline bool bindSocket(int port);

    inline bool listenSocket(int backlog = 5);

    inline client acceptClient();
    inline client acceptClientWithTimeout(Time timeout);

    inline void stop();

private:
};

server::server(int port)
{
    if (createSocket() && bindSocket(port))
    {
        std::cout << "Server socket created and bound to port " << port << std::endl;
    }
    else
    {
        std::cerr << "Failed to create or bind server socket." << std::endl;
    }
}


client server::acceptClient()
{
    sockaddr_in clientAddr;
    socklen_t clientSize = sizeof(clientAddr);
    SOCKET clientSock = accept(sock, (sockaddr*)&clientAddr, &clientSize);
    // if (clientSock == INVALID_SOCKET_VALUE)
    // {
    //     std::cerr << "Error in acceptClient(): " << strerror(errno) << std::endl;
    // }
    return client(clientSock);
}


client server::acceptClient(Time timeout)
{
    sockaddr_in clientAddr;
    socklen_t clientSize = sizeof(clientAddr);

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    // Set timeout structure
    struct timeval timeout;
    timeout.tv_sec  = static_cast<size_t>(timeout)/1000;
    timeout.tv_usec = static_cast<size_t>(timeout)%1000; // the reminder of the seconds

    int selectResult;
#ifdef _WIN32
    selectResult = select(0, &readfds, nullptr, nullptr, &timeout);
#else
    selectResult = select(sock + 1, &readfds, nullptr, nullptr, &timeout);
#endif

    if (selectResult > 0 && FD_ISSET(sock, &readfds))
    {
        SOCKET clientSock = accept(sock, (sockaddr*)&clientAddr, &clientSize);
        if (clientSock == INVALID_SOCKET_VALUE)
        {
            std::cerr << "Error in acceptClientWithTimeout(): " << strerror(errno) << std::endl;
        }
        return client(clientSock);
    }
    else if (selectResult == 0)
    {
        std::cerr << "acceptClientWithTimeout() timed out." << std::endl;
    }
    else
    {
        std::cerr << "Error in select(): " << strerror(errno) << std::endl;
    }
    return client(INVALID_SOCKET_VALUE);
}


bool server::listenSocket(int backlog)
{
    return listen(sock, backlog) != SOCKET_ERROR;
}


bool server::bindSocket(int port)
{
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    return bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) != SOCKET_ERROR;
}


// void server:stop()
// {
//     running = false;       // Set flag to stop accepting new connections
//     closeSocket();         // Close the listening socket, which interrupts accept()
// }


}    // namespace OSCompatible


#endif //_SERVER__