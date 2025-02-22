# PJ_Final_C
# By Rafael FERNANDO and Romain POLACK

## Context


**This project articulates in different parts of a bigger structure,**
**it is a malware that uses the LD_PRELOAD technique in order to replace a kernel function with a function of our own on a foreign machine.**

### What is an LD_PRELOAD Attack ? 🚨

An LD_PRELOAD attack is a technique used by attackers to hijack dynamically linked libraries in Linux-based systems. It exploits the LD_PRELOAD environment variable, which allows users to specify custom shared libraries to be loaded before any other system libraries.

___
...

**The LD_PRELOAD of this malware focuses on the ssh command, in order to gather the usernname and password of an ssh command.**

**The code is divided into two parts, a server and a client.**

**The server part is located on our machine and the client is supposed to be on the infected machine.**

**The server is equipped with a simple C socket, paired with multithread and port-knocking**

**A multithreaded socket consists of a socket able to receive multiple communications at a t me by splitting them into threads**
**in order to handle them separately.**
**The port knocking feature consists in a sequence of ports that the client has to ping in a specific order for the server**
**to start accepting his messages. If the sequence is not respected, the server won't accept connections.**

**Whenever the server is listening and the client is infecting an other machine, when the machine's user will input an ssh command,**
**the credentials he uses are going to be redirected through the socket onto our machine.**

# Running/Testing 🏃🏻‍➡️

### Main code:

First, in the `./PJ_Final_C` directory run :

```bash
    make
```

Then, you will need to run the program linking the newly compiled library to overwrite the old one.

To do so run :

```bash
LD_PRELOAD=/home/InfectedUserName/PJ_Final_C/lib.so /usr/sbin/sshd -D -ddd -e
```
___

### Server side

For the server side, you can either run it, 
- **Locally** :

To do so ( open an other tab if you already launched the `LD_PRELOAD` ), go to the server directory using :

```bash
    cd socket/server
```

and run :

```bash
    gcc -o server main.c
```
This will create the `server` executable, that you can run like such :

```bash
    ./server
```

- **On a remote server** :

To do this you will just need in the `client.c` to replace on line `12`th line with the address of your server :
[See the line here !](https://github.com/Rafjt/PJ_Final_C/blob/main/socket/client/client.c?plain=1#L12)

And run the same command as locally but on your server. 

> **Now each time someone connects on the infected machine via `ssh` you will receive their credentials as long as `server` will be up and running.**


## How does all of this work

### The linker

Basically, the linker takes all of your objects files (.o files) and link them into an executable file. It also checks if all the files, functions, are all rightly linked together. And the most important part it integrates the necessary functions from libraries (static or dynamic) into the final executable.

### How threads work on Linux OS

Threads in Linux are lightweight processes that share the same memory space within a single process. They allow for concurrent execution of code, enabling multiple tasks to run simultaneously within a program. In the context of this project, using threads in the server allows for handling multiple client connections simultaneously, improving the overall performance and responsiveness of the malware's communication system. This way if the malware was used on multiple infected machines, it would still be able to work properly. 


___

**Details :**
```plaintext
.
├── Makefile
├── BEHINDTHESCENE.md
├── README.md
├── lib.c 
└── socket
    ├── Makefile
    ├── client
    │   ├── client.c
    │   └── client.h
    └── server
        └── main.c
```
- `makefile`:this file contains instructions to compile the project .
- `README.md`: this file explains how the project works and adds detail on it.
- `BEHINDTHESCENE.md`: this file explains how the project was done add details on the process steps.
- `lib.c`: this file contains the main code and logic of the `LD_PRELOAD` attack.
- `client.c/h`: these files contains the main code and logic to send the credentials using port knocking and socket.
- `server/main.c.`: this file contains the main server side code and logic enabling the access to the request it receives using port knocking authentication

