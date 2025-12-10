#include "stdafx.h"

#define PIPE_NAME L"\\\\.\\pipe\\Tube"

int main()
{
    setlocale(LC_ALL, "Rus");
    cout << "ClientNPt" << endl;

    HANDLE hPipe; 
    DWORD ps;
    DWORD state = PIPE_READMODE_MESSAGE;

    try
    {
        if ((hPipe = CreateFile(
            PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE, 
            FILE_SHARE_READ | FILE_SHARE_WRITE, 
            NULL, 
            OPEN_EXISTING, 
            NULL, 
            NULL)) == INVALID_HANDLE_VALUE)
            throw SetPipeError("createfile:", GetLastError());

        if (!SetNamedPipeHandleState(hPipe, &state, NULL, NULL)) 
        {
            throw SetPipeError("SetNamedPipeHandleState: ", GetLastError());
        }

        int message_amount = 30;
        char obuf[50];
        char ibuf[50];

        for (int i = 1; i <= message_amount; i++)
        {
            string obufstr = "Hello from ClientNPt " + to_string(i);
            strcpy_s(obuf, obufstr.c_str());

            if(TransactNamedPipe(hPipe, 
                obuf, 
                sizeof(obuf), 
                ibuf, 
                sizeof(ibuf), 
                &ps, 
                NULL)) 
                cout << ibuf << endl;
            else
                throw SetPipeError("TransactNamedPipe: ", GetLastError());
        }

        CloseHandle(hPipe);
    }
    catch (string ErrorPipeText)
    {
        cout << endl << ErrorPipeText;
    }

}
