/**
 * @file client.c
 * @brief Client TCP simple pour l'encodage et l'envoi de stéganographie
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

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int socket_fd;
    struct sockaddr_in server_addr;
    struct in_addr addr_binary;
    const char* img_modifiee = "tmp_stego_encoded.bmp";

    if (argc != 5) {
        fprintf(stderr, "Usage: %s <adresse_ip> <port> <image.bmp> <message.txt>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    char *img_orig = argv[3];
    char *msg_txt = argv[4];

    // 1. Encodage local stéganographique (Phase 1)
    printf("[+] Encodage local du message secrêt...\n");
    app_encoder(img_orig, msg_txt, img_modifiee);

    // 2. Initialisation réseau
    if (inet_pton(AF_INET, ip, &addr_binary) <= 0) {
        fprintf(stderr, "[-] Erreur: Adresse IP invalide\n");
        return EXIT_FAILURE;
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("[-] Erreur socket()");
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr = addr_binary;
    server_addr.sin_port = htons(port);

    // 3. Connexion
    printf("[+] Connexion à la machine distante %s:%d...\n", ip, port);
    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("[-] Erreur connect()");
        close(socket_fd);
        return EXIT_FAILURE;
    }

    // 4. Calcul précis de la taille de l'image modifiée
    FILE *file = fopen(img_modifiee, "rb");
    if (!file) {
        perror("[-] Erreur lecture image temporaire");
        close(socket_fd);
        return EXIT_FAILURE;
    }
    fseek(file, 0, SEEK_END);
    uint32_t file_size = (uint32_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    // 5. Envoi de la taille (Format Big-Endian réseau)
    uint32_t net_file_size = htonl(file_size);
    if (write(socket_fd, &net_file_size, sizeof(net_file_size)) != sizeof(net_file_size)) {
        fprintf(stderr, "[-] Erreur transmission taille\n");
        fclose(file);
        close(socket_fd);
        return EXIT_FAILURE;
    }

    // 6. Envoi des octets du fichier par blocs
    printf("[+] Envoi du flux binaire en cours...\n");
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        write(socket_fd, buffer, bytes_read);
    }

    printf("[+] Envoi complété !\n");
    fclose(file);
    close(socket_fd);
    unlink(img_modifiee); 
    return EXIT_SUCCESS;
}
