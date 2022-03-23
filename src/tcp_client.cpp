#include "tcp_client.h"

#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // inet_addr
#include <netdb.h>      // gethostbyname
#include <fcntl.h>      // fcntl
#include <unistd.h>     // close()
#include <signal.h>     // pthread_kill()

// #define DBG_PRINT(...) // disable debug print
// #define DBG_PRINT(...) printf(__VA_ARGS__) // depend on stdio.h

#include "arc_console.hpp"
#define DBG_PRINT(...) console_tag("TCPClient", __VA_ARGS__) // depend on arc_console.hpp

namespace ARC
{
    TCPClient::TCPClient(std::string iIP, int iPort)
    {
        this->_socket_id = -1;
        this->Event_Connected = nullptr;
        this->Event_Disconnected = nullptr;
        this->Event_DataReceive = nullptr;

        this->_ip_address = iIP;
        this->_port = iPort;
    }

    TCPClient::~TCPClient()
    {
    }

    bool TCPClient::Connect(int timeout)
    {
        // 初始化取得Socket ID
        if (this->_socket_id == -1)
        {
            this->_socket_id = socket(AF_INET, SOCK_STREAM, 0);
            if (this->_socket_id == -1)
            {
                DBG_PRINT("Could not create socket.");
                return false;
            }
        }

        sockaddr_in connect_target;

        // 檢查address文字是否錯誤
        if ((int)inet_addr(this->_ip_address.c_str()) == -1)
        {
            struct hostent *he;
            struct in_addr **addr_list;
            if ((he = gethostbyname(this->_ip_address.c_str())) == NULL)
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
            connect_target.sin_addr.s_addr = inet_addr(this->_ip_address.c_str());
        }

        connect_target.sin_family = AF_INET;
        connect_target.sin_port = htons(this->_port);

        DBG_PRINT("socket(%d) start connecting to %s:%d", this->_socket_id, this->_ip_address.c_str(), this->_port);
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
        fcntl(this->_socket_id, F_SETFL, O_NONBLOCK);
        connect(this->_socket_id, (sockaddr *)&connect_target, sizeof(connect_target));
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(this->_socket_id, &fdset);
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

        if (select(this->_socket_id + 1, NULL, &fdset, NULL, &tv) == 1)
        {
            int so_error;
            socklen_t len = sizeof so_error;

            getsockopt(this->_socket_id, SOL_SOCKET, SO_ERROR, &so_error, &len);

            if (so_error == 0)
            {
                DBG_PRINT("socket(%d) connect to server %s:%d", this->_socket_id, this->_ip_address.c_str(), this->_port);
                /* --------------------------------------- */
                // 把組塞的狀態加回去要不然偵測不到斷線
                // http://stackoverflow.com/questions/388434/linux-fcntl-unsetting-flag
                /* --------------------------------------- */
                int oldfl = fcntl(this->_socket_id, F_GETFL);
                if (oldfl == -1)
                {
                    return false; /* handle error */
                }
                fcntl(this->_socket_id, F_SETFL, oldfl & ~O_NONBLOCK);
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
                DBG_PRINT("socket(%d) so_error:%d", this->_socket_id, so_error);
                return false;
            }
        }
        else
        {
            DBG_PRINT("socket(%d) connect to server %s:%d timeout", this->_socket_id, this->_ip_address.c_str(), this->_port);
            return false;
        }
    }

    bool TCPClient::isConnected()
    {
        if (this->_socket_id < 0)
        {
            return false;
        }
        else if (pthread_kill(this->bgRx_id, 0) == ESRCH)
        {
            // https://stackoverflow.com/questions/1693180/how-do-i-determine-if-a-detached-pthread-is-alive
            // bg_rx thread dose not exists or already quit.
            return false;
        }

        return true;
    }

    void TCPClient::disconnect()
    {
        close(this->_socket_id);
        this->bgRxClose();
    }

    int TCPClient::write(ARC::pkg i_package)
    {
        if (i_package.size() > 0 && this->isConnected())
        {
            return this->write(&i_package[0], i_package.size());
        }
    }

    int TCPClient::write(std::string i_message)
    {
        if (i_message.length() > 0 && this->isConnected())
        {
            ARC::pkg pkg(i_message.c_str(), i_message.c_str() + i_message.size());
            return this->write(&pkg[0], pkg.size());
        }
    }

    int TCPClient::write(const char i_byte[], int i_length)
    {
        if (i_length > 0 && this->isConnected())
        {
            int flag = 0; // MSG_OOB | MSG_DONTROUTE
            int ret = send(this->_socket_id, i_byte, i_length, flag);
            if (ret < 0)
                DBG_PRINT("[Socket ID:(%d)] send function error: (%d), close rx thread.", this->_socket_id, ret);

            return ret;
        }
    }

    void ARC::TCPClient::bgRxWork(void)
    {
        // char rx_buffer[2048] 用於緩衝
        std::vector<char> rx_buffer;
        rx_buffer.resize(ARC_TCP_RX_BUFFER_SIZE); //

        int byte2read = 0;

        // main loop
        while (true)
        {
            char *buffer_ptr = &rx_buffer[0];
            int buffer_size = rx_buffer.size();

            int byte2read = recv(this->_socket_id, buffer_ptr, buffer_size, 0); // recv函數會block住直到有情況要處理
            DBG_PRINT("byte2read = %d", byte2read);

            if (byte2read < 0) // 有通訊錯誤發生
            {
                DBG_PRINT("[Socket ID:(%d)] recv function error: (%d), close rx thread.", this->_socket_id, byte2read);
                break;
            }
            else if (byte2read == 0) // 斷線發生
            {
                DBG_PRINT("[Socket ID:(%d)] Detected TcpServer offline, close rx thread.", this->_socket_id);
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
}
