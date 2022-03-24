#include "tcp_client.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#define PAUSE                                     \
    printf("Press Enter key to continue...\r\n"); \
    fgetc(stdin);

using namespace ARC;

void Connected(TCPClient *context);
void Disconnected(TCPClient *context, int i_error_code);
void DataReceive(TCPClient *context, pkg i_package);

TCPClient client = TCPClient("127.0.0.1", 2000);
//CPClient client = TCPClient("github.com", 80);

int main(void)
{
    client.Event_Connected = Connected;
    client.Event_DataReceive = DataReceive;
    client.Event_Disconnected = Disconnected;

    printf("Press Enter key to connect to target...\r\n");
    fgetc(stdin);
    bool ret = client.Connect();
    if (ret)
    {
        printf("Press Enter key to disconnect...\r\n");
        fgetc(stdin);
        client.disconnect();
    }
    else
    {
        printf("Connect fail\r\n");
    }

    PAUSE;
    return 0;
}

void Connected(TCPClient *context)
{
    printf("[Connected]\r\n");
}

void Disconnected(TCPClient *context, int i_error_code)
{
    printf("[Disconnected] error_code: %d\r\n", i_error_code);
}

void DataReceive(TCPClient *context, pkg i_package)
{
    TCPClient *c = context;
    c->write(i_package); // echo
    std::string msg(i_package.begin(), i_package.end());
    printf("[DataReceive]<%d>[%s]\r\n", (int)i_package.size(), msg.c_str());
}