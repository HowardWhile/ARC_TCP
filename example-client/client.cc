#include "TCP_Client.h"
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

std::vector<int> buffer;
std::mutex bufferLock;
void bg_fun()
{
    int i;
    // while (true)
    while (i < 10)
    {
        std::lock_guard<std::mutex> lock(bufferLock);
        {
            buffer.push_back(i);
        }
        std::cout << "bg_fun" << i << "\r\n";
        i++;
    }
}

int main(void)
{
    TCP_Client c;

    std::thread t1(bg_fun);
    std::thread t2(bg_fun);
    std::thread t3(bg_fun);
    t1.join();
    t2.join();
    t3.join();

    std::cout << "complete\r\n";
    return 0;
}