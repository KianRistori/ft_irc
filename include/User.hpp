#pragma once

#include <string>
#include <vector>
class User
{
private:
    std::string nickName;
    int socket;
public:
    User(std::string name, int socket);
    ~User();
    std::string getNickName() const;
    void setNickName(std::string &name);
    int getSocket() const;
    void setSocket(int socket);
};