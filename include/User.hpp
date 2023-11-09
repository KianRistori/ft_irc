#pragma once

#include <string>
#include <vector>

class User
{
private:
    std::string nickName;
    std::string userName;
    std::string realName;
    std::string token;
    int socket;
public:
    User(std::string name, int socket);
    ~User();
    bool operator==(User user);
    std::string getNickName() const;
    void setNickName(std::string &name);
    void setUserName(std::string &name);
    void setRealName(std::string &name);
    std::string getToken() const;
    void setToken(std::string &token);
    int getSocket() const;
    void setSocket(int socket);
    std::string getUserName() const;
};
