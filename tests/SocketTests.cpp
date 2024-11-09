/**
 * @file SocketTests.cpp
 * @author Rostik
 * @brief 
 * @version 0.4
 * @date 2024-11-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include "OSCompatible.h"


// TEST(ProcessAllEvents, BasicAssertions)
// {
//     Socket clientSocket;
//     if (!clientSocket.initialize())
//     {
//         std::cerr << "Failed to initialize socket library" << std::endl;
//         return -1;
//     }

//     if (!clientSocket.createSocket())
//     {
//         std::cerr << "Failed to create socket" << std::endl;
//         clientSocket.cleanup();
//         return -1;
//     }

//     if (!clientSocket.connectToServer("127.0.0.1", 5555))
//     {
//         std::cerr << "Failed to connect to server" << std::endl;
//         clientSocket.cleanup();
//         return -1;
//     }

//     clientSocket.sendData("Hello, Server!");
//     std::string response = clientSocket.receiveData();

//     std::cout << "Received from server: " << response << std::endl;

//     clientSocket.closeSocket();
//     clientSocket.cleanup();

// }

void handleClient(OSCompatible::socket clientSocket)
{
    std::shared_ptr<std::string> data = clientSocket.receiveData(1024);
    std::cout << "Received from client: " << data << std::endl;
    clientSocket.sendData("Hello from server!");
}


TEST(ClientConnectToServerAndSendData, BasicAssertions)
{
    std::string message = "Hello, world!";
    OSCompatible::server server(5555);

    if (server.listenSocket())
    {
        std::cout << "Server listening on port 5555..." << std::endl;

        //lunch client
        OSCompatible::client client("127.0.0.1", 5555);

        client.sendData(message);

        while (true)
        {
            OSCompatible::socket clientSocket = server.acceptClient();
            std::shared_ptr<std::string> data = clientSocket.receiveData();

            EXPECT_EQ(true, message == *data );

            break;
        }
        server.closeSocket();
        server.cleanup();
        client.closeSocket();
        client.cleanup();
    }
    else
    {
        EXPECT_EQ(false, true)<< "failed to connect";
        std::cerr << "Failed to start server." << std::endl;
    }

}



TEST(ClientConnectToServerSendDataBackAndForward, BasicAssertions)
{
    std::string messageToServer = "Hello, Server!";
    std::string messageToClient = "Hello, Client!";

    OSCompatible::server server(5555);

    if (server.listenSocket())
    {
        std::cout << "Server listening on port 5555..." << std::endl;

        //lunch client
        OSCompatible::client client("127.0.0.1", 5555);

        client.sendData(messageToServer);

        while (true)
        {
            OSCompatible::socket clientSocket = server.acceptClient();
            std::shared_ptr<std::string> dataFromClient = clientSocket.receiveData();

            EXPECT_EQ(messageToServer, *dataFromClient );

            clientSocket.sendData(messageToClient);

            std::shared_ptr<std::string> dataFromServer = client.receiveData();

            EXPECT_EQ(messageToClient, *dataFromServer );

            break;
        }

    }
    else
    {
        EXPECT_EQ(false, true)<< "failed to connect";
        std::cerr << "Failed to start server." << std::endl;
    }
}





void ClientThread()
{
    std::string messageToServer = "Hello, Server!";
    std::string messageToClient = "Hello, Client!";
    OSCompatible::client client("127.0.0.1", 5555);
    int cycle = 20;

    while(cycle-- > 0)
    {
        std::cout << "Client cycle: " << cycle << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(750));

        client.sendData(messageToServer); // 750 ms

        std::shared_ptr<std::string> dataFromServer;

        dataFromServer = client.receiveDataWithTimeout(OSCompatible::BufferSize::Default ,OSCompatible::Time::Milliseconds_500);

        EXPECT_EQ("", *dataFromServer ); // still not received 500 ms

        dataFromServer = client.receiveDataWithTimeout(OSCompatible::BufferSize::Default ,OSCompatible::Time::Milliseconds_500);

        EXPECT_EQ(messageToClient, *dataFromServer ); // here received 750 ms

    }
}




TEST(ReceiveDataWithTimeOut, BasicAssertions)
{
    std::string messageToServer = "Hello, Server!";
    std::string messageToClient = "Hello, Client!";

    OSCompatible::server server(5555);

    if (server.listenSocket())
    {
        std::cout << "Server listening on port 5555..." << std::endl;

        std::thread th = std::thread(ClientThread);
        OSCompatible::socket clientSocket = server.acceptClient();

        int cycle = 20;
        std::shared_ptr<std::string> dataFromClient;

        while(cycle-- > 0)
        {
            std::cout << "Server cycle: " << cycle << std::endl;

            dataFromClient = clientSocket.receiveDataWithTimeout(OSCompatible::BufferSize::Default ,OSCompatible::Time::Milliseconds_500);

            EXPECT_EQ("", *dataFromClient ); // here still not received 500 ms

            dataFromClient = clientSocket.receiveDataWithTimeout(OSCompatible::BufferSize::Default ,OSCompatible::Time::Milliseconds_500);

            EXPECT_EQ(messageToServer, *dataFromClient ); // here received 750 ms

            std::this_thread::sleep_for(std::chrono::milliseconds(600)); 

            clientSocket.sendData(messageToClient); // send on 600 ms
        }

        th.join();
    }
    else
    {
        EXPECT_EQ(false, true)<< "failed to connect";
        std::cerr << "Failed to start server." << std::endl;
    }
}










TEST(ReceiveDataWithTimeOut, BasicAssertions)
{
    std::string messageToServer = "Hello, Server!";
    std::string messageToClient = "Hello, Client!";

    OSCompatible::server server(5555);

    if (server.listenSocket())
    {
        std::cout << "Server listening on port 5555..." << std::endl;

        std::thread th = std::thread(ClientThread);
        OSCompatible::socket clientSocket = server.acceptClientWithTimeout(OSCompatible::Time::Milliseconds_100);

        int cycle = 20;
        std::shared_ptr<std::string> dataFromClient;

        while(cycle-- > 0)
        {
            std::cout << "Server cycle: " << cycle << std::endl;

            dataFromClient = clientSocket.receiveDataWithTimeout(OSCompatible::BufferSize::Default ,OSCompatible::Time::Milliseconds_500);

            EXPECT_EQ("", *dataFromClient ); // here still not received 500 ms

            dataFromClient = clientSocket.receiveDataWithTimeout(OSCompatible::BufferSize::Default ,OSCompatible::Time::Milliseconds_500);

            EXPECT_EQ(messageToServer, *dataFromClient ); // here received 750 ms

            std::this_thread::sleep_for(std::chrono::milliseconds(600)); 

            clientSocket.sendData(messageToClient); // send on 600 ms
        }

        th.join();
    }
    else
    {
        EXPECT_EQ(false, true)<< "failed to connect";
        std::cerr << "Failed to start server." << std::endl;
    }
}
