#include <iostream>
#include "Winsock2.h"
#include "stdafx.h"
#pragma comment(lib, "WS2_32.lib")
#pragma warning(disable : 4996)

using namespace std;

bool GetServerByName(char* callName, char* serverHostName, short port, sockaddr* from, int* flen);

int main()
{
    setlocale(LC_ALL, "Russian");

    try {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            throw "Startup failed";

        char callName[] = "Hello";          
        char serverName[] = "UserVM";  
        short port = 2000;

        SOCKADDR_IN serv;
        int servLen = sizeof(serv);

        cout << "Клиент ищет сервер по имени: " << serverName << endl;

        if (GetServerByName(callName, serverName, port, (sockaddr*)&serv, &servLen))
        {
            cout << "Сервер найден!" << endl;
            cout << "IP: " << inet_ntoa(serv.sin_addr) << endl;
            cout << "Port: " << ntohs(serv.sin_port) << endl;
            cout << "Позывное слово: " << callName << endl;
        }
        else
        {
            cout << "Сервер не найден." << endl;
        }

        WSACleanup();
    }
    catch (const char* err) {
        cout << "Ошибка: " << err << endl;
    }

    return 0;
}

bool GetServerByName(char* callName, char* serverHostName, short port, sockaddr* from, int* flen)
{
    hostent* h = gethostbyname(serverHostName);
    if (!h) throw "gethostbyname failed";

    IN_ADDR* addr = (IN_ADDR*)h->h_addr_list[0];

    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) throw "socket failed";

    SOCKADDR_IN serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    serv.sin_addr = *addr; 

    sendto(s, callName, strlen(callName) + 1, 0, (sockaddr*)&serv, sizeof(serv));

    char buf[50];
    int recvLen = recvfrom(s, buf, sizeof(buf), 0, from, flen);

    closesocket(s);

    if (recvLen == SOCKET_ERROR)
        return false;

    return strcmp(buf, callName) == 0;
}
