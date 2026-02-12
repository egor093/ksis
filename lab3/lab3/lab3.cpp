#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Структура для хранения информации о турах
struct Tour {
    string name;
    double price;
    int duration;
    string transport;
};

// Глобальный каталог туров
vector<Tour> tours;

// Функция для инициализации каталога туров
void initializeTours() {
    tours = {
        {"Отдых в Турции", 45000.0, 7, "Самолет"},
        {"Экскурсия по Европе", 89000.0, 10, "Автобус"},
        {"Горнолыжный курорт", 65000.0, 5, "Самолет"},
        {"Круиз по Средиземному морю", 120000.0, 14, "Корабль"},
        {"Пляжный отдых в Египте", 38000.0, 7, "Самолет"},
        {"Тур по Золотому кольцу", 25000.0, 5, "Автобус"},
        {"Экзотический Таиланд", 75000.0, 10, "Самолет"},
        {"Оздоровительный тур", 55000.0, 7, "Поезд"},
        {"Гастрономический тур по Италии", 95000.0, 8, "Самолет"},
        {"Сафари в Африке", 150000.0, 12, "Самолет"}
    };
}

// Функция для поиска туров по максимальной стоимости
string findToursByPrice(double maxPrice) {
    stringstream result;
    vector<Tour> foundTours;

    // Поиск туров с стоимостью не выше указанной
    for (const auto& tour : tours) {
        if (tour.price <= maxPrice) {
            foundTours.push_back(tour);
        }
    }

    if (foundTours.empty()) {
        result << "Туры стоимостью до " << maxPrice << " рублей не найдены.";
    }
    else {
        // Сортировка по стоимости (от дешевых к дорогим)
        sort(foundTours.begin(), foundTours.end(),
            [](const Tour& a, const Tour& b) { return a.price < b.price; });

        result << "Найдено " << foundTours.size() << " туров стоимостью до " << maxPrice << " рублей:\n\n";

        for (const auto& tour : foundTours) {
            result << "• " << tour.name
                << " - " << tour.price << " руб."
                << " (" << tour.duration << " дней, " << tour.transport << ")\n";
        }
    }

    return result.str();
}

// Функция для получения информации о диапазоне цен
string getPriceRange() {
    double minPrice = tours[0].price;
    double maxPrice = tours[0].price;

    for (const auto& tour : tours) {
        if (tour.price < minPrice) minPrice = tour.price;
        if (tour.price > maxPrice) maxPrice = tour.price;
    }

    stringstream result;
    result << "Диапазон цен на туры: от " << minPrice << " до " << maxPrice << " рублей\n";
    return result.str();
}
// Функция потока для обработки клиента
DWORD WINAPI ClientThread(LPVOID clientSocket) {
    SOCKET s = *(SOCKET*)clientSocket;
    char buffer[1024];

    // Отправка приветственного сообщения с диапазоном цен
    string welcomeMsg = "Добро пожаловать в турагентство!\n" + getPriceRange() +
        "\nВведите максимальную стоимость тура (в рублях): ";
    send(s, welcomeMsg.c_str(), welcomeMsg.length() + 1, 0);

    // Получение запроса от клиента
    int bytesReceived = recv(s, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        string request(buffer);

        cout << "Получен запрос от клиента: " << request << " рублей" << endl;

        try {
            // Преобразование строки в число
            double maxPrice = stod(request);

            if (maxPrice <= 0) {
                string response = "Ошибка: стоимость должна быть положительным числом.";
                send(s, response.c_str(), response.length() + 1, 0);
            }
            else {
                // Обработка запроса
                string response = findToursByPrice(maxPrice);
                send(s, response.c_str(), response.length() + 1, 0);
            }
        }
        catch (...) {
            string response = "Ошибка: введите корректное число (например: 50000)";
            send(s, response.c_str(), response.length() + 1, 0);
        }

        cout << "Отправлен ответ клиенту" << endl;
    }

    closesocket(s);
    delete (SOCKET*)clientSocket;
    return 0;
}

int main() {
    setlocale(LC_ALL, "rus");
    // Инициализация Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    // Инициализация каталога туров
    initializeTours();

    cout << "Каталог туров инициализирован. Всего записей: " << tours.size() << endl;

    // Создание сокета
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed." << endl;
        WSACleanup();
        return 1;
    }

    // Привязка сокета к адресу
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

    // Прослушивание порта
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed." << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Многопоточный TCP-сервер запущен на порту 12345..." << endl;
    cout << "Ожидание подключений клиентов..." << endl;

    int clientCount = 0;

    while (true) {
        // Принятие подключения
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET* clientSocket = new SOCKET;

        *clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if (*clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed." << endl;
            delete clientSocket;
            continue;
        }

        clientCount++;
        cout << "Клиент №" << clientCount << " подключен." << endl;

        // Создание потока для обработки клиента
        DWORD threadID;
        HANDLE hThread = CreateThread(NULL, 0, ClientThread, clientSocket, 0, &threadID);

        if (hThread == NULL) {
            cerr << "Failed to create thread." << endl;
            closesocket(*clientSocket);
            delete clientSocket;
        }
        else {
            CloseHandle(hThread);
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}