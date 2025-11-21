#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <chrono>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using std::cout;
using std::cerr;
using std::endl;
using std::string;

void ThrowIfWSAError(int code, const char* msg) {
    if (code != 0) {
        int err = WSAGetLastError();
        char buf[256];
        snprintf(buf, sizeof(buf), "%s (WSA error %d)", msg, err);
        throw std::runtime_error(buf);
    }
}

int main() {
    try {
        setlocale(0, "Russian");

        WSADATA wsaData;
        int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
        cout << "Winsock инициализирован.\n";

        SOCKET servSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        cout << "UDP сокет создан.\n";

        sockaddr_in servAddr{};
        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(2000);
        servAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(servSock, (sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
            int err = WSAGetLastError();
            closesocket(servSock);
            WSACleanup();
            throw std::runtime_error("Bind failed. WSA error: " + std::to_string(err));
        }
        cout << "Сокет привязан к порту 2000 и готов принимать сообщения.\n";

        cout << "Сервер запущен в многопользовательском режиме. Ожидаем подключений...\n";

        int clientCount = 0;
        const int BUF_SIZE = 1024;
        char buffer[BUF_SIZE];

        while (true) {
            sockaddr_in clientAddr{};
            int clientAddrLen = sizeof(clientAddr);

            cout << "Ждем входящее сообщение (recvfrom)...\n";

            int bytesReceived = recvfrom(servSock, buffer, BUF_SIZE - 1, 0,
                (sockaddr*)&clientAddr, &clientAddrLen);

            buffer[bytesReceived] = '\0';
            char clientIp[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, sizeof(clientIp));

            clientCount++;
            cout << "Клиент #" << clientCount << " - " << clientIp << ":" << ntohs(clientAddr.sin_port) << "\n";
            cout << "Получено " << bytesReceived << " байт: \"" << buffer << "\"\n";

            //Sleep(3000);

            string reply = string("Echo from ServerU [Клиент #") + std::to_string(clientCount) + "]: " + buffer;
            int sent = sendto(servSock, reply.c_str(), (int)reply.size(), 0,
                (sockaddr*)&clientAddr, clientAddrLen);

            if (sent == SOCKET_ERROR) {
                int err = WSAGetLastError();
                cerr << "sendto failed. WSA error: " << err << "\n";
            }
            else {
                cout << "Отправлено " << sent << " байт в адрес клиента.\n";
            }

            cout << "----------------------------------------\n";

            
            if (strcmp(buffer, "exit") == 0) {
                cout << "Получена команда exit. Завершение работы сервера.\n";
                break;
            }
        }

        closesocket(servSock);
        WSACleanup();
        cout << "ServerU завершил работу. Обслужено клиентов: " << clientCount << endl;
        return 0;
    }
    catch (const std::exception& ex) {
        cerr << "[ОШИБКА] " << ex.what() << endl;
        return 1;
    }
}
