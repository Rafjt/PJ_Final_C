#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define NUM_PORTS 4

// Ports sur lesquels le serveur doit écouter
int ports[NUM_PORTS] = {8080, 21, 2, 2004};

// Structure pour stocker l'état de la séquence de port knocking
typedef struct {
    int step; // Étape actuelle dans la séquence
} KnockState;

// Fonction pour gérer chaque client avec logique de port knocking
void *handle_connection_with_knocking(void *socket_desc) {
    int new_socket = *(int *)socket_desc;
    free(socket_desc);

    // Récupérer le port local sur lequel la connexion a été acceptée
    struct sockaddr_in local_address;
    socklen_t addr_len = sizeof(local_address);
    if (getsockname(new_socket, (struct sockaddr *)&local_address, &addr_len) == -1) {
        perror("getsockname failed");
        close(new_socket);
        return NULL;
    }

    // Obtenir le port local en format hôte
    int port = ntohs(local_address.sin_port);

    // Lire le message envoyé par le client
    char buffer[1024] = {0};
    ssize_t valread = read(new_socket, buffer, sizeof(buffer) - 1);
    if (valread > 0) {
        buffer[valread] = '\0'; // Assure la terminaison nulle

        // Affiche uniquement les messages reçus sur le port 8080
        if (port == 8080) {
            printf("Message received on port %d: %s\n", port, buffer);
        }
    }

    // Logique de port knocking
    static KnockState knock_state = {0}; // État global du port knocking (partagé entre connexions)

    if (knock_state.step == 0 && port == 21) {
        knock_state.step = 1; // Passer à l'étape suivante
    } else if (knock_state.step == 1 && port == 2) {
        knock_state.step = 2; // Passer à l'étape suivante
    } else if (knock_state.step == 2 && port == 2004) {
        knock_state.step = 3; // Passer à l'étape suivante
    } else if (knock_state.step == 3 && port == 8080) {
        printf("Port knocking sequence completed successfully!\n");
        knock_state.step = 0; // Réinitialiser la séquence après succès
    } else {
        knock_state.step = 0; // Réinitialiser la séquence en cas d'erreur
    }

    close(new_socket); // Fermer le socket client
    return NULL;
}

// Fonction pour gérer un socket serveur écoutant sur un port spécifique
void *server_thread(void *arg) {
    int port = *(int *)arg;
    free(arg); // Libère la mémoire allouée pour le port

    int server_fd, *new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Crée le socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        pthread_exit(NULL);
    }

    // Configure les options du socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        close(server_fd);
        pthread_exit(NULL);
    }

    // Configure la structure d'adresse
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Associe le socket au port spécifié
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        pthread_exit(NULL);
    }

    // Met le socket en mode écoute
    if (listen(server_fd, 10) < 0) { // Backlog de 10 connexions en attente max
        perror("Listen failed");
        close(server_fd);
        pthread_exit(NULL);
    }

    printf("Server listening on port %d...\n", port);

    // Boucle pour accepter les connexions entrantes
    while (1) {
        new_socket = malloc(sizeof(int)); // Alloue de la mémoire pour le nouveau socket
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

        pthread_detach(thread); // Nettoie automatiquement les ressources du thread une fois terminé
    }

    close(server_fd); // Ferme le socket serveur (jamais atteint ici)
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_PORTS];

    // Lance un thread serveur pour chaque port
    for (int i = 0; i < NUM_PORTS; i++) {
        int *port = malloc(sizeof(int)); // Alloue dynamiquement la mémoire pour le port
        if (!port) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        *port = ports[i];
        
        if (pthread_create(&threads[i], NULL, server_thread, (void *)port) != 0) {
            perror("Failed to create server thread");
            free(port); // Libère la mémoire si la création du thread échoue
            exit(EXIT_FAILURE);
        }
    }

    // Attend que tous les threads serveurs se terminent (ce qui n'arrive jamais ici)
    for (int i = 0; i < NUM_PORTS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
