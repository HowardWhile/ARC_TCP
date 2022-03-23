#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "tcp_base.hpp"

#pragma once
namespace ARC
{
    class TCPClient
    {
    public:
        TCPClient(std::string iIP, int iPort);
        ~TCPClient();

        bool Connect(int timeout = -1);
        bool isConnected();
        void disconnect();
        int write(ARC::pkg i_package);
        int write(std::string i_message);
        int write(const char i_byte[], int i_length);

        // ----------------------------------------
        // Event
        // ----------------------------------------
        void (*Event_Connected)(TCPClient *context);
        void (*Event_Disconnected)(TCPClient *context, int ErrCode);
        void (*Event_DataReceive)(TCPClient *context, pkg Package);

        // ----------------------------------------
        // Thread
        // ----------------------------------------
        BACKGROUND_WORKER(TCPClient, bgRx)
        {
            this->bgRxWork();
        }

    private:
        void bgRxWork(void);
        int _socket_id;
        std::string _ip_address;
        int _port;
    };

}
#endif