#include <vector>
#include "User.hpp"
class Channel
{
private:
    std::string channelName;
    std::vector<User> userList;
public:
    Channel(std::string name);
    std::string getChannelName() const;
    void addUser(User user);
    ~Channel();
};