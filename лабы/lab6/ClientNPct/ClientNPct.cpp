#include "stdafx.h"

#define PIPE_NAME L"\\\\.\\pipe\\Tube"

int main()
{
    setlocale(LC_ALL, "Rus");
    cout << "ClientNPct" << endl;

    DWORD ps;

    try
    {
        char obuf[50] = "Hello from ClientNPct!";
        char ibuf[50];


        if (CallNamedPipe(PIPE_NAME,
            obuf, 
            sizeof(obuf), 
            ibuf, 
            sizeof(ibuf), 
            &ps, 
            NULL))  
            cout << ibuf << endl;
        else
            throw SetPipeError("CallNamedPipe: ", GetLastError());

    }
    catch (string ErrorPipeText)
    {
        cout << endl << ErrorPipeText;
    }

}
