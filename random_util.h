#pragma once
#include <string>
#include <random>

// I can't remember why I made this a thing, however it looks fun so I'm keeping it.  フフ～

inline std::string generateRandomString(size_t length) {
    static const char charset[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, sizeof(charset) - 2);

    std::string str;
    for (size_t i = 0; i < length; ++i)
        str += charset[distrib(gen)];
    return str;
}
