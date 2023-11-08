#pragma once

#include "../include/Channel.hpp"
#include "../include/User.hpp"

void modSetInviteOnly(Channel *targetChannel ,std::string &modeChange);
void modSetLimit(Channel *targetChannel ,std::string &modeChange, std::vector<std::string> splitMessage, User user);