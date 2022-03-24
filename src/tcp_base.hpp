#ifndef TCP_BASE_H
#define TCP_BASE_H

#pragma once

/**
    @file TCP_Base

    @brief ARC_TCP的基礎定義與License宣告

    @author Howard Cheng

    @Contact makubex49@hotmail.com

    @date 2022-03-24

    @version v0.1.0
    
*/
/**
MIT License

Copyright (c) 2022 Howard Cheng

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/


#include <vector>
#include <string>
#include <thread>

#define ARC_TCP_RX_BUFFER_SIZE 2048
namespace ARC
{
    typedef std::vector<char> pkg;
    typedef std::vector<pkg> pkgs;

    typedef struct
    {
        std::string endpoint; // ex: 192.168.1.1:50000 我用該字串這個作為連入列表的key
        std::string ip;       // ex: 192.168.1.1
        int port;             // ex: 50000
        int socket_id;        //
    } AcceptInfo;
}

// ------------------------------------------------
// thread
// ------------------------------------------------
// 看不懂嗎?沒關係這個巨集的開發過程與使用方法我紀錄在這裡
// https://github.com/HowardWhile/pthread-macro
// ------------------------------------------------
#define BACKGROUND_WORKER_CLOSE(cName, bgName) \
public:                                        \
    void bgName##Close(void)                   \
    {                                          \
        pthread_cancel(this->bgName##_id);     \
    }

#define BACKGROUND_WORKER_START(cName, bgName)                           \
public:                                                                  \
    void bgName##Start(void)                                             \
    {                                                                    \
        pthread_create(&bgName##_id, NULL, &cName::bgName##SLink, this); \
    }

#define BACKGROUND_WORKER_DOWORK(cName, bgName)        \
private:                                               \
    pthread_t bgName##_id;                             \
                                                       \
private:                                               \
    static void *bgName##SLink(void *i_context)        \
    {                                                  \
        return ((cName *)i_context)->bgName##DoWork(); \
    }                                                  \
                                                       \
private:                                               \
    void *bgName##DoWork(void)

#define BACKGROUND_WORKER(ClassName, ThreadName)   \
    BACKGROUND_WORKER_START(ClassName, ThreadName) \
    BACKGROUND_WORKER_CLOSE(ClassName, ThreadName) \
    BACKGROUND_WORKER_DOWORK(ClassName, ThreadName)
// ------------------------------------------------
// ------------------------------------------------

#endif