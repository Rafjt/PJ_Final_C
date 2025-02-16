# PJ_Final_C

This project articulates in different parts of a bigger structure, 
it is a malware that uses the LD_PRELOAD technique in order to replace a kernel function with a function of our own on a foreign machine.

The LD_PRELOAD of this malware focuses on the ssh command, in order to gather the usernname and password of an ssh command. 

The code is divided into two parts, a server and a client. 

The server part is located on our machine and the client is supposed to be on the infected machine. 

The server is equipped with a simple C socket, paired with multithread and port-knocking

A multithreaded socket consists of a socket able to receive multiple communications at a time by splitting them into threads
in order to handle them separately. 
The port knocking feature consists in a sequence of ports that the client has to ping in a specific order for the server
to start accepting his messages. If the sequence is not respected, the server won't accept connections.

Whenever the server is listening and the client is infecting an other machine, when the machine's user will input an ssh command, 
the credentials he uses are going to be redirected through the socket onto our machine. 
