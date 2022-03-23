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
void DataReceive(TCPServer *context, AcceptInfo *i_client_info, pkg package);

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
}

void Disconnected(TCPServer *context, AcceptInfo *i_client_info, int i_error_code)
{
}

void DataReceive(TCPServer *context, AcceptInfo *i_client_info, pkg package)
{
}
