#include "TCP_Client.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#define PAUSE printf("Press Enter key to continue...");  fgetc(stdin); 


int main(void)
{
    using namespace ARC;
    
    PAUSE;
    // TCP_Client client = TCP_Client("192.168.2.1", 2000);    
    TCP_Client client = TCP_Client("127.0.0.1", 3000);
    PAUSE;
    client.bg_thread1_start();
    client.bg_thread2_start();
    PAUSE;
    client.bg_thread1_close();
    client.bg_thread2_close();
    PAUSE;

    if(client.Connect())
    {

    }
    
    PAUSE;
    return 0;
}
