/**
 * @file stegano.c
 * @author  Houssam Moussafir
 * @brief Programme principal phase 1 & 2
 * @version 0.2
 * @date 2026-0
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./Application/app.h"

#define BACKLOG 1
#define BUFFER_SIZE 1024

int fichier_existe(const char *chemin) {
    FILE *fichier = fopen(chemin, "r");
    if (fichier) {
        fclose(fichier);
        return 1;
    }
    return 0;
}

// --- LOGIQUE RESEAU ENCAPSULEE POUR ENVOI ---
int executer_send(const char* ip, int port, const char* img_orig, const char* msg_txt) {
    int socket_fd;
    struct sockaddr_in server_addr;
    struct in_addr addr_binary;
    const char* img_modifiee = "tmp_stego_encoded.bmp";

    app_encoder(img_orig, msg_txt, img_modifiee);

    if (inet_pton(AF_INET, ip, &addr_binary) <= 0) {
        fprintf(stderr, "[-] Erreur: Adresse IP invalide\n");
        return 1;
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) { perror("[-] Erreur socket()"); return 1; }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr = addr_binary;
    server_addr.sin_port = htons(port);

    printf("[+] Connexion au serveur %s:%d...\n", ip, port);
    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("[-] Erreur connect()");
        close(socket_fd);
        return 1;
    }
    printf("[+] Connecté au serveur !\n");

    FILE *file = fopen(img_modifiee, "rb");
    if (!file) { perror("[-] Erreur image"); close(socket_fd); return 1; }
    fseek(file, 0, SEEK_END);
    uint32_t file_size = (uint32_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    printf("[+] Envoi de la taille du fichier : %u octets\n", file_size);
    uint32_t net_file_size = htonl(file_size);
    write(socket_fd, &net_file_size, sizeof(net_file_size));

    printf("[+] Envoi des données en cours...\n");
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        write(socket_fd, buffer, bytes_read);
    }

    printf("[+] Envoi réussi et terminé !\n");
    fclose(file);
    close(socket_fd);
    unlink(img_modifiee); 
    return 0;
}

// --- LOGIQUE RESEAU ENCAPSULEE POUR RECEPTION ---
int executer_receive(int port) {
    int socket_fd, accept_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    const char* img_recue = "output_received.bmp";
    const char* msg_extrait = "recovered_secret.txt";

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) { perror("[-] Erreur socket()"); return 1; }

    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("[-] Erreur bind() (Port probablement déjà utilisé)");
        close(socket_fd);
        return 1;
    }

    if (listen(socket_fd, BACKLOG) == -1) {
        perror("[-] Erreur listen()");
        close(socket_fd);
        return 1;
    }

    printf("[+] Écoute active sur le port %d. En attente de connexion...\n", port);
    accept_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);
    if (accept_fd == -1) { perror("[-] Erreur accept()"); close(socket_fd); return 1; }
    printf("[+] Émetteur connecté depuis %s\n", inet_ntoa(client_addr.sin_addr));

    uint32_t net_file_size;
    read(accept_fd, &net_file_size, sizeof(net_file_size));
    uint32_t file_size = ntohl(net_file_size);
    printf("[+] Taille de fichier à recevoir : %u octets\n", file_size);

    FILE *file = fopen(img_recue, "wb");
    if (!file) { perror("[-] Erreur fopen wb"); close(accept_fd); close(socket_fd); return 1; }
    
    char buffer[BUFFER_SIZE];
    uint32_t total_received = 0;
    int bytes_read;

    printf("[+] Réception des octets binaires...\n");
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

    printf("[+] Image binaire reçue entièrement (%u octets).\n", total_received);
    printf("[+] Décodage automatique...\n");
    app_decoder(img_recue, msg_extrait);
    return 0;
}

void afficher_aide(void) {
    printf("Usage:\n");
    printf("  ./stegano capacity <photo.bmp>\n");
    printf("  ./stegano encode <photo.bmp> <secret.txt> <photo_steg.bmp>\n");
    printf("  ./stegano verify <photo_steg.bmp>\n");
    printf("  ./stegano compare <photo.bmp> <photo_steg.bmp>\n");
    printf("  ./stegano decode <photo_steg.bmp> <recovered.txt>\n");
    printf("  ./stegano send <adresse> <port> <image.bmp> <message.txt>\n");
    printf("  ./stegano receive <port>\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        afficher_aide();
        return 0; 
    }

    if (strcmp(argv[1], "capacity") == 0) {
        if (argc != 3) return 1;
        if (!fichier_existe(argv[2])) return 1;
        app_capacite(argv[2]);
    }
    else if (strcmp(argv[1], "encode") == 0) {
        if (argc != 5) return 1;
        if (!fichier_existe(argv[2]) || !fichier_existe(argv[3])) return 1;
        app_encoder(argv[2], argv[3], argv[4]);
    }
    else if (strcmp(argv[1], "verify") == 0) {
        if (argc != 3) return 1;
        if (!fichier_existe(argv[2])) return 1;
        app_verifier(argv[2]);
    }
    else if (strcmp(argv[1], "compare") == 0) {
        if (argc != 4) return 1;
        if (!fichier_existe(argv[2]) || !fichier_existe(argv[3])) return 1;
        app_comparer(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "decode") == 0) {
        if (argc != 4) return 1;
        if (!fichier_existe(argv[2])) return 1;
        app_decoder(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "send") == 0) {
        if (argc != 6) return 1;
        if (!fichier_existe(argv[4]) || !fichier_existe(argv[5])) return 1;
        return executer_send(argv[2], atoi(argv[3]), argv[4], argv[5]);
    }
    else if (strcmp(argv[1], "receive") == 0) {
        if (argc != 3) return 1;
        return executer_receive(atoi(argv[2]));
    }
    else {
        afficher_aide();
        return 1;
    }
    return 0;
}
