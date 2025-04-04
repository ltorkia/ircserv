# IRC connection guide 📚

- This file contains common commands for running the IRC server and client.
- Use `valgrind` for memory leak detection when debugging the server.
- The `quote` command is primarily used during the authentication phase of the connection.
- Use logging commands for troubleshooting and debugging the IRC communication.

---

## Running the IRC Server 🚀
**To start the server on port `6667` with the password `bla`:**
```bash
./ircserv 6667 "bla"
```

### With flags for debugging and memory checks 🛠️
**To run the server with memory leak checks using `valgrind`:**
```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s --track-fds=yes ./ircserv 6667 "bla"
```

## Testing with Netcat 🌐
**To test the server by connecting with Netcat:**
```bash
nc 127.0.0.1 6667
```

## Using Irssi as a Client 💻

### Starting Irssi (without parameters) 🖥️
**To launch Irssi without any parameters:**
```bash
irssi
```

### Connect to the server (without parameters) 🔌
**To connect to the IRC server (using default settings):**
```bash
/connect 127.0.0.1 6667
```

### Connect to the server with parameters (nickname and password) 🔑
**To connect to the server with a specific nickname `Lee` and password `bla`:**
```bash
irssi -c 127.0.0.1 -p 6667 -n "Lee" -w "bla"
```

### Quote Command (for authentication, then not needed after) 🔓
**To authenticate and send a raw command to the server:**
```bash
/quote + <your_command_here>
```

## Window Management in Irssi 🪟

- **To change windows (channels):** `Alt + <window_number>`
- **To send Irssi to the background without disconnecting:** `CTRL + Z`
- **To bring Irssi back to the foreground:** `fg`

## Debugging and Logging in Irssi 🐞

- **To enable autologging:**
```bash
  /SET autolog ON
  ```
- **To open a log file and start logging IRC messages:**
```bash
  /LOG OPEN irclog.txt
  ```

## Quit Irssi ❌
**To quit Irssi and disconnect from the server:**
```bash
/quit
```
