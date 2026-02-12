#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

string swapEvenOddPositions(const string& input) {
    string result = input;

    for (size_t i = 0; i + 1 < result.length(); i += 2) {
        swap(result[i], result[i + 1]);
    }

    return result;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12345);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed." << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "UDP Server is listening on port 12345..." << endl;

    while (true) {
        char buffer[1024];
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);

        int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0,
            (sockaddr*)&clientAddr, &clientAddrSize);

        if (bytesReceived == SOCKET_ERROR) {
            cerr << "recvfrom failed: " << WSAGetLastError() << endl;
            continue;
        }

        buffer[bytesReceived] = '\0';
        cout << "Received from client: " << buffer << endl;

        string originalStr(buffer);
        string processedStr = swapEvenOddPositions(originalStr);

        int sendResult = sendto(serverSocket, processedStr.c_str(), processedStr.length() + 1, 0,
            (sockaddr*)&clientAddr, clientAddrSize);

        if (sendResult == SOCKET_ERROR) {
            cerr << "sendto failed: " << WSAGetLastError() << endl;
        }
        else {
            cout << "Sent to client: " << processedStr << endl;
            cout << "Original: " << originalStr << endl;
            cout << "Processed: " << processedStr << endl;
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}