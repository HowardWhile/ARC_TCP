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
        void Disconnect();

        // ----------------------------------------
        // Event
        // ----------------------------------------
        void (*Event_Connected)(TCP_Client *context);
        void (*Event_Disconnected)(TCP_Client *context, int ErrCode);
        void (*Event_DataReceive)(TCP_Client *context, pkg Package);

        // ----------------------------------------
        // Thread
        // ----------------------------------------
        BACKGROUND_WORKER(TCP_Client, bgRx)
        {
            this->bgRxWork();
        }

    private:
        void init();
        void bgRxWork(void);
        int socket_id;
        std::string ip_address;
        int port;
    };

}
#endif