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

User::~User()
{}