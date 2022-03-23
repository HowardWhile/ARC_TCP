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
#include <map>
#include <mutex>
namespace ARC
{
    class AcceptClient // 每個連入訊息
    {
    private:
    public:
        AcceptClient(AcceptInfo i_accept_info, void* i_context);
        ~AcceptClient();

    private:
        AcceptInfo accept_info;
        // ----------------------------------------
        // Thread for processing incoming messages.
        // ----------------------------------------
        BACKGROUND_WORKER(AcceptClient, bgRx)
        {
            this->bgRxWork();
        }
        void bgRxWork(void);
    };

    class TCPServer
    {
    public:
        TCPServer(int i_port);
        ~TCPServer();

        int open();   // 啟動server
        void close(); // 關閉server

        // ----------------------------------------
        // 連入的table與操作時會用到的執行序安全變數
        std::map<std::string, AcceptClient> table_accept_client;
        std::mutex* mutex_accept_client;
        // ----------------------------------------

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
    
}

#endif