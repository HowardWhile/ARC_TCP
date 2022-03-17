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

#endif