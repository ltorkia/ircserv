# IRC Server 42 Project: [EN SUBJECT](https://github.com/ltorkia/ircserv/blob/main/fr.subject.pdf)

This project involves developing an IRC server in C++ 98, designed to handle multiple clients simultaneously while respecting the IRC protocol standards. The goal is to implement a lightweight, efficient, and modular server with robust error handling and non-blocking operations.

---

## **Project Overview**

- Develop an IRC server running on TCP/IP.
- Support multiple simultaneous clients without blocking.
- Provide essential IRC functionalities, such as authentication, private messaging, and channel management.
- Ensure compliance with a reference IRC client like irssi.

**Execution :**
```bash
./ircserv <port> <password>
```
- `port` : Port number for incoming connections.
- `password` : Password required for user authentication.

---

## **Technical Requirements**

### Constraints :
- C++ 98 standard only (no C++11 or later features).
- Non-blocking I/O operations.
- Only one call to poll() or equivalent (we use select()).
- No forking or multi-threading.

### Allowed External Functions :
- `socket`, `close`, `setsockopt`, `bind`, `connect`, `listen`, `accept`...
- See full list [here](https://github.com/ltorkia/ircserv/blob/main/fr.subject.pdf).

---

## **Core Features**

### Basic Functionalities :
•	Client Authentication using a password.
•	Handling Multiple Clients concurrently.
•	Message Parsing and Routing compliant with the IRC protocol.
•	Proper Error Handling to avoid crashes and connection failures.

### IRC Commands :
•	Connection Commands: `PASS`, `NICK`, `USER`
•	Messaging: `PRIVMSG`
•	Channel Management: `JOIN`, `PART`, `TOPIC`, `MODE`, `INVITE`, `KICK`
•	Operator Commands: Assigning and managing user privileges.

### Channel Modes (MODE) Implementation :
•	+i (Invite-only channels)
•	+t (Only operators can change the topic)
•	+k (Password-protected channels)
•	+o (Grant/revoke operator status)
•	+l (User limit in a channel)

 ### Network Handling
•	Efficient message broadcasting using a select-based non-blocking event loop.
•	Packet fragmentation handling to support broken or delayed messages.

---

## **Project Structure**

### File Organization :
```

├── Makefile
├── README.md
├── data
│   └── quotes.txt
├── incs
│   ├── classes
│   │   ├── bot
│   │   │   └── Bot.hpp
│   │   ├── commands
│   │   │   ├── CommandHandler_File.hpp
│   │   │   └── CommandHandler.hpp
│   │   ├── core
│   │   │   ├── Channel.hpp
│   │   │   ├── Client.hpp
│   │   │   └── Server.hpp
│   │   └── utils
│   │       ├── IrcHelper.hpp
│   │       ├── MessageHandler.hpp
│   │       └── Utils.hpp
│   └── config
│       ├── bot.hpp
│       ├── colors.hpp
│       ├── commands.hpp
│       ├── irc_config.hpp
│       ├── irc_replies.hpp
│       ├── server_libs.hpp
│       └── server_messages.hpp
└── srcs
    ├── main.cpp
    ├── bot
    │   ├── Bot_Authenticate.cpp
    │   ├── Bot_CommandHandlerServer.cpp
    │   ├── Bot_CommandHandlerUser.cpp
    │   ├── Bot.cpp
    │   ├── Bot_MessageStream.cpp
    │   ├── Bot_ParsingHelper.cpp
    │   └── main.cpp
    ├── commands
    │   ├── CommandHandler_Auth.cpp
    │   ├── CommandHandler_Channel.cpp
    │   ├── CommandHandler.cpp
    │   ├── CommandHandler_File.cpp
    │   ├── CommandHandler_Log.cpp
    │   ├── CommandHandler_Message.cpp
    │   └── CommandHandler_Mode.cpp
    ├── core
    │   ├── Channel.cpp
    │   ├── Client.cpp
    │   └── Server.cpp
    └── utils
        ├── IrcHelper.cpp
        ├── MessageHandler.cpp
        └── Utils.cpp
```
---

## **Project Approach**

1. Object-Oriented Design
•	Server Class: Manages network connections and client sessions.
•	Client Class: Represents an IRC user with its state and actions.
•	Channel Class: Handles channel-specific logic and member management.
•	CommandHandler Class: Parses and executes IRC commands.
•	Bot Class (Bonus): Implements additional interactive features.

2. Non-Blocking Event Handling
•	File descriptor management using select() for efficient client handling.
•	Asynchronous message parsing to process multiple requests simultaneously.

3. Error Handling & Debugging
•	Try-catch blocks to prevent crashes.
•	Logging mechanisms for debugging unexpected behaviors.

---

## **Testing & Usage**

### Connect to server :
```bash
./ircserv 127.0.0.1 6667
```

### Basic Test with `nc` :
```bash
PASS pass_server
NICK test_user
USER test_user 0 * :Real Name
```
Packet framentation test with CTRL+D  :
```bash
com^Dman^Dd
```

### Connecting with irssi :
```bash
irssi 127.0.0.1 6667 -c <nickname> -w <password>
```
Test commands like:
	•	/join #test_channel
	•	/msg user Hello!
	•	/kick user

---

## **Bonus Features**

### IRC Bot :
•	Interactive commands: !time, !funfact, !age.
•	Automated responses for user interactions.
•	Role management for different access levels.

**Bot execution :**
```bash
./ircbot
```

### File Transfer Support (DCC Protocol) :
•	Direct peer-to-peer file sharing between users.
•	Secure authentication for file transfers.

### Advanced Logging System :
•	Detailed event logs for debugging and server management.
•	Real-time monitoring of connections and messages.
---
