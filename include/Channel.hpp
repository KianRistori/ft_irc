#pragma once

#include <vector>
#include "User.hpp"
class Channel
{
private:
    std::string channelName;
    std::vector<User> userList;
    std::vector<User> invitedUsers;
    std::vector<User> operators;
    std::string topic;
    bool    inviteOnly;
    int     userLimit;
public:
    Channel(std::string name);
    std::string getChannelName() const;
    void addUser(User user);
    bool isOperator(User user) const;
    void addOperators(User user);
    void addInvitesUser(User user);
    bool isInvitedUser(User user);
    std::string getTopic() const;
    void setTopic(std::string &topic);
    bool getInviteOnly() const;
    bool isUserInChannel(User user) const;
    bool checkUserLimit();
    void setUserLimit(int limit);
    void removeUserLimit();
    std::vector<User> getUserList() const;
    void setInviteOnly(bool value);
    bool inviteUser(User user, std::string const &invitedNick, std::string const &channelName,std::vector<User> &users);
    bool kickUser(User user, std::string const &targetNick, std::vector<User> &users);
    void broadcastMessage(const std::string &message);
    void removeUser(User user);
    ~Channel();
};
