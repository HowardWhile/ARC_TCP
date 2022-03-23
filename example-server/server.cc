#include "tcp_server.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#define PAUSE                                 \
    printf("Press Enter key to continue..."); \
    fgetc(stdin);

using namespace ARC;

TCPServer ss = TCPServer(2000);
void Accepted(TCPServer *context, AcceptInfo *i_client_info);
void Disconnected(TCPServer *context, AcceptInfo *i_client_info, int i_error_code);
void DataReceive(TCPServer *context, AcceptInfo *i_client_info, pkg i_package);

int main(void)
{
    ss.Event_Accepted = Accepted;
    ss.Event_DataReceive = DataReceive;
    ss.Event_Disconnected = Disconnected;

    ss.start();
    PAUSE;
    return 0;
}

void Accepted(TCPServer *context, AcceptInfo *i_client_info)
{
    printf("[Accepted][%s]\r\n", i_client_info->endpoint.c_str());
}

void Disconnected(TCPServer *context, AcceptInfo *i_client_info, int i_error_code)
{
    printf("[Disconnected][%s]\r\n", i_client_info->endpoint.c_str());
}

void DataReceive(TCPServer *context, AcceptInfo *i_client_info, pkg i_package)
{
    std::string msg(i_package.begin(), i_package.end());
    printf("[DataReceive][%s]\r\n", i_client_info->endpoint.c_str());
    printf("[DataReceive]<%d>[%s]\r\n", (int)i_package.size(), msg.c_str());

    // echo
    //context->write(i_package, i_client_info->endpoint);

    // echo broadcast
    context->write(i_package);
}
