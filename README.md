# IRC Server 42 Group Project - [SUBJECT](ressources/en.subject.pdf)

This project involves developing an **IRC server** in **C++ 98**, designed to handle multiple clients simultaneously while respecting the IRC protocol standards. The goal is to implement a lightweight, efficient, and modular server with robust error handling and non-blocking operations.

---

## **Project Overview** 📚

### Objectives 🎯:
- Develop an **IRC server** running on **TCP/IP**.
- Support **multiple simultaneous clients** without blocking.
- Provide essential **IRC functionalities**, such as authentication, private messaging, and channel management.
- Ensure compliance with a **reference IRC client** like `irssi`.

**Execution :**
```bash
./ircserv <port> <password>
```
- `port` : Port number for incoming connections.
- `password` : Password required for user authentication.

---

## **Technical Requirements** 🔧

### Constraints ⚙️:
- **C++ 98 standard only** (no C++11 or later features).
- **Non-blocking I/O operations**.
- **Only one call to `poll()` or equivalent** (we used `select()`).
- **No forking or multi-threading**.

### Allowed External Functions 🧑‍💻:
- `socket`, `close`, `setsockopt`, `bind`, `connect`, `listen`, `accept`...
- Full list [here](ressources/en.subject.pdf).

---

## **Core Features** ⚡

### Basic Functionalities :
- **Client Authentication** using a password.
- **Handling Multiple Clients** concurrently.
- **Message Parsing and Routing** compliant with the IRC protocol.
- **Proper Error Handling** to avoid crashes and connection failures.

### IRC Commands ⌨️:
- **Connection Commands**: `PASS`, `NICK`, `USER`
- **Messaging**: `PRIVMSG`
- **Channel Management**: `JOIN`, `PART`, `TOPIC`, `MODE`, `INVITE`, `KICK`
- **Operator Commands**: Assigning and managing user privileges.

### Channel Modes (`MODE`) Implementation 🔐:
- `+i` (Invite-only channels)
- `+t` (Only operators can change the topic)
- `+k` (Password-protected channels)
- `+o` (Grant/revoke operator status)
- `+l` (User limit in a channel)

 ### Network Handling 🌐
- **Efficient message broadcasting** using a select-based non-blocking event loop.
- **Packet fragmentation handling** to support broken or delayed messages.

---

## **Project Structure** 🗂️

### File Organization :
```

├── assets
│   └── quotes.txt
├── incs
│   └── bot
│   │   └── Bot.hpp
│   ├── config
│   │   ├── bot_config.hpp
│   │   ├── colors.hpp
│   │   ├── commands.hpp
│   │   ├── irc_config.hpp
│   │   ├── irc_replies.hpp
│   │   ├── server_libs.hpp
│   │   └── server_messages.hpp
│   ├── server
│   │   ├── Channel.hpp
│   │   ├── Client.hpp
│   │   ├── CommandHandler.hpp
│   │   ├── FileData.hpp
│   │   └── Server.hpp
│   └── utils
│       ├── IrcHelper.hpp
│       ├── MessageBuilder.hpp
│       └── Utils.hpp
└── srcs
│   ├── bot
│   │   ├── Bot_Command.cpp
│   │   ├── Bot_Message.cpp
│   │   ├── Bot_Parser.cpp
│   │   ├── Bot_Register.cpp
│   │   ├── Bot.cpp
│   │   └── BotMain.cpp
│   ├── server
│   │   ├── channels
│   │   │   ├── Channel_Actions.cpp
│   │   │   ├── Channel_Attributes.cpp
│   │   │   └── Channel.cpp
│   │   ├── clients
│   │   │   ├── Client_Actions.cpp
│   │   │   ├── Client_Attributes.cpp
│   │   │   └── Client.cpp
│   │   ├── commands
│   │   │   ├── CommandHandler_Channel.cpp
│   │   │   ├── CommandHandler_File.cpp
│   │   │   ├── CommandHandler_Log.cpp
│   │   │   ├── CommandHandler_Message.cpp
│   │   │   ├── CommandHandler_Mode.cpp
│   │   │   ├── CommandHandler_Register.cpp
│   │   │   └── CommandHandler.cpp
│   │   ├── core
│   │   │   ├── Server_Clients.cpp
│   │   │   ├── Server_Infos.cpp
│   │   │   ├── Server_Loop.cpp
│   │   │   └── Server.cpp
│   │   └── ServerMain.cpp
│   └── utils
│       ├── IrcHelper.cpp
│       ├── MessageBuilder.cpp
│       └── Utils.cpp
├── Makefile
└── README.md

```
---

## **Project Approach** 💡

**1. Object-Oriented Design** 💼
- **`Server` Class**: Manages network connections and client sessions.
- **`Client` Class**: Represents an IRC user with its state and actions.
- **`Channel` Class**: Handles channel-specific logic and member management.
- **`CommandHandler` Class**: Parses and executes IRC commands.
- **`Bot` Class (Bonus)**: Implements additional interactive features.

**2. Non-Blocking Event Handling** 🔄
- **File descriptor management** using `select()` for handling multiple clients concurrently.
- **Setting sockets to non-blocking mode** with `fcntl()`:
```bash
fcntl(fd, F_SETFL, O_NONBLOCK);
```

**3. Error Handling & Debugging** ⚠️
- **Try-catch blocks** to prevent crashes.
- **Logging mechanisms** for debugging unexpected behaviors.

---

## **Installation & Compilation** 🛠️

### Compiling the Server and the Bot :
```bash
make
```
- Both `ircserv` and `ircbot` share common utility tools for handling IRC-related operations.
- The compilation process first builds the shared library containing common utilities before linking it to each executable.

### Cleaning the Project :
```bash
make clean    # Removes object files
make fclean   # Removes binaries and object files
make re       # Cleans and recompiles everything
```

---

## **Testing & Usage** 🧪

**If you have any issues, refer to [`irc.connect.help.md`](ressources/irc.connect.help.md)**

### Running `server` 🚀:
```bash
./ircserv 6667 "my_password"
```

### Connecting with `nc` 🌐:
```bash
nc 127.0.0.1 6667
```

- Authentication commands 🛂:
```bash
PASS pass_server
```
```bash
NICK test_user
```
```bash
USER test_user 0 * :Real Name
```
- Packet framentation test with `CTRL+D`:
```bash
com^Dman^Dd
```

### Connecting with `irssi` 💻:
```bash
irssi -c 127.0.0.1 -p 6667 -w "my_password"
```
- Test commands like:
```bash
/join #channel
```
```bash
/msg user Hello!
```
```bash
/msg #channel Hello!
```
```bash
/kick user
```

---

## **Bonus Features** 🎉

### IRC Bot 🤖:
**Bot execution :**
```bash
./ircbot
```

**Interactive commands**:
- `!funfact`: Returns a random tech-related fun fact.
- `!age <YYYY-MM-DD>`: Calculates and displays the user's exact age in years, months, and days.
- `!time`: Displays the current time.

### File Transfer Support (`DCC Protocol`) 📁:
- **Direct peer-to-peer file sharing** between users.

### Advanced Logging System 📑:
- **Detailed event logs** for debugging and server management.
- **Real-time monitoring** of connections and messages.
---
