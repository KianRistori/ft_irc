#pragma once

#include <vector>
#include "User.hpp"
class Channel
{
private:
    std::string channelName;
    std::vector<User> userList;
    std::vector<User> operators;
    std::string topic;
public:
    Channel(std::string name);
    std::string getChannelName() const;
    void addUser(User user);
    bool isOperator(User user) const;
    void addOperators(User user);
    std::string getTopic() const;
    void setTopic(std::string &topic);
    bool isUserInChannel(User user) const;
    std::vector<User> getUserList() const;
    bool inviteUser(User user, std::string const &invitedNick, std::string const &channelName,std::vector<User> &users);
    bool kickUser(User user, std::string const &targetNick, std::vector<User> &users);
    void broadcastMessage(const std::string &message);
    void removeUser(User user);
    ~Channel();
};
