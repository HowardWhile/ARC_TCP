#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "TCP_Base.hpp"

#pragma once
namespace ARC
{
    class TCP_Client
    {
    public:
        TCP_Client(std::string iIP, int iPort);
        ~TCP_Client(); 

        bool Connect(int timeout = -1);

        // ----------------------------------------
        // Event
        // ----------------------------------------
        void (*Event_Connected)(TCP_Client* context);
        void (*Event_Disconnected)(TCP_Client* context);
        void (*Event_DataReceive)(TCP_Client* context);

        // ----------------------------------------
        // Thread
        // ----------------------------------------
        // https://stackoverflow.com/questions/1151582/pthread-function-from-a-class
        pthread_t bg_rx;
        static void* bg_rx_sLink(void* iContext)
        {
            return ((TCP_Client*)iContext)->bg_rx_DoWork();
        }
        void* bg_rx_DoWork(void)
        {
            int i = 0;
            while (true)
            {
                printf("HelloWorld %d \r\n",i++);
            }            
            return 0;
        }
        void bg_rx_start()
        {
            pthread_create(&bg_rx, NULL, &TCP_Client::bg_rx_sLink, this);
        }
        void bg_rx_stop()
        {
            pthread_cancel(bg_rx);
        }
        // ----------------------------------------



    private:
        void init();
        int socket_id;
        std::string ip_address;
        int port;


        
    };

}
#endif