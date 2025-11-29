#include <iostream>
#include "Winsock2.h"
#include "stdafx.h"
#pragma comment(lib, "WS2_32.lib")
#pragma warning(disable : 4996)
using namespace std;

bool GetServer(char* call, short port, sockaddr* from, int* flen);

int main()
{
    setlocale(LC_ALL, "Russian");

    try {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            throw "Startup failed";

        char callName[] = "Hello";
        short port = 2000;

        SOCKADDR_IN serv;
        int servLen = sizeof(serv);

        cout << "Клиент ищет сервер..." << endl;

        if (GetServer(callName, port, (sockaddr*)&serv, &servLen)) {
            cout << "Сервер найден!" << endl;
            cout << "IP: " << inet_ntoa(serv.sin_addr) << endl;
            cout << "Port: " << ntohs(serv.sin_port) << endl;
            cout << "name: " << callName;
        }
        else {
            cout << "Сервер не найден." << endl;
        }

        WSACleanup();
    }
    catch (const char* err) {
        cout << "Ошибка: " << err << endl;
    }
    return 0;
}

bool GetServer(char* call, short port, sockaddr* from, int* flen)
{
    SOCKET sS = socket(AF_INET, SOCK_DGRAM, 0);
    if (sS == INVALID_SOCKET) throw "socket failed";

    int optval = 1;
    if (setsockopt(sS, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
        throw "setsockopt failed";

    SOCKADDR_IN broadcastAddr;
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(port);
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

    sendto(sS, call, strlen(call) + 1, 0, (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));

    
    char buf[50];
    int recvLen = recvfrom(sS, buf, sizeof(buf), 0, from, flen);
    closesocket(sS);
    if (strcmp(buf, call) != 0) {
        throw "Имя сервера не совпадает с отправленным именем ";
    }
    if (recvLen == SOCKET_ERROR) {
        return false;
    }
    
    return strcmp(buf, call) == 0;
}
