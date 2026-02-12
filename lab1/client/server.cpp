#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

using namespace std;

string determineQuadrant(double x, double y) {
    if (x > 0 && y > 0) return "I четверть";
    if (x < 0 && y > 0) return "II четверть";
    if (x < 0 && y < 0) return "III четверть";
    if (x > 0 && y < 0) return "IV четверть";
    if (x == 0 && y == 0) return "Начало координат";
    if (x == 0) return "На оси Y";
    if (y == 0) return "На оси X";
    return "Неопределено";
}


int main() {
    setlocale(LC_ALL, "Russian");
    // Инициализация Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    // Создание сокета
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed." << endl;
        WSACleanup();
        return 1;
    }

    // Привязка сокета к адресу и порту
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

    // Прослушивание входящих подключений
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed." << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server is listening on port 12345..." << endl;

    while (true) {
        // Принятие подключения
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed." << endl;
            continue;
        }

        cout << "Client connected." << endl;

        char buffer[1024];
        int bytesReceived;

        // Получение координат от клиента
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            cout << "Received coordinates: " << buffer << endl;

            // Парсинг координат (формат: "x y")
            double x, y;
            string receivedData(buffer);
            size_t spacePos = receivedData.find(' ');

            if (spacePos != string::npos) {
                try {
                    x = stod(receivedData.substr(0, spacePos));
                    y = stod(receivedData.substr(spacePos + 1));

                    // Определение четверти
                    string quadrant = determineQuadrant(x, y);

                    // Отправка результата клиенту
                    send(clientSocket, quadrant.c_str(), quadrant.size() + 1, 0);
                    cout << "Point (" << x << ", " << y << ") is in: " << quadrant << endl;
                }
                catch (...) {
                    string error = "Invalid coordinates format";
                    send(clientSocket, error.c_str(), error.size() + 1, 0);
                }
            }
            else {
                string error = "Invalid format. Use: x y";
                send(clientSocket, error.c_str(), error.size() + 1, 0);
            }
        }

        // Закрытие соединения с клиентом
        closesocket(clientSocket);
        cout << "Client disconnected." << endl;
    }

    // Закрытие серверного сокета и очистка Winsock
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}