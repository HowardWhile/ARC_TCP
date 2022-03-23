#include "tcp_server.h"
#include <sys/socket.h>
#include <arpa/inet.h> /*inet_ntoa*/
#include <string.h>    // bzero

#define MAX_ACCEPT_NUM 2 // 最大的連接數量

// #define DBG_PRINT(...) // disable debug print
// #define DBG_PRINT(...) printf(__VA_ARGS__) // depend on stdio.h

#include "arc_console.hpp"
#define DBG_PRINT(...) console_tag("TCPServer", __VA_ARGS__) // depend on arc_console.hpp

namespace ARC
{

    TCPServer::TCPServer(int i_port)
    {
        this->_socket_id = -1;
        this->_port = i_port;
    }

    TCPServer::~TCPServer()
    {
    }

    int TCPServer::open()
    {
        // ----------------------------------------------------
        // 如果開啟過先關閉
        this->close();
        // ----------------------------------------------------
        // initial socket
        this->_socket_id = socket(AF_INET, SOCK_STREAM, 0);
        if (this->_socket_id < 0)
        {
            DBG_PRINT("create socket error!");
            return -1;
        }
        DBG_PRINT("create socket ID: (%d)", this->_socket_id);
        // ----------------------------------------------------
        // 如果不正常斷線時下次bind()會出現錯誤直到Timeout(大約1 min)才恢復正常
        // 因此藉由以下方式釋放數值
        // http://stackoverflow.com/questions/5592747/bind-error-while-recreating-socket
        int rst_val = 1;
        if (setsockopt(this->_socket_id, SOL_SOCKET, SO_REUSEADDR, &rst_val, sizeof(rst_val)) < 0)
        {
            DBG_PRINT("setsockopt to remove bind error");
            return -1;
        }
        // ----------------------------------------------------
        // Set Timeout
        timeval timeout;
        timeout.tv_sec = 4;
        timeout.tv_usec = 0;
        if (setsockopt(this->_socket_id, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        {
            DBG_PRINT("setsockopt failed\n");
        }
        // ----------------------------------------------------
        // 初始化 socket 監聽時的參數
        sockaddr_in serverAddr;
        bzero(&serverAddr, sizeof(serverAddr)); // memset 0
        serverAddr.sin_family = AF_INET;        // TCP/IP
        serverAddr.sin_port = htons(this->_port);
        serverAddr.sin_addr.s_addr = htons(INADDR_ANY); // accept any incoming messages
        // ----------------------------------------------------
        // 如果bind port <= 1023 需要sudo權限
        int ret_bind = bind(this->_socket_id, (sockaddr *)&serverAddr, sizeof(serverAddr));
        if (ret_bind < 0)
        {
            DBG_PRINT("binds parameter error! code:(%d)", ret_bind);
            return -1;
        }
        DBG_PRINT("binds parameter to the socket");
        // ----------------------------------------------------
        if (listen(this->_socket_id, MAX_ACCEPT_NUM) < 0)
        {
            DBG_PRINT("listen error!");
            return -1;
        }
        // ----------------------------------------------------
        DBG_PRINT("start listen!");
        this->bgListenStart();
        // ----------------------------------------------------
    }

    void TCPServer::close()
    {
    }

    void TCPServer::bgListenWork(void)
    {
        DBG_PRINT("bgListenWork thread start! socket ID:(%d)", this->_socket_id);
        while (true)
        {
            // 等待連入的訊息
            sockaddr_in accept_msg;
            int addr_len = sizeof(accept_msg);
            int accept_socket_id = accept(this->_socket_id, (sockaddr *)&accept_msg, (socklen_t *)&addr_len); // block
            if (accept_socket_id < 0)
            {
                // 發生錯誤
                DBG_PRINT("accept error: (%d)\r\n", accept_socket_id);
                break;
            }
            else
            {
                // 有client連入
                // -----------------------------------------
                // 把accept_msg轉換成比較好處理的資料型態
                accept_info info;
                info.ip = std::string(inet_ntoa(accept_msg.sin_addr));
                info.port = htons(accept_msg.sin_port);
                info.socket_id = accept_socket_id;
                info.endpoint = info.ip + ":" + std::to_string(info.port);
                // -----------------------------------------
                DBG_PRINT("Detected one accept request!");
                DBG_PRINT("++ accept from [%s]", info.endpoint.c_str());
                // -----------------------------------------
                // try
                // {
                //     char endpoint_str[32];
                //     sprintf(endpoint_str, "%s:%d", inet_ntoa(accept_info.sin_addr), htons(accept_info.sin_port));
                //     accept_list_node *value = new accept_list_node(accept_socket_id, accept_info, pthis);
                //     std::string key(endpoint_str);
                //     pthread_mutex_lock(&pthis->accept_list_mutex);
                //     //pthis->accept_list.insert({accept_info, value });
                //     pthis->accept_list.insert(std::pair<std::string, accept_list_node *>(key, value));
                //     pthread_mutex_unlock(&pthis->accept_list_mutex);

                //     if (pthis->Event_AcceptSocket != NULL)
                //     {
                //         pthis->Event_AcceptSocket(key);
                //     }
                // }
                // catch (std::exception ex)
                // {
                //     std::cout << ex.what();
                // }
            }
        }
        DBG_PRINT("bgListenWork thread exit! socket ID:(%d)", this->_socket_id);
    }

    AcceptClient::AcceptClient()
    {
    }

    AcceptClient::~AcceptClient()
    {
    }

    void AcceptClient::bgRxWork(void)
    {
    }

}
