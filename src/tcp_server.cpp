#include "tcp_server.h"
#include <sys/socket.h>
#include <arpa/inet.h> // inet_ntoa
#include <string.h>    // bzero
#include <unistd.h>    // close()
#include <signal.h>    // pthread_kill()

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

        this->mutex_accept_client = new std::mutex();

        this->Event_Accepted = nullptr;
        this->Event_DataReceive = nullptr;
        this->Event_Disconnected = nullptr;
    }

    TCPServer::~TCPServer()
    {
        delete this->mutex_accept_client;
    }

    int TCPServer::start()
    {
        // ----------------------------------------------------
        // 如果開啟過先關閉
        this->shutDown();
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

    void TCPServer::shutDown()
    {
    }

    int TCPServer::write(ARC::pkg i_package, std::string i_endpoint)
    {
        return this->write(&i_package[0], i_package.size(), i_endpoint);
    }

    int TCPServer::write(std::string i_message, std::string i_endpoint)
    {
        ARC::pkg pkg(i_message.c_str(), i_message.c_str() + i_message.size());
        return this->write(&pkg[0], pkg.size(), i_endpoint);
    }

    int TCPServer::write(const char i_byte[], int i_length, std::string i_endpoint)
    {
        if (i_length > 0)
        {
            if (i_endpoint == "") //廣播
            {
                // lock
                std::lock_guard<std::mutex> lock(*this->mutex_accept_client);

                // for loop table
                for (auto const &client : this->table_accept_client)
                {
                    client.second->write(i_byte, i_length);
                }
            }
            else // 指定目標
            {
                // lock
                std::lock_guard<std::mutex> lock(*this->mutex_accept_client);
                accept_table::iterator it = this->table_accept_client.find(i_endpoint);
                if (it != this->table_accept_client.end())
                {
                    it->second->write(i_byte, i_length);
                }
            }
        }
        return 0;
    }

    void TCPServer::bgListenWork(void)
    {
        DBG_PRINT("bgListenWork thread start! socket ID:(%d) thread ID (0x%X)", this->_socket_id, (unsigned)this->bgListen_id);
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
                AcceptInfo info;
                info.ip = std::string(inet_ntoa(accept_msg.sin_addr));
                info.port = htons(accept_msg.sin_port);
                info.socket_id = accept_socket_id;
                info.endpoint = info.ip + ":" + std::to_string(info.port);
                // -----------------------------------------
                DBG_PRINT("Detected one accept request!");
                DBG_PRINT("++ accept from [%s]", info.endpoint.c_str());
                // -----------------------------------------
                {
                    // lock起後建立AcceptClient的執行任務
                    std::lock_guard<std::mutex> lock(*this->mutex_accept_client); // lock table_accept_client
                    this->table_accept_client.insert({info.endpoint, new AcceptClient(info, this)});
                }

                if (this->Event_Accepted != nullptr)
                {
                    this->Event_Accepted(this, &info);
                }
            }
        }
        DBG_PRINT("bgListenWork thread exit! socket ID:(%d) thread ID (0x%X)", this->_socket_id, (unsigned)this->bgListen_id);
    }

    TCPServer::AcceptClient::AcceptClient(AcceptInfo i_accept_info, TCPServer *i_parent)
    {
        this->_accept_info = i_accept_info;
        this->_parent = i_parent;
        this->bgRxStart();
    }

    TCPServer::AcceptClient::~AcceptClient()
    {
        close(this->_accept_info.socket_id);
        this->bgRxClose();
    }

    void TCPServer::AcceptClient::bgRxWork(void)
    {
        DBG_PRINT("bg_rx thread start! socket ID:(%d), thread ID (0x%X)", this->_accept_info.socket_id, (unsigned)this->bgRx_id);

        // char rx_buffer[2048] 用於緩衝
        std::vector<char> rx_buffer;
        rx_buffer.resize(ARC_TCP_RX_BUFFER_SIZE); //

        int byte2read = 0;

        // main loop
        while (true)
        {
            char *buffer_ptr = &rx_buffer[0];
            int buffer_size = rx_buffer.size();

            int byte2read = recv(this->_accept_info.socket_id, buffer_ptr, buffer_size, 0); // recv函數會block住直到有情況要處理
            DBG_PRINT("byte2read = %d", byte2read);

            if (byte2read < 0) // 有通訊錯誤發生
            {
                DBG_PRINT("[Socket ID:(%d)] recv function error: (%d), close rx thread.", this->_accept_info.socket_id, byte2read);
                break;
            }
            else if (byte2read == 0) // 斷線發生
            {
                DBG_PRINT("[Socket ID:(%d)] Detected TcpServer offline, close rx thread.", this->_accept_info.socket_id);
                break;
            }
            else // 正常接收到訊息
            {
                pkg package;
                package.assign(rx_buffer.begin(), rx_buffer.begin() + byte2read);
                if (this->_parent->Event_DataReceive != nullptr)
                {
                    this->_parent->Event_DataReceive(this->_parent, &this->_accept_info, package);
                }
            }
        }

        if (this->_parent->Event_Disconnected != nullptr)
        {
            this->_parent->Event_Disconnected(this->_parent, &this->_accept_info, byte2read);
        }
    }

    int TCPServer::AcceptClient::write(const char i_byte[], int i_length)
    {
        if (this->isConnect())
        {
            send(this->_accept_info.socket_id, i_byte, i_length, 0);
        }
        return 0;
    }

    bool TCPServer::AcceptClient::isConnect()
    {
        if (this->_accept_info.socket_id < 0)
        {
            return false;
        }
        else if (pthread_kill(this->bgRx_id, 0) == ESRCH)
        {
            // bg_rx thread did not exists or already quit.
            return false;
        }
        return true;
    }
}
