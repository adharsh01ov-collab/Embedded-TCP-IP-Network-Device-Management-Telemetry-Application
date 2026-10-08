#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 5000
#define BUFFER_SIZE 1024

bool sendAll(SOCKET socket, const std::string& message)
{
    int totalSent = 0;
    int messageLength = static_cast<int>(message.length());

    while (totalSent < messageLength)
    {
        int bytesSent = send(
            socket,
            message.c_str() + totalSent,
            messageLength - totalSent,
            0
        );

        if (bytesSent == SOCKET_ERROR)
        {
            return false;
        }

        totalSent += bytesSent;
    }

    return true;
}

void processMessage(const std::string& message)
{
    std::stringstream stream(message);
    std::string type;

    std::getline(stream, type, '|');

    if (type == "REGISTER")
    {
        std::string deviceId;
        std::string deviceType;

        std::getline(stream, deviceId, '|');
        std::getline(stream, deviceType, '|');

        std::cout << "\n[REGISTER]" << std::endl;
        std::cout << "Device ID   : " << deviceId << std::endl;
        std::cout << "Device Type : " << deviceType << std::endl;
        std::cout << "Status      : ONLINE" << std::endl;
    }
    else if (type == "TELEMETRY")
    {
        std::string deviceId;
        std::string temperature;
        std::string speed;
        std::string voltage;

        std::getline(stream, deviceId, '|');
        std::getline(stream, temperature, '|');
        std::getline(stream, speed, '|');
        std::getline(stream, voltage, '|');

        std::cout << "\n[TELEMETRY]" << std::endl;
        std::cout << "Device ID   : " << deviceId << std::endl;
        std::cout << "Temperature : " << temperature << " C" << std::endl;
        std::cout << "Speed       : " << speed << " RPM" << std::endl;
        std::cout << "Voltage     : " << voltage << " V" << std::endl;
    }
    else if (type == "HEARTBEAT")
    {
        std::string deviceId;

        std::getline(stream, deviceId, '|');

        std::cout << "\n[HEARTBEAT]" << std::endl;
        std::cout << "Device ID : " << deviceId << std::endl;
        std::cout << "Status    : ALIVE" << std::endl;
    }
    else
    {
        std::cout << "\n[UNKNOWN MESSAGE]" << std::endl;
        std::cout << message << std::endl;
    }
}

int main()
{
    std::cout << "======================================" << std::endl;
    std::cout << " Embedded TCP/IP Network Device Manager" << std::endl;
    std::cout << "======================================" << std::endl;

    WSADATA wsaData;

    int result = WSAStartup(
        MAKEWORD(2, 2),
        &wsaData
    );

    if (result != 0)
    {
        std::cout << "WSAStartup failed." << std::endl;
        return 1;
    }

    std::cout << "[OK] Winsock initialized." << std::endl;

    SOCKET serverSocket = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (serverSocket == INVALID_SOCKET)
    {
        std::cout << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    std::cout << "[OK] TCP socket created." << std::endl;

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(SERVER_PORT);

    result = bind(
        serverSocket,
        reinterpret_cast<sockaddr*>(&serverAddress),
        sizeof(serverAddress)
    );

    if (result == SOCKET_ERROR)
    {
        std::cout << "Bind failed. Error: "
                  << WSAGetLastError() << std::endl;

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "[OK] Server bound to 127.0.0.1:"
              << SERVER_PORT << std::endl;

    result = listen(serverSocket, 5);

    if (result == SOCKET_ERROR)
    {
        std::cout << "Listen failed. Error: "
                  << WSAGetLastError() << std::endl;

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "[OK] Server listening..." << std::endl;

    SOCKET clientSocket = accept(
        serverSocket,
        nullptr,
        nullptr
    );

    if (clientSocket == INVALID_SOCKET)
    {
        std::cout << "Accept failed. Error: "
                  << WSAGetLastError() << std::endl;

        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "[CONNECTED] Device connected." << std::endl;

    std::string receiveBuffer;
    char buffer[BUFFER_SIZE];

    while (true)
    {
        int bytesReceived = recv(
            clientSocket,
            buffer,
            BUFFER_SIZE - 1,
            0
        );

        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';

            receiveBuffer += buffer;

            size_t newlinePosition;

            while (
                (newlinePosition = receiveBuffer.find('\n'))
                != std::string::npos
            )
            {
                std::string message =
                    receiveBuffer.substr(0, newlinePosition);

                receiveBuffer.erase(
                    0,
                    newlinePosition + 1
                );

                if (!message.empty() &&
                    message.back() == '\r')
                {
                    message.pop_back();
                }

                processMessage(message);

                sendAll(
                    clientSocket,
                    "ACK\n"
                );
            }
        }
        else if (bytesReceived == 0)
        {
            std::cout << "\n[DISCONNECTED] Device disconnected."
                      << std::endl;

            break;
        }
        else
        {
            std::cout << "\n[ERROR] recv() failed. Error: "
                      << WSAGetLastError()
                      << std::endl;

            break;
        }
    }

    closesocket(clientSocket);
    closesocket(serverSocket);

    WSACleanup();

    std::cout << "\nServer shutdown complete." << std::endl;

    return 0;
}
