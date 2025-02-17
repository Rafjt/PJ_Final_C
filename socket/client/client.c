#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "client.h"
#define PORT 8080


#define MESSAGE "Knock"

#define SERVER_ADDRESS "127.0.0.1"

// fonction pour envoyer un message à un port spécifique (fait partie du port knocking)
int send_to_port(int port, const char *message) {
    int status, client_fd;
    struct sockaddr_in serv_addr;

    // création du socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error for port %d\n", port);
        return -1;
    }

    // sonfiguration de l'adresse du serveur
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // conversion de l'adresse IP en format binaire
    if (inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/Address not supported for port %d\n", port);
        close(client_fd);
        return -1;
    }

    // connexion au socket
    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection failed for port %d\n", port);
        close(client_fd);
        return -1;
    }

    // envoi d'un message
    send(client_fd, message, strlen(message), 0);
    printf("Message sent to port %d: %s\n", port, message);

    //fermeture du socket
    close(client_fd);
    return 0;
}

// fonction principale pour effectuer le port knocking
int knock_knock() {
    int ports[] = {21, 2, 2004};
    int num_ports = sizeof(ports) / sizeof(ports[0]);

    for (int i = 0; i < num_ports; i++) {
        if (send_to_port(ports[i], MESSAGE) != 0) { // itère à travers les ports pour toquer sur chacun d'eux
            printf("Failed to knock on port %d\n", ports[i]);
            return -1; // Retourne une erreur si un knock échoue
        }
        sleep(1); // Pause optionnelle entre les knocks pour éviter d'envoyer trop rapidement
    }

    printf("Port knocking sequence completed successfully.\n");
    return 0;
}

// fonction servant à envoyer les credentials, similaire à celle du port knocking
// mais contenant un réel message
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

    if (inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0) {
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

    valread = read(client_fd, buffer, 1024 - 1); 
    buffer[valread] = '\0'; 
    printf("Server response: %s\n", buffer);

    // fermeture de la connexion socket
    close(client_fd);
    return 0;
}

// fonction main point d'entrée du programme
int main()
{
    const char *message = "a";
    return send_credentials_client(message);
}