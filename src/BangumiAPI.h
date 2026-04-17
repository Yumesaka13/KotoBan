#pragma once
#include <string>

class Message
{
public:
    int id;
    std::string nickname;
    std::string content;

public:
    Message() { id = 0; }
    ~Message() {}
};