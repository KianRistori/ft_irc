# ft_irc - Internet Relay Chat Server

## Overview
The ft_irc project focuses on creating a custom IRC server using C++98. This server allows users to connect via IRC clients, facilitating real-time message exchange and participation in group channels. The goal is to gain familiarity with Internet standard protocols and deepen understanding of network interactions.

## Contents
- [Introduction](#introduction)
- [General Rules](#general-rules)
- [Mandatory Part](#mandatory-part)
- [Bonus Part](#bonus-part)

## Introduction
IRC, or Internet Relay Chat, is a text-based communication protocol on the Internet. Enabling real-time messaging, users can exchange public or private messages and join thematic channels.

## General Rules
- The program must not crash under any circumstances.
- Submission requires the inclusion of a Makefile with essential rules.
- Code must comply with C++98 standards.
- The use of external libraries, including Boost, is prohibited.

## Mandatory Part - Program ircserv
- **Makefile Rules:** `NAME`, `all`, `clean`, `fclean`, `re`
- **Arguments:** `<port>` (listening port), `<password>` (connection password)
- **External Functions:** Standard C++98 functions
- **Description:** Implementation of an IRC server in C++98 with non-blocking I/O operations.

### Mandatory Requirements
- Capable of handling multiple clients simultaneously.
- Use only 1 `poll()` (or equivalent) for all operations.
- Implementation of functionalities using a specified reference IRC client.

### Test Example
Thorough verification of every possible error and issue, including cases of receiving partial data or low bandwidth.

## Bonus Part
- Possibility to implement additional features such as file transfer or the inclusion of a bot.
- Evaluation of the bonus part only if the mandatory part is executed flawlessly.
