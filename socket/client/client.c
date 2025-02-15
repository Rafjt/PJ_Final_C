// Client side C program to demonstrate Socket programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "client.h"
#define PORT 8080


#define MESSAGE "Knock"

// Fonction pour envoyer un message à un port spécifique
int send_to_port(int port, const char *message) {
    int status, client_fd;
    struct sockaddr_in serv_addr;

    // Création du socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error for port %d\n", port);
        return -1;
    }

    // Configuration de l'adresse du serveur
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Conversion de l'adresse IP en format binaire
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/Address not supported for port %d\n", port);
        close(client_fd);
        return -1;
    }

    // Connexion au serveur
    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection failed for port %d\n", port);
        close(client_fd);
        return -1;
    }

    // Envoi du message
    send(client_fd, message, strlen(message), 0);
    printf("Message sent to port %d: %s\n", port, message);

    // Fermeture du socket
    close(client_fd);
    return 0;
}

// Fonction principale pour effectuer le "port knocking"
int knock_knock() {
    int ports[] = {21, 2, 2004}; // Liste des ports à contacter
    int num_ports = sizeof(ports) / sizeof(ports[0]);

    for (int i = 0; i < num_ports; i++) {
        if (send_to_port(ports[i], MESSAGE) != 0) {
            printf("Failed to knock on port %d\n", ports[i]);
            return -1; // Retourne une erreur si un knock échoue
        }
        sleep(1); // Pause optionnelle entre les knocks pour éviter d'envoyer trop rapidement
    }

    printf("Port knocking sequence completed successfully.\n");
    return 0;
}


int send_credentials_client(const char *message)
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/Address not supported\n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed\n");
        return -1;
    }

    knock_knock();
    send(client_fd, message, strlen(message), 0);
    printf("Message sent: %s\n", message);

    valread = read(client_fd, buffer, 1024 - 1); // subtract 1 for the null terminator at the end
    buffer[valread] = '\0'; // Ensure null termination
    printf("Server response: %s\n", buffer);

    // Closing the connected socket
    close(client_fd);
    return 0;
}

// Main function to call send_credentials_client
//int main()
//{
//    const char *message = "Hello from client";
//    return send_credentials_client(message);
//}
//
