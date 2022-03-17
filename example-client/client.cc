#include "TCP_Client.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
int main(void)
{
    using namespace ARC;
    printf("Press Enter key to continue...");  
    fgetc(stdin);  

    // TCP_Client client = TCP_Client("192.168.2.1", 2000);
    TCP_Client client = TCP_Client("127.0.0.1", 3000);
    if(client.Connect())
    {

    }
    
    printf("Press Enter key to continue...");  
    fgetc(stdin);  
    return 0;
}
