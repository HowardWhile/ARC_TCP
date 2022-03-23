#include "TCPServer.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#define PAUSE printf("Press Enter key to continue...");  fgetc(stdin); 

using namespace ARC;

TCPServer ss = TCPServer(2000);

int main(void)
{    
    ss.open();
    PAUSE;
    return 0;
}
