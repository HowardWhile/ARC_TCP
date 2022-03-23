#include "TCP_Client.h"

#include "arc_console.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // inet_addr
#include <netdb.h>      // gethostbyname
#include <fcntl.h>      // fcntl
#include <unistd.h>     // close()

// #define DBG_PRINT(...) // disable debug print
// #define DBG_PRINT(...) printf(__VA_ARGS__) // depend on stdio.h
#define DBG_PRINT(...) console_tag("TCP_Client", __VA_ARGS__) // depend on arc_console.hpp

ARC::TCP_Client::TCP_Client(std::string iIP, int iPort)
{
    this->init();
    this->ip_address = iIP;
    this->port = iPort;
}

ARC::TCP_Client::~TCP_Client()
{
}

void ARC::TCP_Client::init()
{
    this->socket_id = -1;
    this->Event_Connected = nullptr;
    this->Event_Disconnected = nullptr;
    this->Event_DataReceive = nullptr;
}

bool ARC::TCP_Client::Connect(int timeout)
{
    // 初始化取得Socket ID
    if (this->socket_id == -1)
    {
        this->socket_id = socket(AF_INET, SOCK_STREAM, 0);
        if (this->socket_id == -1)
        {
            DBG_PRINT("Could not create socket.");
            return false;
        }
    }

    sockaddr_in connect_target;

    // 檢查address文字是否錯誤
    if ((int)inet_addr(this->ip_address.c_str()) == -1)
    {
        struct hostent *he;
        struct in_addr **addr_list;
        if ((he = gethostbyname(this->ip_address.c_str())) == NULL)
        {
            herror("gethostbyname");
            DBG_PRINT("Failed to resolve hostname");
            return false;
        }
        addr_list = (struct in_addr **)he->h_addr_list;
        for (int i = 0; addr_list[i] != NULL; i++)
        {
            connect_target.sin_addr = *addr_list[i];
            break;
        }
    }
    else
    {
        connect_target.sin_addr.s_addr = inet_addr(this->ip_address.c_str());
    }

    connect_target.sin_family = AF_INET;
    connect_target.sin_port = htons(this->port);

    DBG_PRINT("socket(%d) start connecting to %s:%d", this->socket_id, this->ip_address.c_str(), this->port);
    // ---------------------------------------------------------------------
    // 簡易連線的方法 (捨棄)
    // ---------------------------------------------------------------------
    // if (connect(this->socket_id, (struct sockaddr *)&connect_target, sizeof(connect_target)) < 0)
    // {
    // 	perror("connect failed. Error");
    // 	return false;
    // }
    // return true;
    // ---------------------------------------------------------------------
    // ---------------------------------------------------------------------
    // 連線同時檢查timeout的方法 (採用)
    // http://stackoverflow.com/questions/2597608/c-socket-connection-timeout
    // ---------------------------------------------------------------------
    fcntl(this->socket_id, F_SETFL, O_NONBLOCK);
    connect(this->socket_id, (sockaddr *)&connect_target, sizeof(connect_target));
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(this->socket_id, &fdset);
    timeval tv;
    if (timeout == -1)
    {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
    }
    else
    {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
    }

    if (select(this->socket_id + 1, NULL, &fdset, NULL, &tv) == 1)
    {
        int so_error;
        socklen_t len = sizeof so_error;

        getsockopt(this->socket_id, SOL_SOCKET, SO_ERROR, &so_error, &len);

        if (so_error == 0)
        {
            DBG_PRINT("socket(%d) connect to server %s:%d", this->socket_id, this->ip_address.c_str(), this->port);
            /* --------------------------------------- */
            // 把組塞的狀態加回去要不然偵測不到斷線
            // http://stackoverflow.com/questions/388434/linux-fcntl-unsetting-flag
            /* --------------------------------------- */
            int oldfl = fcntl(this->socket_id, F_GETFL);
            if (oldfl == -1)
            {
                return false; /* handle error */
            }
            fcntl(this->socket_id, F_SETFL, oldfl & ~O_NONBLOCK);
            /* --------------------------------------- */
            // 到這邊就成功連線了
            // 把rx執行序打開來
            /* --------------------------------------- */
            this->bgRxStart();
            /* --------------------------------------- */

            if (this->Event_Connected != nullptr)
            {
                this->Event_Connected(this);
            }
            return true;
        }
        else
        {
            DBG_PRINT("socket(%d) so_error:%d", this->socket_id, so_error);
            return false;
        }
    }
    else
    {
        DBG_PRINT("socket(%d) connect to server %s:%d timeout", this->socket_id, this->ip_address.c_str(), this->port);
        return false;
    }
}

void ARC::TCP_Client::Disconnect()
{
    close(this->socket_id);
    this->bgRxClose();
}

void ARC::TCP_Client::bgRxWork(void)
{
    // char rx_buffer[2048] 用於緩衝
    std::vector<char> rx_buffer;
    rx_buffer.resize(2048);

    int byte2read = 0;

    // main loop
    while (true)
    {
        char *buffer_ptr = &rx_buffer[0];
        int buffer_size = rx_buffer.size();

        int byte2read = recv(this->socket_id, buffer_ptr, buffer_size, 0); // recv函數會block住直到有情況要處理
        DBG_PRINT("byte2read = %d", byte2read);

        if (byte2read < 0) // 有通訊錯誤發生
        {
            DBG_PRINT("[Socket ID:(%d)] recv function error: (%d), close rx thread.", this->socket_id, byte2read);
            break;
        }
        else if (byte2read == 0) // 斷線發生
        {
            DBG_PRINT("[Socket ID:(%d)] Detected TcpServer offline, close rx thread.", this->socket_id);
            break;
        }
        else
        {
            pkg package;
            package.assign(rx_buffer.begin(), rx_buffer.begin() + byte2read);

            if (this->Event_DataReceive != nullptr)
            {
                this->Event_DataReceive(this, package);
            }
        }
    }

    if (this->Event_Disconnected != nullptr)
    {
        this->Event_Disconnected(this, byte2read);
    }
}
