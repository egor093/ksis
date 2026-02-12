#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
    setlocale(LC_ALL, "rus");
    // Инициализация Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    while (true) {
        // Создание сокета
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Socket creation failed." << endl;
            WSACleanup();
            return 1;
        }

        // Настройка адреса сервера
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(12345);
        InetPton(AF_INET, TEXT("127.0.0.1"), &serverAddr.sin_addr.s_addr);

        // Подключение к серверу
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "Connection failed." << endl;
            closesocket(clientSocket);
            continue;
        }

        cout << "Подключение к серверу установлено." << endl;

        // Получение приветственного сообщения от сервера
        char buffer[4096];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            cout << buffer << endl;
        }

        // Ввод максимальной стоимости
        cout << "> ";
        string input;
        getline(cin, input);

        if (input == "exit") {
            closesocket(clientSocket);
            break;
        }

        // Отправка запроса серверу
        send(clientSocket, input.c_str(), input.length() + 1, 0);

        // Получение ответа от сервера
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            cout << "\n" << buffer << endl;
        }
        else {
            cout << "Ошибка при получении ответа от сервера." << endl;
        }

        cout << "\n" << string(50, '=') << endl;
        closesocket(clientSocket);
    }

    WSACleanup();
    return 0;
}