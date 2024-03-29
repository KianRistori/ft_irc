#include "../include/User.hpp"
#include "../include/Channel.hpp"
#include "../include/HandleIrcCommand.hpp"
#include "../include/IrcBot.hpp"

#define TRUE   1
#define FALSE  0

std::vector<User> users;
std::vector<Channel> channels;

static void signalHandler(int signum) {
    std::cout << "Irc server stop running" << std::endl;
    users.clear();
    channels.clear();
    exit(signum);
}

void handleIRCMessage(User &user, std::string const &message, std::vector<User> &users, std::vector<Channel> &channels, std::string server_password) {
    std::cout << message << std::endl;
    IrcBot bot;

    if (message.find("PASS :") == 0)
        handlePassCommand(user, message, server_password);

    else if (message.find("NICK ") == 0)
        handleNickCommand(user, message, users);

    else if (message.find("USER ") == 0)
        handleUserCommand(user, message);

    else if (message.find("PRIVMSG") == 0 && message.find("DCC SEND") != std::string::npos)
        handleDCCOffer(message, users);

    else if (message.find("PRIVMSG") == 0)
        handlePrivMsgCommand(user, message, users, channels);

    else if (message.find("JOIN") == 0)
        handleJoinCommand(user, message, channels);

    else if (message.find("PART") == 0)
        handlePartCommand(user, message, channels);

    else if (message.find("INVITE") == 0)
        handleInviteCommand(user, message, users, channels);

    else if (message.find("TOPIC") == 0)
        handleTopicCommand(user, message, channels);

    else if (message.find("PING") == 0)
        handlePingCommand(user, message);

    else if (message.find("KICK") == 0)
        handleKickCommand(user, users, message, channels);

    else if (message.find("MODE") == 0)
        handleModeCommand(user, message, channels);

    else if (message.find("QUIT") == 0)
        handleQuitCommand(user, users, channels);

    else if (message.compare("!info\n") == 0)
        bot.replyInfo(user);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    std::string server_password = argv[2];

    int opt = TRUE;
    int master_socket , addrlen , new_socket, activity, valread , sd;
    int max_sd;
    struct sockaddr_in address;

    char buffer[1025];

    std::string receivedMessage;

    fd_set readfds;

    const char *message = "ECHO Daemon v1.0 \r\n";

    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );

    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "Listener on port: " << port << std::endl;

    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    std::cout << "Waiting for connections ..." << std::endl;

    signal(SIGINT, signalHandler);

    while(TRUE)
    {
        FD_ZERO(&readfds);

        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        for (size_t i = 0 ; i < users.size() ; i++)
        {
            sd = users[i].getSocket();

            if(sd > 0)
                FD_SET( sd , &readfds);

            if(sd > max_sd)
                max_sd = sd;
        }

        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
            std::cout << "select error" << std::endl;

        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            std::cout << "New connection , socket fd is: " << new_socket << ", ip is: " << inet_ntoa(address.sin_addr)  << ", port: " << ntohs(address.sin_port) << std::endl;

            if (send(new_socket, message, static_cast<ssize_t>(strlen(message)), 0) != static_cast<ssize_t>(strlen(message)))
            {
                perror("send");
            }

            std::cout << "Welcome message sent successfully" << std::endl;

            User newUser("newUser", new_socket);
            users.push_back(newUser);
            std::cout << "Adding to list of sockets" << std::endl;
            IrcBot bot;
            bot.infoMessage(newUser);
        }

        for (size_t i = 0; i < users.size(); i++)
        {
            sd = users[i].getSocket();
            if (FD_ISSET( sd , &readfds))
            {
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    std::cout << "Host disconnected ip: " << inet_ntoa(address.sin_addr) << ", port: " << ntohs(address.sin_port) << ", nickname: " << users[i].getNickName() << std::endl;
                    close( sd );
                    users[i].setSocket(0);
                }

                else
                {
                    receivedMessage += std::string(buffer, valread);
                    if (buffer[valread - 1] == '\n') {
                        handleIRCMessage(users[i], receivedMessage, users, channels, server_password);
                        receivedMessage.clear();
                    }
                }
            }
        }
    }

    return 0;
}
