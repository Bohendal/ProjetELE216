/**
 * @file server.c
 * @brief Serveur TCP simple pour la réception de stéganographie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./Application/app.h"

#define BACKLOG 1
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int socket_fd, accept_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    const char* img_recue = "output_received.bmp";
    const char* msg_extrait = "recovered_secret.txt";

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);

    // 1. Création du socket de streaming IPv4/TCP
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("[-] Erreur socket()");
        return EXIT_FAILURE;
    }

    // Autoriser la réutilisation immédiate du port après fermeture
    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Liaison locale
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("[-] Erreur bind()");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    // 3. Passage en écoute passive
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("[-] Erreur listen()");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    printf("[+] Serveur en attente sur le port %d...\n", port);

    // 4. Acceptation de l'émetteur
    accept_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);
    if (accept_fd == -1) {
        perror("[-] Erreur accept()");
        close(socket_fd);
        return EXIT_FAILURE;
    }
    printf("[+] Émetteur connecté : %s\n", inet_ntoa(client_addr.sin_addr));

    // 5. Lecture de la taille du fichier (Obligatoire pour le protocole réseau !)
    uint32_t net_file_size;
    if (read(accept_fd, &net_file_size, sizeof(net_file_size)) != sizeof(net_file_size)) {
        fprintf(stderr, "[-] Erreur de lecture de la taille attendue\n");
        close(accept_fd);
        close(socket_fd);
        return EXIT_FAILURE;
    }
    uint32_t file_size = ntohl(net_file_size);
    printf("[+] Taille annoncée par le client : %u octets\n", file_size);

    // 6. Réception de l'image par morceaux en mode binaire ("wb")
    FILE *file = fopen(img_recue, "wb");
    if (!file) {
        perror("[-] Erreur création image");
        close(accept_fd);
        close(socket_fd);
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    uint32_t total_received = 0;
    int bytes_read;

    while (total_received < file_size) {
        uint32_t to_read = (file_size - total_received < BUFFER_SIZE) ? (file_size - total_received) : BUFFER_SIZE;
        bytes_read = read(accept_fd, buffer, to_read);
        if (bytes_read <= 0) break;
        
        fwrite(buffer, 1, bytes_read, file);
        total_received += bytes_read;
    }

    fclose(file);
    close(accept_fd);
    close(socket_fd);
    printf("[+] Image binaire stockée avec succès.\n");

    // 7. Décodage automatique du message caché (Phase 1)
    printf("[+] Extraction automatique du texte secret...\n");
    app_decoder(img_recue, msg_extrait);

    return EXIT_SUCCESS;
}
