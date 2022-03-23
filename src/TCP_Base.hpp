#ifndef TCP_BASE_H
#define TCP_BASE_H

/**
    @file TCP_Base

    @brief ARC_TCP的基礎型態

    @author Howard Cheng

    @Contact makubex49@hotmail.com

    @date 2022-03-22
*/

#pragma once

#include <vector>
#include <string>
#include <thread>

namespace ARC
{
    typedef std::vector<char> pkg;
    typedef std::vector<pkg> pkgs;
}

// ------------------------------------------------
// thread
// ------------------------------------------------
// 看不懂嗎?沒關西這個巨集的開發過程與使用方法我記載在這裡
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