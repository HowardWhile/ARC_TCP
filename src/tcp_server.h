#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#pragma once

/**
    @file TCPServer

    @brief TCP Server的API

    @author Howard Cheng

    @Contact makubex49@hotmail.com

    @date 2022-03-23
*/

#include "tcp_base.hpp"
namespace ARC
{
    class TCPServer
    {
    public:
        TCPServer(int i_port);
        ~TCPServer();

        int open();   // 啟動server
        void close(); // 關閉server

    private:
        int _socket_id;
        int _port;
        // ----------------------------------------
        // Thread used to handle clinet connect
        // ----------------------------------------
        BACKGROUND_WORKER(TCPServer, bgListen)
        {
            this->bgListenWork();
        }
        void bgListenWork(void);
        // ----------------------------------------
    };

    class AcceptClient // 每個連入訊息
    {
    private:
    public:
        AcceptClient();
        ~AcceptClient();

    private:
        // ----------------------------------------
        // Thread for processing incoming messages.
        // ----------------------------------------
        BACKGROUND_WORKER(AcceptClient, bgRx)
        {
            this->bgRxWork();
        }
        void bgRxWork(void);
    };
}

#endif