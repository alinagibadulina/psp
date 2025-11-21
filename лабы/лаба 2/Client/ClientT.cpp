#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <string>
#include <ctime>
#include "ErrorHandler.h"
#pragma comment(lib, "WS2_32.lib")

using namespace std;

int main() {
    setlocale(0, "Russian");

    WSADATA wsaData;
    SOCKET cC;

    try {
        // Инициализация библиотеки Winsock
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
            throw SetErrorMsgText("Startup:", WSAGetLastError());

        // Создание клиентского сокета
        if ((cC = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
            throw SetErrorMsgText("socket:", WSAGetLastError());

        // Подключение к серверу
        SOCKADDR_IN serv;
        serv.sin_family = AF_INET;
        serv.sin_port = htons(2000);
        serv.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(cC, (sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR)
            throw SetErrorMsgText("connect:", WSAGetLastError());

        cout << "Подключение к серверу установлено." << endl;

        // Ввод количества сообщений
        int messageCount;
        cout << "Введите количество сообщений для отправки: ";
        cin >> messageCount;

        // Измерение времени начала обмена
        clock_t startTime = clock();

        // Основной цикл обмена сообщениями
        for (int i = 1; i <= messageCount; i++) {
            // Формирование сообщения
            string message = "Hello from Client " + to_string(i);

            // Отправка сообщения серверу
            if (send(cC, message.c_str(), message.length(), NULL) == SOCKET_ERROR)
                throw SetErrorMsgText("send:", WSAGetLastError());

            cout << "Отправлено [" << i << "/" << messageCount << "]: " << message << endl;

            // Прием эхо-ответа от сервера
            char buffer[1024];
            int bytesRecv = recv(cC, buffer, sizeof(buffer) - 1, NULL);
            if (bytesRecv == SOCKET_ERROR)
                throw SetErrorMsgText("recv:", WSAGetLastError());

            buffer[bytesRecv] = '\0';
            cout << "Получено эхо: " << buffer << endl;

            // Вывод прогресса каждые 100 сообщений
            if (i % 100 == 0) {
                cout << "Обработано " << i << " сообщений из " << messageCount << endl;
            }
        }

        // Отправка сообщения нулевой длины для завершения работы сервера
        if (send(cC, "", 0, NULL) == SOCKET_ERROR)
            throw SetErrorMsgText("send:", WSAGetLastError());

        // Измерение времени окончания и расчет
        clock_t endTime = clock();
        double elapsedTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;

        cout << "\n=== РЕЗУЛЬТАТЫ ТЕСТИРОВАНИЯ ===" << endl;
        cout << "Общее количество сообщений: " << messageCount << endl;
        cout << "Общее время обмена: " << elapsedTime << " секунд" << endl;
        cout << "Среднее время на сообщение: " << (elapsedTime * 1000 / messageCount) << " мс" << endl;
        cout << "Сообщений в секунду: " << (messageCount / elapsedTime) << endl;

        // Завершение работы клиента
        closesocket(cC);

        if (WSACleanup() == SOCKET_ERROR)
            throw SetErrorMsgText("Cleanup:", WSAGetLastError());
    }
    catch (string errorMsgText) {
        cout << "Ошибка: " << errorMsgText << endl;
        if (cC != INVALID_SOCKET) closesocket(cC);
        WSACleanup();
    }

    return 0;
}