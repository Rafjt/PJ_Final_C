#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define NUM_PORTS 4

// ports sur lesquels le serveur doit écouter
int ports[NUM_PORTS] = {8080, 21, 2, 2004};

// structure pour stocker l'état de la séquence de port knocking
typedef struct {
    int step; 
} KnockState;

// fonction pour gérer chaque client avec logique de port knocking
void *handle_connection_with_knocking(void *socket_desc) {
    int new_socket = *(int *)socket_desc;
    free(socket_desc);

    // récupérer le port local sur lequel la connexion a été acceptée
    struct sockaddr_in local_address;
    socklen_t addr_len = sizeof(local_address);
    if (getsockname(new_socket, (struct sockaddr *)&local_address, &addr_len) == -1) {
        perror("getsockname failed");
        close(new_socket);
        return NULL;
    }

    int port = ntohs(local_address.sin_port);

    // lire le message envoyé par le client
    char buffer[1024] = {0};
    ssize_t valread = read(new_socket, buffer, sizeof(buffer) - 1);
    if (valread > 0) {
        buffer[valread] = '\0'; // garde fou en cas de message vide

        // affiche uniquement les messages reçus sur le port 8080
        if (port == 8080) {
            printf("Message received on port %d: %s\n", port, buffer);
        }
    }

    // logique de port knocking
    static KnockState knock_state = {0}; 

    if (knock_state.step == 0 && port == 21) {
        knock_state.step = 1; 
    } else if (knock_state.step == 1 && port == 2) {
        knock_state.step = 2; 
    } else if (knock_state.step == 2 && port == 2004) {
        knock_state.step = 3; 
    } else if (knock_state.step == 3 && port == 8080) {
        printf("Port knocking sequence completed successfully!\n");
        knock_state.step = 0; 
    } else {
        knock_state.step = 0; // réinitialiser la séquence en cas d'erreur
    }

    close(new_socket); // fermer le socket client
    return NULL;
}

// fonction gérant la logique de threads
void *server_thread(void *arg) {
    int port = *(int *)arg;
    free(arg); // libère la mémoire allouée pour le port

    int server_fd, *new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // création du socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        pthread_exit(NULL);
    }

    // configuration des options du socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        close(server_fd);
        pthread_exit(NULL);
    }

    // configuration des structures d'adresses
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // bind le socket au port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        pthread_exit(NULL);
    }

    // lance le mode écoute sur le port
    if (listen(server_fd, 10) < 0) { 
        perror("Listen failed");
        close(server_fd);
        pthread_exit(NULL);
    }

    printf("Server listening on port %d...\n", port);

    // tant que le programme tourne, la boucle créera de nouveaux socket
    while (1) {
        new_socket = malloc(sizeof(int)); 
        if (!new_socket) {
            perror("Memory allocation failed");
            continue;
        }

        *new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (*new_socket < 0) {
            perror("Accept failed");
            free(new_socket);
            continue;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_connection_with_knocking, (void *)new_socket) != 0) {
            perror("Failed to create thread");
            free(new_socket);
            continue;
        }

        pthread_detach(thread); // nettoyage automatique du thread lorsque ce dernier s'arrête
    }

  //  close(server_fd);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_PORTS];

    // lance un thread pour chaque port
    for (int i = 0; i < NUM_PORTS; i++) {
        int *port = malloc(sizeof(int)); 
        if (!port) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        *port = ports[i];
        
        if (pthread_create(&threads[i], NULL, server_thread, (void *)port) != 0) {
            perror("Failed to create server thread");
            free(port); 
            exit(EXIT_FAILURE);
        }
    }


    for (int i = 0; i < NUM_PORTS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
