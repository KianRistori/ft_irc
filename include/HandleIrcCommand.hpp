#pragma once
#include "../include/User.hpp"
#include "../include/Channel.hpp"
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
#include <fstream>
#include <csignal>
#include <cstdio>

# define RPL_KICK(nick, user, channel, kickuser) ":" + nick + "!" + user + "@" + " KICK " + channel + " " + kickuser + "\r\n"

void	handlePassCommand(User &user, std::string const &message, std::string &server_password);
void	handleNickCommand(User &user, std::string const &message, std::vector<User> &users);
void	handleUserCommand(User &user, std::string const &message);
void	handlePrivMsgCommand(User &user, std::string const &message, std::vector<User> &users, std::vector<Channel> &channels);
void	handleJoinCommand(User &user, std::string const &message, std::vector<Channel> &channels);
void	handlePartCommand(User &user, std::string const &message, std::vector<Channel> &channels);
void	handleInviteCommand(User &user, std::string const &message, std::vector<User> &users, std::vector<Channel> &channels);
void	handleTopicCommand(User &user, std::string const &message, std::vector<Channel> &channels);
void	handlePingCommand(User &user, std::string const &message);
void    handleKickCommand(User &user, std::vector<User> &users, std::string const &message, std::vector<Channel> &channels);
void    handleModeCommand(User &user, std::string const &message, std::vector<Channel> &channels);
void	handleQuitCommand(User &user, std::vector<User> &users, std::vector<Channel> &channels);
void    handleDCCOffer(const std::string &message, std::vector<User> &users);
