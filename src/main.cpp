#include "../include/User.hpp"
#include "../include/Channel.hpp"
#include "../include/HandleIrcCommand.hpp"

#define TRUE   1
#define FALSE  0

std::string server_password;

void handleIRCMessage(User &user, std::string const &message, std::vector<User> &users, std::vector<Channel> &channels) {
    std::cout << message << std::endl;

    if (message.find("PASS :") == 0)
        handlePassCommand(user, message, server_password);

    else if (message.find("NICK ") == 0)
        handleNickCommand(user, message, users);

    else if (message.find("USER ") == 0)
        handleUserCommand(user, message);

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

    else if (message.find("MODE") == 0) 
        handleModeCommand(user, message, channels);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    server_password = argv[2];

    int opt = TRUE;
    int master_socket , addrlen , new_socket, activity, valread , sd;
    int max_sd;
    struct sockaddr_in address;

    char buffer[1025];  //data buffer of 1K

    //set of socket descriptors
    fd_set readfds;

    //a message
    const char *message = "ECHO Daemon v1.0 \r\n";

    std::vector<User> users;
    std::vector<Channel> channels;

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections, this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );

    //bind the socket to localhost port
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", port);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while(TRUE)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for (size_t i = 0 ; i < users.size() ; i++)
        {
            //socket descriptor
            sd = users[i].getSocket();

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }


        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely

        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket, then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            //send new connection greeting message
            if (send(new_socket, message, static_cast<ssize_t>(strlen(message)), 0) != static_cast<ssize_t>(strlen(message)))
            {
                perror("send");
            }

            puts("Welcome message sent successfully");

            //add new socket to array of sockets
            User newUser("name", new_socket);
            users.push_back(newUser);
            printf("Adding to list of sockets\n");
        }

        //else its some IO operation on some other socket
        for (size_t i = 0; i < users.size(); i++)
        {
            sd = users[i].getSocket();
            if (FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d , nickname %s\n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port), users[i].getNickName().c_str());

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    users[i].setSocket(0);
                }

                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end of the data read
                    buffer[valread] = '\0';
                    handleIRCMessage(users[i], std::string(buffer), users, channels);
                    // send(sd , buffer , strlen(buffer) , 0 );
                    // printf("buffer: %s", buffer);
                }
            }
        }
    }

    return 0;
}
