#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#pragma once

/**
    @file tcp_server

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
    class TCPServer
    {
    public:
        TCPServer(int i_port);
        ~TCPServer();
        // ----------------------------------------
        int start();     // 啟動server
        void shutDown(); // 關閉server
        // ----------------------------------------
        // Write 不指定endpoint就用廣播來傳送訊息
        // ----------------------------------------
        int write(ARC::pkg i_package, std::string i_endpoint = "");
        int write(std::string i_message, std::string i_endpoint = "");
        int write(const char i_byte[], int i_length, std::string i_endpoint = "");
        // ----------------------------------------
        // Event
        // ----------------------------------------
        void (*Event_Accepted)(TCPServer *context, AcceptInfo *i_client_info);
        void (*Event_Disconnected)(TCPServer *context, AcceptInfo *i_client_info, int i_error_code);
        void (*Event_DataReceive)(TCPServer *context, AcceptInfo *i_client_info, pkg i_package);
        // ----------------------------------------
        class AcceptClient // 連入訊息的執行個體
        {
        public:
            AcceptClient(AcceptInfo i_accept_info, TCPServer *i_parent);
            ~AcceptClient();
            int write(const char i_byte[], int i_length);
            bool isConnect();

        private:
            AcceptInfo _accept_info;
            TCPServer *_parent; // 可由此查找這個連入訊息的執行個體是由哪個TCPServer建立的
            // ----------------------------------------
            // Thread for processing incoming messages.
            // ----------------------------------------
            BACKGROUND_WORKER(AcceptClient, bgRx)
            {
                this->bgRxWork();
            }
            void bgRxWork(void);
        };

    private:
        int _socket_id;
        int _port;
        // ----------------------------------------
        // 連入的table與操作時會用到的執行序安全變數
        typedef std::map<std::string, AcceptClient *> accept_table; //[key, value] = [終端點, 連入Cline的執行個體]
        accept_table table_accept_client;
        std::mutex *mutex_accept_client;
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