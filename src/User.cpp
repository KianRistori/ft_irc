#include "../include/User.hpp"

User::User(std::string name, int socket) : nickName(name), socket(socket)
{}

std::string User::getNickName() const
{
    return nickName;
}

int User::getSocket() const
{
    return socket;
}

void User::setSocket(int socket)
{
    this->socket = socket;
}

void User::setNickName(std::string &name) {
    this->nickName = name;
}

void User::setRealName(std::string &name) {
    this->realName = name;
}

void User::setUserName(std::string &name) {
    this->userName = name;
}

std::string User::getToken() const {
    return this->token;
}

void    User::setToken(std::string &token) {
    this->token = token;
}

User::~User() { }
