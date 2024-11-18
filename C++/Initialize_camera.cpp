#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include "uc480.h"  // Include the Thorlabs SDK

#pragma comment(lib, "ws2_32.lib")  // Link with Winsock library

typedef long unsigned int HCAM;

HCAM initializeCamera();
void captureAndSendMaxIntensity(HCAM hCam);
void sendPositionToServer(const std::string &position);

void sendPositionToServer(const std::string &position) {
    SOCKET clientSocket;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(65432);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to server: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    int bytesSent = send(clientSocket, position.c_str(), position.length(), 0);
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Failed to send data: " << WSAGetLastError() << std::endl;
    } else {
        std::cout << "Sent max intensity position to server: " << position << std::endl;
    }

    closesocket(clientSocket);
    WSACleanup();
}

HCAM initializeCamera() {
    HCAM hCam = 0;
    int status = is_InitCamera(&hCam, NULL);
    if (status != IS_SUCCESS) {
        std::cerr << "Failed to initialize the camera, status code: " << status << std::endl;
        return 0;
    }

    // Set the color mode, pixel clock, and frame rate
    is_SetColorMode(hCam, IS_CM_MONO8);
    
    UINT pixelClock = 12;  // Set pixel clock to a lower value for testing
    status = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, &pixelClock, sizeof(pixelClock));
    if (status != IS_SUCCESS) {
        std::cerr << "Failed to set pixel clock, status code: " << status << std::endl;
    }

    double actualFrameRate = 1.0;  // Lower frame rate for testing
    status = is_SetFrameRate(hCam, actualFrameRate, &actualFrameRate);
    if (status != IS_SUCCESS) {
        std::cerr << "Failed to set frame rate, status code: " << status << std::endl;
    }

    double exposureTime = 0.1;  // Set exposure time for testing
    UINT nCommand = IS_EXPOSURE_CMD_SET_EXPOSURE;
    status = is_Exposure(hCam, nCommand, &exposureTime, sizeof(exposureTime));
    if (status != IS_SUCCESS) {
        std::cerr << "Failed to set exposure time, status code: " << status << std::endl;
    }

    // Adding delay to allow the camera to be ready
    Sleep(500);  // Sleep for 500 ms to ensure camera is fully initialized

    return hCam;
}

void captureAndSendMaxIntensity(HCAM hCam) {
    char* imageMemory;
    int memID;
    int status = is_AllocImageMem(hCam, 1280, 1024, 8, &imageMemory, &memID);
    if (status != IS_SUCCESS) {
        std::cerr << "Failed to allocate image memory, status code: " << status << std::endl;
        return;
    }

    status = is_SetImageMem(hCam, imageMemory, memID);
    if (status != IS_SUCCESS) {
        std::cerr << "Failed to set image memory, status code: " << status << std::endl;
        is_FreeImageMem(hCam, imageMemory, memID);
        return;
    }

    // Attempt to capture frames using freeze capture (instead of continuous capture for now)
    while (true) {
        status = is_FreezeVideo(hCam, IS_WAIT);  // Use freeze capture for testing
        if (status != IS_SUCCESS) {
            std::cerr << "Failed to capture frame, status code: " << status << std::endl;
            break;
        }

        std::cout << "Captured a frame, processing image..." << std::endl;

        // Ensure the image memory has been filled correctly
        // Convert the image to an array for processing
        int width = 1280, height = 1024;
        unsigned char* imgData = reinterpret_cast<unsigned char*>(imageMemory);  // Image data is stored here

        int maxIntensity = 0;
        int maxX = 0, maxY = 0;
        // Loop over the image data to find the max intensity
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int pixelIndex = y * width + x;  // 1D index of the pixel in the image buffer
                int pixelValue = imgData[pixelIndex];  // Assuming image is 8-bit grayscale (IS_CM_MONO8)

                // If this pixel has the highest intensity, store its position
                if (pixelValue > maxIntensity) {
                    maxIntensity = pixelValue;
                    maxX = x;
                    maxY = y;
                }
            }
        }

        // Now send the max position to the server
        std::string maxPosition = std::to_string(maxX) + "," + std::to_string(maxY);
        sendPositionToServer(maxPosition);
    }

    is_FreeImageMem(hCam, imageMemory, memID);
    is_ExitCamera(hCam);
}

void runCamera() {
    HCAM hCam = initializeCamera();
    if (hCam != 0) {
        captureAndSendMaxIntensity(hCam);
    }
}
