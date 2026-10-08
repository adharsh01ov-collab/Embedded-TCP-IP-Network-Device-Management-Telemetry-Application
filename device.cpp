#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 5000

const char* SERVER_IP = "127.0.0.1";

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

int main()
{
    std::cout << "======================================" << std::endl;
    std::cout << " Embedded Device Simulator" << std::endl;
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

    SOCKET clientSocket = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (clientSocket == INVALID_SOCKET)
    {
        std::cout << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    std::cout << "[OK] TCP socket created." << std::endl;

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;

    serverAddress.sin_addr.s_addr =
        inet_addr(SERVER_IP);

    serverAddress.sin_port =
        htons(SERVER_PORT);

    result = connect(
        clientSocket,
        reinterpret_cast<sockaddr*>(&serverAddress),
        sizeof(serverAddress)
    );

    if (result == SOCKET_ERROR)
    {
        std::cout << "[ERROR] Connection failed. Error: "
                  << WSAGetLastError()
                  << std::endl;

        closesocket(clientSocket);
        WSACleanup();

        return 1;
    }

    std::cout << "[CONNECTED] Connected to server."
              << std::endl;

    std::string registration =
        "REGISTER|DEV001|ECU\n";

    if (!sendAll(clientSocket, registration))
    {
        std::cout << "[ERROR] Registration failed."
                  << std::endl;

        closesocket(clientSocket);
        WSACleanup();

        return 1;
    }

    std::cout << "[SENT] Device registration."
              << std::endl;

    double temperature = 28.5;
    double speed = 1200.0;
    double voltage = 3.30;

    int heartbeatCounter = 0;

    while (true)
    {
        std::this_thread::sleep_for(
            std::chrono::seconds(3)
        );

        temperature += 0.2;
        speed += 50.0;

        std::string telemetry =
            "TELEMETRY|DEV001|" +
            std::to_string(temperature) + "|" +
            std::to_string(speed) + "|" +
            std::to_string(voltage) + "\n";

        if (!sendAll(clientSocket, telemetry))
        {
            std::cout << "[ERROR] Telemetry transmission failed."
                      << std::endl;
            break;
        }

        std::cout << "[SENT] Telemetry -> "
                  << "Temp=" << temperature
                  << " C, Speed=" << speed
                  << " RPM"
                  << std::endl;

        heartbeatCounter++;

        if (heartbeatCounter >= 2)
        {
            std::string heartbeat =
                "HEARTBEAT|DEV001\n";

            if (!sendAll(clientSocket, heartbeat))
            {
                std::cout << "[ERROR] Heartbeat failed."
                          << std::endl;
                break;
            }

            std::cout << "[SENT] Heartbeat."
                      << std::endl;

            heartbeatCounter = 0;
        }
    }

    closesocket(clientSocket);

    WSACleanup();

    std::cout << "Device shutdown complete."
              << std::endl;

    return 0;
}
