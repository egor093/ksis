#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    // Создание UDP-сокета
    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed." << endl;
        WSACleanup();
        return 1;
    }


    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    InetPton(AF_INET, TEXT("127.0.0.1"), &serverAddr.sin_addr.s_addr);

    cout << "Enter a string: ";
    string input;
    getline(cin, input);

    // Отправка строки серверу
    int sendResult = sendto(clientSocket, input.c_str(), input.length() + 1, 0,
                          (sockaddr*)&serverAddr, sizeof(serverAddr));

    if (sendResult == SOCKET_ERROR) {
        cerr << "sendto failed: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Sent to server: " << input << endl;


    char buffer[1024];
    sockaddr_in fromAddr;
    int fromAddrSize = sizeof(fromAddr);

    int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0,
                               (sockaddr*)&fromAddr, &fromAddrSize);

    if (bytesReceived == SOCKET_ERROR) {
        cerr << "recvfrom failed: " << WSAGetLastError() << endl;
    } else {
        buffer[bytesReceived] = '\0';
        cout << "Processed string: " << buffer << endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
