#include <iostream>
#include "Winsock2.h"
#include "stdafx.h"
#pragma comment(lib, "WS2_32.lib")
#pragma warning(disable : 4996)

using namespace std;

bool GetRequestFromClient(char* name, short port, struct sockaddr* from, int* flen);
bool PutAnswerToClient(char* name, short port, struct sockaddr* to, int* lto);

int main()
{
    setlocale(LC_ALL, "Russian");

    try {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            throw "Startup failed";

        char callName[] = "Hello";
        short port = 2000;

        char serverHost[256];
        gethostname(serverHost, sizeof(serverHost));
        cout << "Имя сервера: " << serverHost << endl;

        SOCKADDR_IN clientAddr;
        int clientLen = sizeof(clientAddr);

        cout << "Сервер запущен, ждём клиентов..." << endl;

        while (true)
        {
            if (GetRequestFromClient(callName, port, (sockaddr*)&clientAddr, &clientLen))
            {
                cout << "\nЗапрос от клиента:" << endl;
                cout << "IP: " << inet_ntoa(clientAddr.sin_addr) << endl;
                cout << "Port: " << ntohs(clientAddr.sin_port) << endl;

                hostent* h = gethostbyaddr(
                    (char*)&clientAddr.sin_addr,
                    4, AF_INET);

                if (h)
                    cout << "Имя клиента: " << h->h_name << endl;
                else
                    cout << "Имя клиента определить не удалось." << endl;

                if (PutAnswerToClient(callName, port, (sockaddr*)&clientAddr, &clientLen))
                {
                    cout << "Ответ клиенту отправлен!" << endl;
                }
            }
        }

        WSACleanup();
    }
    catch (const char* err) {
        cout << "Ошибка: " << err << endl;
    }

    return 0;
}

bool GetRequestFromClient(char* name, short port, struct sockaddr* from, int* flen)
{
    SOCKET sS = socket(AF_INET, SOCK_DGRAM, 0);
    if (sS == INVALID_SOCKET) throw "socket failed";

    SOCKADDR_IN serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr = INADDR_ANY;

    if (bind(sS, (sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR)
        throw "bind failed";

    char buf[50] = "";
    int recvLen = recvfrom(sS, buf, sizeof(buf), 0, from, flen);
    if (recvLen == SOCKET_ERROR) {
        closesocket(sS);
        return false;
    }

    bool ok = strcmp(buf, name) == 0;
    closesocket(sS);
    return ok;
}

bool PutAnswerToClient(char* name, short port, struct sockaddr* to, int* lto)
{
    SOCKET sS = socket(AF_INET, SOCK_DGRAM, 0);
    if (sS == INVALID_SOCKET) throw "socket failed";

    int sentLen = sendto(sS, name, strlen(name) + 1, 0, to, *lto);
    closesocket(sS);
    return sentLen != SOCKET_ERROR;
}
