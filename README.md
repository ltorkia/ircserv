# Projet IRC Server ---> [SUJET FR](https://github.com/ltorkia/ft_irc/blob/lee/fr.subject.pdf)

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
- Un seul appel à `poll()` ou équivalent (select, epoll, kqueue).

### Fonctions externes autorisées :
- `socket`, `close`, `setsockopt`, `bind`, `connect`, `listen`, `accept`...
- Voir la liste complète dans la [consigne](https://github.com/ltorkia/ft_irc/blob/lee/fr.subject.pdf).

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

### Spécificités MacOS :
Pour MacOS, utilisez `fcntl()` pour les fichiers non bloquants :
```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```
---

## **Structure des fichiers**

### Exemple d'organisation :
```
project_root/
├── Makefile
├── srcs/
│   ├── main.cpp
│   ├── server.cpp
│   └── ...
├── incs/
│   ├── server.hpp
│   └── ...
└── config/
    └── ircserv.conf (optionnel)
```
---

## **Difficultés et points à surveiller**

1. **Gestion non bloquante :**
   - Toutes les opérations d'entrée/sortie doivent être non bloquantes.
   - Assurez-vous que `poll()` ou équivalent est correctement utilisé pour surveiller les descripteurs de fichier.

2. **Tests avec un client IRC de référence :**
   - Choisissez un client (par ex. `WeeChat`, `irssi`, ou `mIRC`).
   - Testez chaque fonctionnalité obligatoire avec ce client.

3. **Gestion des erreurs :**
   - Vérifiez toutes les erreurs possibles : connexions interrompues, données partielles, etc.
   - Utilisez un `try/catch` pour capturer les exceptions et éviter les crashs.

4. **Respect de la norme C++ 98 :**
   - Pas de nouvelles fonctionnalités C++ 11 ou supérieures.
   - Évitez les bibliothèques non autorisées.

5. **Qualité du code :**
   - Structure claire et modulaire.
   - Documentation dans le code pour chaque classe et fonction.

---

## **Exemples de tests**

### Test basique avec `nc` :
```bash
nc 127.0.0.1 6667
com^Dman^Dd
```
- Envoyez une commande fragmentée pour tester la reconstitution des paquets.

### Test avec un client IRC :
1. Connectez-vous au serveur :
   ```bash
   /server 127.0.0.1 6667
   ```
2. Authentifiez-vous avec le mot de passe.
3. Testez les commandes :
   - `NICK`, `USER`, `JOIN`, `PRIVMSG`, `KICK`, etc.

---

## **Partie bonus**

### Fonctionnalités supplémentaires possibles :
1. **Envoi de fichiers** : Permettre aux clients de s'envoyer des fichiers.
2. **Bot IRC** : Ajouter un bot automatisé pour répondre aux messages ou fournir des services.

---
