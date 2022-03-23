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
        bool isConnected();
        void disconnect();
        int write(ARC::pkg i_package);
        int write(std::string i_message);
        int write(const char i_byte[], int i_length);

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