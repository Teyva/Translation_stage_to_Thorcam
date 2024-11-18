#include <iostream>
#include <thread>
#include "send_data.h"         // Inclure l'en-tête de send_data
#include "initialize_camera.h" // Inclure l'en-tête de initialize_camera

// Function declarations
void runSendData();  // From send_data.cpp
void runCamera();    // From initialize_camera.cpp

int main() {
    std::cout << "Starting the application..." << std::endl;

    // Start serial communication (send_data) in a separate thread
    std::cout << "Starting serial communication (send_data)..." << std::endl;
    std::thread sendDataThread(runSendData);

    // Start camera capture and max intensity data transmission in a separate thread
    std::cout << "Starting camera operation..." << std::endl;
    std::thread cameraThread(runCamera);

    // Wait for both threads to complete
    sendDataThread.join();
    cameraThread.join();

    std::cout << "Application has completed." << std::endl;
    return 0;
}
