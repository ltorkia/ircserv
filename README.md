# Projet IRC Server 42: [SUJET FR](https://github.com/ltorkia/ircserv/blob/main/fr.subject.pdf)

Ce projet consiste à développer un serveur IRC en C++ 98, conforme à la norme et capable de gérer plusieurs clients simultanément.

---

## **Description du projet**

L'objectif est de développer un serveur IRC fonctionnant en TCP/IP, capable de :
- Gérer plusieurs clients simultanément sans jamais bloquer.
- Fournir les fonctionnalités de base d'un serveur IRC (authentification, messages privés, channels, etc.).
- Être utilisé avec un client IRC de référence.

**Exécution du programme :**
```bash
./ircserv <port> <password>
```
- `port` : Numéro de port pour les connexions entrantes.
- `password` : Mot de passe pour s'authentifier sur le serveur.

---

## **Prérequis**

### Contraintes techniques :
- Norme C++ 98 uniquement.
- Pas de forking.
- Opérations d'entrée/sortie non bloquantes.
- Un seul appel à `poll()` ou équivalent. Ici nous avons choisi `select()`.

### Fonctions externes autorisées :
- `socket`, `close`, `setsockopt`, `bind`, `connect`, `listen`, `accept`...
- Voir la liste complète dans la [consigne]((https://github.com/ltorkia/ircserv/blob/main/fr.subject.pdf)).

---

## **Fonctionnalités obligatoires**

### Fonctionnalités de base :
1. Authentification avec mot de passe.
2. Gestion de plusieurs clients simultanément.
3. Commandes IRC essentielles :
   - Connexion (nickname, username).
   - Gestion des channels :
     - Rejoindre un channel.
     - Envoyer et recevoir des messages dans un channel.
   - Commandes opérateurs :
     - `KICK` : Éjecter un client.
     - `INVITE` : Inviter un client.
     - `TOPIC` : Modifier/afficher le sujet.
     - `MODE` : Gérer les modes du channel (`i`, `t`, `k`, `o`, `l`).
4. Communication conforme au protocole TCP/IP.

### Spécificités :
Utiliser `fcntl()` uniquement pour les fichiers non bloquants :
```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```
---

## **Structure des fichiers**

### Organisation :
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
    ├── main.cpp
    └── utils
        ├── IrcHelper.cpp
        ├── MessageHandler.cpp
        └── Utils.cpp
```
---

## **Gestion du projet**

1. **Gestion non bloquante :**
   - Toutes les opérations d'entrée/sortie sont non bloquantes.

2. **Client IRC de référence :**
   - `irssi`

3. **Gestion des erreurs :**
   - Connexions interrompues, données partielles, etc.
   - Utilisation de blocs `try/catch` pour capturer les exceptions et éviter les crashs.

4. **Respect de la norme C++ 98 :**
   - Pas de nouvelles fonctionnalités C++ 11 ou supérieures.
   - Pas de bibliothèque non autorisée.

---

## **Exemples de tests**

### Test basique avec `nc` :
```bash
nc 127.0.0.1 6667
com^Dman^Dd
```
- Envoyez une commande fragmentée pour tester la reconstitution des paquets.

### Test avec un client IRC :
1. Connection au serveur :
   ```bash
   /server 127.0.0.1 6667
   ```
2. Authentification avec mot de passe.
3. Tester les commandes :
   - `NICK`, `USER`, `JOIN`, `PRIVMSG`, `KICK`, etc.

---

## **Partie bonus gérée**

### Fonctionnalités supplémentaires possibles :
1. **Envoi de fichiers** : Permet aux clients de s'envoyer des fichiers via le protocole DCC.
2. **Bot IRC** : Fournit sur demande des `!funfact`, l'heure `!time`, et notre `!age` au jour près.
---
