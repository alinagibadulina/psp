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
using std::cin;


void ThrowIfWSAError(int code, const char* msg) {
    if (code != 0) {
        int err = WSAGetLastError();
        char buf[256];
        snprintf(buf, sizeof(buf), "%s (WSA error %d)", msg, err);
        throw std::runtime_error(buf);
    }
}

void measureSpeed(SOCKET sock, sockaddr_in& servAddr, const string& message, int iterations) {
    cout << "\n=== ИЗМЕРЕНИЕ СКОРОСТИ ПЕРЕДАЧИ ===\n";
    cout << "Количество итераций: " << iterations << "\n";
    cout << "Размер сообщения: " << message.size() << " байт\n";

    const int BUF_SIZE = 1024;
    char buffer[BUF_SIZE];
    sockaddr_in fromAddr{};
    int fromLen = sizeof(fromAddr);

    int timeoutMs = 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));

    int successCount = 0;
    int totalBytesSent = 0;
    int totalBytesReceived = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++) {
        string currentMessage = message + " [" + std::to_string(i + 1) + "]";
        int sent = sendto(sock, currentMessage.c_str(), (int)currentMessage.size(), 0,
            (sockaddr*)&servAddr, sizeof(servAddr));

        if (sent != SOCKET_ERROR) {
            totalBytesSent += sent;

            int bytesReceived = recvfrom(sock, buffer, BUF_SIZE - 1, 0,
                (sockaddr*)&fromAddr, &fromLen);

            if (bytesReceived != SOCKET_ERROR) {
                totalBytesReceived += bytesReceived;
                successCount++;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    cout << "Успешных обменов: " << successCount << "/" << iterations << "\n";
    cout << "Всего отправлено байт: " << totalBytesSent << "\n";
    cout << "Всего получено байт: " << totalBytesReceived << "\n";
    cout << "Общее время: " << duration.count() << " мс\n";

    if (duration.count() > 0) {
        double speedSent = (double)totalBytesSent / duration.count() * 1000.0 / 1024.0; 
        double speedReceived = (double)totalBytesReceived / duration.count() * 1000.0 / 1024.0;
        cout << "Скорость отправки: " << speedSent << " КБ/с\n";
        cout << "Скорость приема: " << speedReceived << " КБ/с\n";
    }
    cout << "====================================\n\n";
}

int main() {
    setlocale(0, "Russian");

    try {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData) ;
        cout << "Winsock инициализирован.\n";

        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) {
            int err = WSAGetLastError();
            WSACleanup();
            throw std::runtime_error("Не удалось создать сокет. WSA error: " + std::to_string(err));
        }
        cout << "UDP сокет создан.\n";

        sockaddr_in servAddr{};
        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(2000);

        char serverIP[16];
        cout << "Введите IP сервера (по умолчанию 127.0.0.1): ";
        cin.getline(serverIP, 16);
        if (strlen(serverIP) == 0) {
            strcpy_s(serverIP, "127.0.0.1");
        }
        servAddr.sin_addr.s_addr = inet_addr(serverIP);

        cout << "Подключение к серверу: " << serverIP << ":2000\n";

        string testMessage = "Test message for speed measurement ";
        measureSpeed(sock, servAddr, testMessage, 100);

        while (true) {
            string message;
            cout << "Введите сообщение (или 'exit' для выхода): ";
            std::getline(std::cin, message);

            if (message == "exit") {
                break;
            }

            if (message.empty()) {
                message = "Hello from ClientU";
            }

            int sent = sendto(sock, message.c_str(), (int)message.size(), 0,
                (sockaddr*)&servAddr, sizeof(servAddr));

            if (sent == SOCKET_ERROR) {
                int err = WSAGetLastError();
                cerr << "sendto вернул SOCKET_ERROR. WSAGetLastError() = " << err << "\n";
            }
            else {
                cout << "Отправлено " << sent << " байт серверу " << serverIP << ":2000\n";
            }

            int timeoutMs = 5000;
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));

            const int BUF_SIZE = 1024;
            char buffer[BUF_SIZE];
            sockaddr_in fromAddr{};
            int fromLen = sizeof(fromAddr);

            int bytesReceived = recvfrom(sock, buffer, BUF_SIZE - 1, 0,
                (sockaddr*)&fromAddr, &fromLen);

            if (bytesReceived == SOCKET_ERROR) {
                int err = WSAGetLastError();
                if (err == WSAETIMEDOUT) {
                    cout << "recvfrom таймаут (сервер не ответил в течение " << timeoutMs << " мс).\n";
                }
                else {
                    cerr << "recvfrom вернул SOCKET_ERROR. WSAGetLastError() = " << err << "\n";
                }
            }
            else {
                buffer[bytesReceived] = '\0';
                char fromIp[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &fromAddr.sin_addr, fromIp, sizeof(fromIp));
                cout << "Получен ответ от " << fromIp << ":" << ntohs(fromAddr.sin_port) << "\n";
                cout << "Текст: \"" << buffer << "\"\n";
            }
        }

        closesocket(sock);
        WSACleanup();
        cout << "ClientU завершил работу.\n";
        return 0;
    }
    catch (const std::exception& ex) {
        cerr << "[ОШИБКА] " << ex.what() << endl;
        return 1;
    }
}