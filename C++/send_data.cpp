#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

#define COM_PORT "COM4"
#define BAUD_RATE CBR_115200
#define TCP_PORT 65432
#define BUFFER_SIZE 1024

void initializeSerial(HANDLE &hSerial) {
    hSerial = CreateFile(COM_PORT, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open serial port" << std::endl;
        exit(1);
    }

    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);
    serialParams.BaudRate = BAUD_RATE;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;
    serialParams.fDtrControl = DTR_CONTROL_DISABLE;
    serialParams.fRtsControl = RTS_CONTROL_ENABLE;

    if (!SetCommState(hSerial, &serialParams)) {
        std::cerr << "Failed to set serial parameters" << std::endl;
        CloseHandle(hSerial);
        exit(1);
    }

    std::cout << "Serial communication initialized on " << COM_PORT << " with baud rate " << BAUD_RATE << std::endl;
}

void initializeWinsock(SOCKET &serverSocket) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        exit(1);
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        exit(1);
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(TCP_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    std::cout << "TCP server is listening on port " << TCP_PORT << std::endl;
}

void sendDataToSerial(HANDLE hSerial, const std::string &data) {
    DWORD bytesWritten;
    if (!WriteFile(hSerial, data.c_str(), data.length(), &bytesWritten, NULL)) {
        std::cerr << "Failed to send data to serial device" << std::endl;
    } else {
        std::cout << "Sent to ESP32: " << data << " (" << bytesWritten << " bytes)" << std::endl;
    }
}

void receiveDataFromSerial(HANDLE hSerial) {
    DWORD bytesRead;
    char buffer[BUFFER_SIZE];

    if (ReadFile(hSerial, buffer, BUFFER_SIZE, &bytesRead, NULL) && bytesRead > 0) {
        std::string receivedData(buffer, bytesRead);
        std::cout << "Received from ESP32: " << receivedData << std::endl;
    }
}

void runSendData() {
    HANDLE hSerial;
    SOCKET serverSocket, clientSocket;
    initializeSerial(hSerial);
    initializeWinsock(serverSocket);

    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        CloseHandle(hSerial);
        WSACleanup();
        exit(1);
    }
    std::cout << "Client connected" << std::endl;

    char recvBuffer[BUFFER_SIZE];
    int bytesReceived;

    while (true) {
        // Receiving data from the client over TCP
        bytesReceived = recv(clientSocket, recvBuffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            recvBuffer[bytesReceived] = '\0';
            std::string data(recvBuffer);
            std::cout << "Received from TCP client: " << data << std::endl;

            // Send data to the ESP32 over serial
            sendDataToSerial(hSerial, data);

            // Optionally, read any response from the ESP32
            receiveDataFromSerial(hSerial);
        } else if (bytesReceived == 0) {
            std::cout << "TCP client disconnected" << std::endl;
            break;
        } else {
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
            break;
        }
    }

    closesocket(clientSocket);
    closesocket(serverSocket);
    CloseHandle(hSerial);
    WSACleanup();
}
