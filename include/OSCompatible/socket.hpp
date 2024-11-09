/**
* @file socket.hpp
* @author Rostik
* @brief 
* @version 0.4
* @date 2024-11-08
* 
* @copyright Copyright (c) 2024
* 
*/
#ifndef _SOCKET__
#define _SOCKET__
// Or: #pragma once

// 1. Forward declarations (wherever possible)

// 2. Standard library headers
#include <iostream>
#include <string>
#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <poll.h>
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR   (-1)
    typedef int SOCKET;
#endif
// 3. Third-party library headers

// 4. Other project headers (only if necessary)
#include "definitions.h"


namespace OSCompatible
{

class socket
{
public:
    
    // Constructor
    inline socket(int port = INVALID_SOCKET);

    // Destructor
    inline virtual ~socket();

    // socket(const socket& o_) = delete; // CCtor
    // socket& operator=(const socket& o_) = delete; // Op=



    // Cleanup (for Windows)
    inline void cleanup();

    // Create a socket
    inline bool createSocket();

    // Connect to server
    //inline bool connectToServer(const std::string &ip, int port);

    // Send data
    inline bool sendData(const std::string &data) const;

    // Receive data
    /**
     * @brief function to receive data, blocks until received data uses inner buffer
     * 
     * @return std::string
     */
    inline std::shared_ptr<std::string> receiveData(size_t bufferSize = BufferSize::Default) const;
    inline std::shared_ptr<std::string> receiveDataWithTimeout(size_t bufferSize = BufferSize::Default, Time timeout = Time::Infinite) const;


    /**
     * @brief function to receive data, blocks until received data uses inner buffer
     * 
     * @param buffer 
     * @param bufferSize 
     * @return int bytes received
     */
    inline int receiveData(char* buffer, size_t bufferSize) const;
    inline int receiveDataWithTimeout(char* buffer, size_t bufferSize = BufferSize::Default, Time timeout = Time::Infinite) const;


    // Close socket
    inline void closeSocket();

protected:
    /**
     * @brief Initializes for Windows socket
     * 
     */
    inline bool initialize();

    /**
     * @brief Get the Socket object
     * 
     * @return SOCKET 
     */
    inline SOCKET getSocket() const;


    SOCKET sock;
    sockaddr_in serverAddr;


private:
    int setTimeout(int timeout) const;

};


socket::socket(int port) : sock(port)
{}

socket::~socket()
{
    closeSocket();
    cleanup();
}

bool socket::initialize()
{
#ifdef _WIN32
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
    return true;
#endif
}


void socket::cleanup()
{
#ifdef _WIN32
    WSACleanup();
#endif
}


bool socket::createSocket()
{
    sock = ::socket(AF_INET, SOCK_STREAM, 0); // take socket from global scope the real socket(windows/linux)
    // Set the SO_REUSEADDR option
    int opt = 1;
    if (sock != INVALID_SOCKET && setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "Failed to set SO_REUSEADDR option." << std::endl;
        close(sock);
        return false;
    }
    return sock != INVALID_SOCKET;
}


// bool socket::connectToServer(const std::string &ip, int port)
// {
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(port);
// #ifdef _WIN32
//     serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
// #else
//     inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);
// #endif
//     return connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) != SOCKET_ERROR;
// }


bool socket::sendData(const std::string &data) const
{
    return send(sock, data.c_str(), data.size(), 0) != SOCKET_ERROR;
}


std::shared_ptr<std::string> socket::receiveData(size_t bufferSize) const
{
    auto buffer = std::make_shared<std::string>(bufferSize, '\0');

    int receivedBytes = recv(sock, buffer->data(), bufferSize, 0);
    if (receivedBytes <= 0)
    {
        return nullptr;
    }
    buffer->resize(receivedBytes); // make the string buffer smaller so all the trailing \0 not exists and string compare works properly
    return buffer;
}

int socket::receiveData(char* buffer, size_t bufferSize) const
{
    int receivedBytes = recv(sock, buffer, bufferSize, 0);

    return receivedBytes;
}


std::shared_ptr<std::string> socket::receiveDataWithTimeout(size_t bufferSize, Time timeout) const
{
    if(setTimeout(static_cast<int>(timeout)) != 0)
    {
        return std::make_shared<std::string>();
    }
    return receiveData(bufferSize);
}

int socket::receiveDataWithTimeout(char* buffer, size_t bufferSize, Time timeout) const
{
    if(setTimeout(static_cast<int>(timeout)) != 0)
    {
        return -1; // error
    }
    return receiveData(buffer, bufferSize);
}



void socket::closeSocket()
{
    if (sock != INVALID_SOCKET)
    {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        sock = INVALID_SOCKET;
    }
}


SOCKET socket::getSocket() const
{
    return sock;
}


int socket::setTimeout(int timeout_ms) const
{
#ifdef _WIN32
        if (timeout_ms >= 0)
        { // Set timeout only if it's non-negative
            DWORD timeout = static_cast<DWORD>(timeout_ms);
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        }
#else
        if (timeout_ms >= 0)
        { // Use poll() for timeout on Linux
            struct pollfd fds;
            fds.fd = sock;
            fds.events = POLLIN;
            
            int pollResult = poll(&fds, 1, timeout_ms);
            if (pollResult == 0)
            {
                //std::cerr << "Receive timed out." << std::endl;
                return 1; // timeout
            }
            else if (pollResult < 0)
            {
                //std::cerr << "Poll error." << std::endl;
                return 2; // poll error
            }
        }
        return 0; // no timeout something received
#endif
}


} // namespace OSCompatible


#endif //_SOCKET__
