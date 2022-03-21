#ifndef TCP_BASE_H
#define TCP_BASE_H

/**
    @file TCP_Base

    @brief ARC_TCP的基礎型態

    @author Howard Cheng

    @Contact makubex49@hotmail.com

    @date 2022-03-17
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

#define BACKGROUND_WORKER_CLOSE(cName, bgName) \
    void bg_##bgName##_close(void)             \
    {                                          \
        pthread_cancel(this->bg_##bgName);     \
    }

#define BACKGROUND_WORKER_START(cName, bgName)                               \
    void bg_##bgName##_start(void)                                           \
    {                                                                        \
        pthread_create(&bg_##bgName, NULL, &cName::bg_##bgName##_sLink, this); \
    }

#define BACKGROUND_WORKER_DOWORK(cName, bgName)           \
    pthread_t bg_##bgName;                                \
    static void *bg_##bgName##_sLink(void *iContext)        \
    {                                                     \
        return ((cName *)iContext)->bg_##bgName##_DoWork(); \
    }                                                     \
    void *bg_##bgName##_DoWork(void)

#define BACKGROUND_WORKER(ClassName, ThreadName)   \
    BACKGROUND_WORKER_START(ClassName, ThreadName) \
    BACKGROUND_WORKER_CLOSE(ClassName, ThreadName) \
    BACKGROUND_WORKER_DOWORK(ClassName, ThreadName)

#endif