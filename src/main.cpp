#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h>
#include <vector>
#include "../include/User.hpp"
#include "../include/Channel.hpp"
     
#define TRUE   1
#define FALSE  0

Channel *findChannel(std::string &name, std::vector<Channel> &channels) {
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (channels[i].getChannelName() == name)
            return &channels[i];
    }
    return NULL;
}

size_t split(const std::string &txt, std::vector<std::string> &strs, char ch)
{
    size_t pos = txt.find( ch );
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    // Add the last one
    strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ) );

    return strs.size();
}

std::string server_password;

void handleIRCMessage(User &user, std::string const &message, std::vector<User> users, std::vector<Channel> &channels) {
    std::cout << message << std::endl;

    if (message.find("PASS :") == 0) {
        std::string pass = message.substr(6);
        pass.erase(pass.length() - 1);

        // Verifica se la password è corretta
        if (pass != server_password) {
            const char *authErrorMessage = "464 * :Password incorrect\r\n";
            send(user.getSocket(), authErrorMessage, strlen(authErrorMessage), 0);
            close(user.getSocket());
            user.setSocket(0);
        }
    }


    if (message.find("NICK ") == 0) {
        std::string newNick = message.substr(5);
        newNick.erase(newNick.length() - 1);
        bool nicknameTaken = false;

        // Verifica se il nickname è già in uso da un altro utente
        for (size_t i = 0; i < users.size(); i++) {
            if (users[i].getNickName() == newNick) {
                nicknameTaken = true;
                break;
            }
        }

        if (nicknameTaken) {
            // Invia un messaggio di errore
            std::string nickTakenMessage = "433 * " + newNick + " :Nickname is already in use\r\n";
            send(user.getSocket(), nickTakenMessage.c_str(), strlen(nickTakenMessage.c_str()), 0);
        } else {
            // Imposta il nuovo nickname per l'utente
            user.setNickName(newNick);

            // Invia una conferma al client
            std::string nickSetMessage = "001 " + newNick + " :Welcome to the IRC server\r\n";
            send(user.getSocket(), nickSetMessage.c_str(), strlen(nickSetMessage.c_str()), 0);
        }
    }

    else if (message.find("USER ") == 0) {
        std::vector<std::string> userParams;
        split(message, userParams, ' ');

        if (userParams.size() >= 5) {
            // userParams[1] contiene il nome utente
            // userParams[4] contiene il nome reale dell'utente

            std::string username = userParams[1];
            std::string realname = userParams[4];

            // Puoi salvare il nome utente e il nome reale dell'utente nei dati dell'oggetto User
            user.setUserName(username);
            user.setRealName(realname);

            // Invia una conferma al client
            std::string userSetMessage = "002 " + user.getNickName() + " :Your host is irc.example.com, running version ExampleIRCServer 1.0\r\n";
            send(user.getSocket(), userSetMessage.c_str(), strlen(userSetMessage.c_str()), 0);
        }
    }

    else if (message.find("PRIVMSG") == 0) {
        std::vector<std::string> splitMessage;
        split(message, splitMessage, ' ');
        std::string nickname = user.getNickName();
        std::string target = splitMessage[1];
        std::string messageText = message.substr(message.find(":", 1) + 1);
        std::string response;
        if (target.at(0) == '#') {
            for (size_t i = 0; i < channels.size(); i++)
            {
                if (channels[i].getChannelName() == target) {
                    std::vector<User> userList = channels[i].getUserList();
                    for (size_t j = 0; j < userList.size(); j++) {
                        if (nickname != userList[j].getNickName()) {
                            response = ":" + nickname + " PRIVMSG " + target + " :" + messageText + "\r\n";
                            send(userList[j].getSocket(), response.c_str(), response.length(), 0);
                        }
                    }
                    return;
                }
            }
            
        }
        else
        {
            for (size_t i = 0; i < users.size(); i++) {
                std::string nick = users[i].getNickName();
                if (nick == target) {
                    response = ":" + nickname + " PRIVMSG " + target + " :" + messageText + "\r\n";
                    send(users[i].getSocket(), response.c_str(), response.length(), 0);
                    return;
                }
            }
        }
    }

   else if (message.find("JOIN") == 0) {
        std::vector<std::string> splitMessage;
        split(message, splitMessage, ' ');
        std::string channelName = splitMessage[splitMessage.size() - 1].erase(splitMessage[splitMessage.size() - 1].length() - 1);

        // Verifica se il canale esiste già
        Channel *existingChannel = NULL;
        for (size_t i = 0; i < channels.size(); i++) {
            if (channels[i].getChannelName() == channelName) {
                existingChannel = &channels[i];
                break;
            }
        }

        if (existingChannel) {
            // Il canale esiste già, quindi aggiungi l'utente al canale
            existingChannel->addUser(user);
            std::cout << user.getNickName() << " join " << existingChannel->getChannelName() << std::endl;

            // Invia il messaggio "JOIN" al client Konversation
            std::string joinMessage = ":" + user.getNickName() + " JOIN " + channelName + "\r\n";
            send(user.getSocket(), joinMessage.c_str(), joinMessage.length(), 0);
        } else {
            // Il canale non esiste, quindi crealo e aggiungi l'utente
            Channel newChannel(channelName);
            newChannel.addUser(user);
            newChannel.addOperators(user);
            channels.push_back(newChannel);
            std::cout << "Channel: " << channelName << " creato" << std::endl;

            // Invia il messaggio "JOIN" al client Konversation
            std::string joinMessage = ":" + user.getNickName() + " JOIN " + channelName + "\r\n";
            send(user.getSocket(), joinMessage.c_str(), joinMessage.length(), 0);
        }
    }

    else if (message.find("INVITE") == 0) {
        // Estrai il nome utente invitato dalla richiesta di invito
        std::vector<std::string> splitMessage;
        split(message, splitMessage, ' ');
        std::string invitedNick = splitMessage[1];

        // Cerca il canale a cui si applica l'invito
        std::string channelName = splitMessage[splitMessage.size() - 1];
        channelName.erase(channelName.length() - 1);
        Channel *channel = findChannel(channelName, channels);

        std::cout << "channel find: " << channel->getChannelName() << std::endl;

        if (channel) {
            // Prova ad invitare l'utente al canale
            if (channel->inviteUser(user, invitedNick, channelName, users)) {
                // Invito riuscito, invia una risposta al client che ha inviato l'invito
                std::string inviteResponse = ":IRCServer 341 " + user.getNickName() + " " + invitedNick + " " + channelName + " :Invitation sent\r\n";
                send(user.getSocket(), inviteResponse.c_str(), inviteResponse.length(), 0);
            } else {
                // Invito fallito, invia un messaggio di errore
                std::string inviteErrorMessage = ":IRCServer 443 " + user.getNickName() + " " + channelName + " " + invitedNick + " :Cannot invite user\r\n";
                send(user.getSocket(), inviteErrorMessage.c_str(), inviteErrorMessage.length(), 0);
            }
        }
    }

    else if (message.find("KICK") == 0) {
        // Estrai il nome utente bersaglio e il nome dell'utente che esegue il comando "KICK"
        std::vector<std::string> splitMessage;
        split(message, splitMessage, ' ');
        std::string channelName = splitMessage[1];
        std::string targetNick = splitMessage[2];
        std::string reason = message.substr(message.find(":", 1) + 1);
        Channel *channel = findChannel(channelName,  channels);

        if (channel) {
            // Prova a eseguire il comando "KICK"
            if (channel->kickUser(user, targetNick, users)) {
                // Espulsione riuscita, puoi inviare una risposta al client che ha eseguito il "KICK" se lo desideri
                std::string kickMessage = ":" + user.getNickName() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
                send(user.getSocket(), kickMessage.c_str(), kickMessage.length(), 0);
            } else {
                // Espulsione fallita, invia un messaggio di errore
                std::string kickErrorMessage = ":IRCServer 441 " + user.getNickName() + " " + channelName + " " + targetNick + " :They aren't in this channel\r\n";
                send(user.getSocket(), kickErrorMessage.c_str(), kickErrorMessage.length(), 0);
            }
        }
    }
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