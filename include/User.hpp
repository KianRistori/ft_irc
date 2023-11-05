#pragma once

#include <string>
#include <vector>
#include <map>
class User
{
private:
    std::string nickName;
    std::string userName;
    std::string realName;
    int socket;
public:
    User(std::string name, int socket);
    ~User();
    std::string getNickName() const;
    void setNickName(std::string &name);
    void setUserName(std::string &name);
    void setRealName(std::string &name);
    int getSocket() const;
    void setSocket(int socket);
};