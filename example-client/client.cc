#include "TCP_Client.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#define PAUSE printf("Press Enter key to continue...");  fgetc(stdin); 

using namespace ARC;

void Connected(TCP_Client* Context);
void Disconnected(TCP_Client* Context, int ErrCode);
void DataReceive(TCP_Client* Context, pkg Package);
TCP_Client client = TCP_Client("192.168.43.2", 2000);

int main(void)
{    
    printf("Press Enter key to initial tcp client...");  fgetc(stdin); 
    client.Event_Connected = Connected;
    client.Event_DataReceive = DataReceive;
    client.Event_Disconnected = Disconnected;

    printf("Press Enter key to connect to target...");  fgetc(stdin); 
    if(client.Connect())
    {

    }    
    
    printf("Press Enter key to disconnect...");  fgetc(stdin); 
    client.disconnect();

    PAUSE;
    return 0;
}

void Connected(TCP_Client* Context)
{
    printf("Connected\r\n");
}

void Disconnected(TCP_Client* Context, int ErrCode)
{
    printf("Disconnected\r\n"); 
}

void DataReceive(TCP_Client* Context, pkg Package)
{
    TCP_Client* c = Context;
    c->write(Package); // echo

    std::string msg(Package.begin(), Package.end());
    printf("DataReceive: [%s]\r\n",msg.c_str());
    
}