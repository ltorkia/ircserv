#pragma once

// === STANDARD LIBRARY : IN/OUT, STREAM, ERRORS ===
#include <iostream>				// gestion chaînes de caractères -> std::cout, std::cerr, std::string
#include <iomanip>				// gestion formatage -> std::setw(), std::setfill()
#include <sstream>				// gestion flux -> std::ostringstream
#include <cstdio>				// perror()
#include <cerrno>				// codes erreur -> errno
#include <stdexcept>			// exceptions -> std::runtime_error etc...

// === CONTAINERS AND ALGORITHMS ===
#include <map>					// container map
#include <vector>				// container vector
#include <set>					// container set
#include <algorithm>			// transform()

// === NETWORK LIBRARY ===
#include <arpa/inet.h>			// gestion adresses IP -> inet_ntoa()
#include <ifaddrs.h>			// gestion interfaces réseau -> getifaddrs()
#include <netinet/in.h>			// gestion IP + ports -> sockaddr_in, htons etc.
#include <sys/socket.h> 		// gestion sockets -> send(), bind(), accept() etc...
#include <netdb.h>				// gestion DNS -> gethostbyname()
#include <fcntl.h>				// fcntl() -> F_SETFL, O_NONBLOCK
#include <unistd.h>				// gestion descripteurs de fichiers -> close()

// === OTHER TOOLS ===
#include <cstring>				// fonctions C de manipulation mémoire -> memset()
#include <cstdlib>				// conversion et gestion mémoire -> atoi()
#include <ctime> 				// gestion temps -> std::time_t, std::tm
#include <csignal>				// gestion signaux -> SIGINT, SIGTSTP