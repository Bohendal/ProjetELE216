#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./Application/app.h"

#define BACKLOG 1
#define BUFFER_SIZE 1024
#define TYPE_TEXTE 0
#define TYPE_IMAGE 1

/* ------------------------------------------------------------------------- */
/* Utilitaires                                                               */
/* ------------------------------------------------------------------------- */

int fichier_existe(const char *chemin)
{
    FILE *fichier = fopen(chemin, "rb");
    if (fichier == NULL)
        return 0;

    fclose(fichier);
    return 1;
}

static int prochain_nom_message(char *chemin, size_t capacite)
{
    int numero = 0;
    int longueur;

    do {
        if (numero == 0) {
            longueur = snprintf(chemin, capacite,
                "ImageBMP/recovered_secret.txt");
        } else {
            longueur = snprintf(chemin, capacite,
                "ImageBMP/recovered_secret%d.txt", numero);
        }

        if (longueur < 0 || (size_t)longueur >= capacite)
            return -1;

        if (access(chemin, F_OK) != 0)
            return 0;

        numero++;

    } while (numero > 0);

    return -1;
}

/* Envoie exactement taille octets, même si send() effectue un envoi partiel. */
int envoyer_tout(int socket_fd, const void *donnees, size_t taille)
{
    const char *p = (const char *)donnees;
    size_t total = 0;

    while (total < taille) {
        ssize_t n = send(socket_fd, p + total, taille - total, 0);
        if (n <= 0)
            return -1;
        total += (size_t)n;
    }

    return 0;
}

/* Reçoit exactement taille octets, même si recv() reçoit des blocs partiels. */
int recevoir_tout(int socket_fd, void *donnees, size_t taille)
{
    char *p = (char *)donnees;
    size_t total = 0;

    while (total < taille) {
        ssize_t n = recv(socket_fd, p + total, taille - total, 0);
        if (n <= 0)
            return -1;
        total += (size_t)n;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
/* Transfert autonome historique : ./stegano send IP PORT image.bmp msg.txt  */
/* Le format reste compatible avec executer_receive() : taille + fichier.   */
/* ------------------------------------------------------------------------- */

static int envoyer_fichier_brut(int socket_fd, const char *chemin)
{
    FILE *file = fopen(chemin, "rb");
    if (file == NULL) {
        perror("[-] Erreur ouverture image");
        return 1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 1;
    }

    long taille_longue = ftell(file);
    if (taille_longue <= 0 || (uint64_t)taille_longue > UINT32_MAX) {
        fprintf(stderr, "[-] Taille de fichier invalide.\n");
        fclose(file);
        return 1;
    }
    rewind(file);

    uint32_t taille_reseau = htonl((uint32_t)taille_longue);
    if (envoyer_tout(socket_fd, &taille_reseau, sizeof(taille_reseau)) < 0) {
        fclose(file);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    size_t lus;

    while ((lus = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (envoyer_tout(socket_fd, buffer, lus) < 0) {
            fclose(file);
            return 1;
        }
    }

    if (ferror(file)) {
        fclose(file);
        return 1;
    }

    fclose(file);
    return 0;
}

/*
 * Version utilisée par la commande autonome "send".
 * Elle conserve le protocole historique attendu par "receive".
 */
static int executer_send_autonome(const char *ip, int port,
                                  const char *img_orig,
                                  const char *msg_txt)
{
    int socket_fd;
    struct sockaddr_in server_addr;
    const char *img_modifiee = "tmp_stego_encoded.bmp";

    if (!fichier_existe(img_orig) || !fichier_existe(msg_txt)) {
        fprintf(stderr, "[-] Image ou fichier texte introuvable.\n");
        return 1;
    }

    app_encoder(img_orig, msg_txt, img_modifiee);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("[-] Erreur socket()");
        unlink(img_modifiee);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "[-] Adresse IP invalide.\n");
        close(socket_fd);
        unlink(img_modifiee);
        return 1;
    }

    printf("[+] Connexion au serveur %s:%d...\n", ip, port);
    if (connect(socket_fd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("[-] Erreur connect()");
        close(socket_fd);
        unlink(img_modifiee);
        return 1;
    }

    int resultat = envoyer_fichier_brut(socket_fd, img_modifiee);
    if (resultat == 0)
        printf("[+] Envoi réussi et terminé !\n");
    else
        fprintf(stderr, "[-] Erreur pendant l'envoi.\n");

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    unlink(img_modifiee);
    return resultat;
}

/* ------------------------------------------------------------------------- */
/* Réception autonome historique : ./stegano receive PORT                   */
/* ------------------------------------------------------------------------- */

int executer_receive(int port)
{
    int socket_fd, accept_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    const char *img_recue = "ImageBMP/output_received.bmp";
    char msg_extrait[256];

    if (prochain_nom_message(msg_extrait, sizeof(msg_extrait)) < 0) {
        fprintf(stderr, "Erreur génération nom fichier.\n");
        return -1;
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("[-] Erreur socket()");
        return 1;
    }

    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((uint16_t)port);

    if (bind(socket_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("[-] Erreur bind()");
        close(socket_fd);
        return 1;
    }

    if (listen(socket_fd, BACKLOG) < 0) {
        perror("[-] Erreur listen()");
        close(socket_fd);
        return 1;
    }

    printf("[+] Écoute active sur le port %d...\n", port);
    accept_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);
    if (accept_fd < 0) {
        perror("[-] Erreur accept()");
        close(socket_fd);
        return 1;
    }

    printf("[+] Émetteur connecté depuis %s\n", inet_ntoa(client_addr.sin_addr));

    uint32_t taille_reseau;
    if (recevoir_tout(accept_fd, &taille_reseau, sizeof(taille_reseau)) < 0) {
        fprintf(stderr, "[-] Réception de la taille impossible.\n");
        close(accept_fd);
        close(socket_fd);
        return 1;
    }

    uint32_t taille = ntohl(taille_reseau);
    if (taille == 0) {
        fprintf(stderr, "[-] Fichier vide ou taille invalide.\n");
        close(accept_fd);
        close(socket_fd);
        return 1;
    }

    FILE *file = fopen(img_recue, "wb");
    if (file == NULL) {
        perror("[-] Erreur ouverture image reçue");
        close(accept_fd);
        close(socket_fd);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    uint32_t total = 0;
    int erreur = 0;

    while (total < taille) {
        size_t a_lire = (taille - total < BUFFER_SIZE)
                      ? (size_t)(taille - total) : BUFFER_SIZE;

        if (recevoir_tout(accept_fd, buffer, a_lire) < 0) {
            erreur = 1;
            break;
        }

        if (fwrite(buffer, 1, a_lire, file) != a_lire) {
            erreur = 1;
            break;
        }
        total += (uint32_t)a_lire;
    }

    fclose(file);
    close(accept_fd);
    close(socket_fd);

    if (erreur) {
        fprintf(stderr, "[-] Réception incomplète.\n");
        return 1;
    }

    printf("[+] Image reçue (%u octets).\n", total);
    printf("[+] Décodage automatique...\n");
    app_decoder(img_recue, msg_extrait);
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Envoi de messages texte dans le chat                                      */
/* ------------------------------------------------------------------------- */

int envoyer_message(int socket_fd, const char *message, size_t taille)
{
    uint8_t type = TYPE_TEXTE;
    uint32_t taille_reseau;

    if (taille > UINT32_MAX)
        return -1;

    taille_reseau = htonl((uint32_t)taille);

    if (envoyer_tout(socket_fd, &type, sizeof(type)) < 0)
        return -1;
    if (envoyer_tout(socket_fd, &taille_reseau, sizeof(taille_reseau)) < 0)
        return -1;

    return envoyer_tout(socket_fd, message, taille);
}

/* ------------------------------------------------------------------------- */
/* ENVOI STEGANO DANS LE CHAT                                                 */
/* On réutilise executer_send() avec le socket déjà établi par le chat.       */
/* ------------------------------------------------------------------------- */

int executer_send(int socket_fd, const char *img_orig, const char *msg_txt)
{
    const char *img_modifiee = "tmp_stego_encoded.bmp";

    if (!fichier_existe(img_orig)) {
        fprintf(stderr, "[-] Image introuvable : %s\n", img_orig);
        return 1;
    }
    if (!fichier_existe(msg_txt)) {
        fprintf(stderr, "[-] Fichier message introuvable : %s\n", msg_txt);
        return 1;
    }

    /* Encodage du texte dans l'image. */
    app_encoder(img_orig, msg_txt, img_modifiee);

    FILE *file = fopen(img_modifiee, "rb");
    if (file == NULL) {
        perror("[-] Erreur ouverture image encodée");
        unlink(img_modifiee);
        return 1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        unlink(img_modifiee);
        return 1;
    }

    long taille_longue = ftell(file);
    if (taille_longue <= 0 || (uint64_t)taille_longue > UINT32_MAX) {
        fprintf(stderr, "[-] Taille de l'image encodée invalide.\n");
        fclose(file);
        unlink(img_modifiee);
        return 1;
    }
    rewind(file);

    /* En-tête du protocole chat : type IMAGE, puis taille, puis octets. */
    uint8_t type = TYPE_IMAGE;
    uint32_t taille_reseau = htonl((uint32_t)taille_longue);

    if (envoyer_tout(socket_fd, &type, sizeof(type)) < 0 ||
        envoyer_tout(socket_fd, &taille_reseau, sizeof(taille_reseau)) < 0) {
        fprintf(stderr, "[-] Erreur envoi de l'en-tête image.\n");
        fclose(file);
        unlink(img_modifiee);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    size_t lus;
    int erreur = 0;

    while ((lus = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (envoyer_tout(socket_fd, buffer, lus) < 0) {
            erreur = 1;
            break;
        }
    }

    if (ferror(file))
        erreur = 1;

    fclose(file);
    unlink(img_modifiee);

    if (erreur) {
        fprintf(stderr, "[-] Erreur pendant le transfert de l'image.\n");
        return 1;
    }

    printf("[+] Image stéganographiée envoyée !\n");
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Réception d'un paquet chat : texte ou image                                */
/* ------------------------------------------------------------------------- */

int recevoir_paquet_chat(int socket_fd)
{
    uint8_t type;
    uint32_t taille_reseau;

    if (recevoir_tout(socket_fd, &type, sizeof(type)) < 0)
        return -1;

    if (recevoir_tout(socket_fd, &taille_reseau, sizeof(taille_reseau)) < 0)
        return -1;

    uint32_t taille = ntohl(taille_reseau);
    if (taille == 0) {
        fprintf(stderr, "[-] Paquet vide ou taille invalide.\n");
        return -1;
    }

    if (type == TYPE_TEXTE) {
        char message[BUFFER_SIZE];

        if (taille >= sizeof(message)) {
            fprintf(stderr, "[-] Message texte trop long.\n");
            return -1;
        }

        if (recevoir_tout(socket_fd, message, taille) < 0)
            return -1;

        message[taille] = '\0';
        printf("\rPartenaire : %s\n", message);
        return 0;
    }

    if (type == TYPE_IMAGE) {
        const char *img_recue = "ImageBMP/output_received.bmp";
        char msg_extrait[256];

        if (prochain_nom_message(msg_extrait, sizeof(msg_extrait)) < 0) {
            fprintf(stderr, "Erreur génération nom fichier.\n");
            return -1;
        }

        FILE *file = fopen(img_recue, "wb");
        if (file == NULL) {
            perror("[-] Erreur création image reçue");
            return -1;
        }

        char buffer[BUFFER_SIZE];
        uint32_t total = 0;
        int erreur = 0;

        while (total < taille) {
            size_t a_lire = (taille - total < BUFFER_SIZE)
                          ? (size_t)(taille - total) : BUFFER_SIZE;

            if (recevoir_tout(socket_fd, buffer, a_lire) < 0) {
                erreur = 1;
                break;
            }

            if (fwrite(buffer, 1, a_lire, file) != a_lire) {
                erreur = 1;
                break;
            }

            total += (uint32_t)a_lire;
        }

        fclose(file);

        if (erreur) {
            fprintf(stderr, "[-] Réception de l'image incomplète.\n");
            return -1;
        }

        printf("\r[+] Image stéganographiée reçue (%u octets).\n", total);
        printf("[+] Décodage automatique...\n");
        app_decoder(img_recue, msg_extrait);
        printf("[+] Message récupéré dans : %s\n", msg_extrait);
        return 0;
    }

    fprintf(stderr, "[-] Type de paquet inconnu : %u\n", type);
    return -1;
}

/* ------------------------------------------------------------------------- */
/* MODE CHAT INTERACTIF                                                       */
/* ------------------------------------------------------------------------- */

void executer_chat(int socket_fd)
{
    fd_set lecteurs;
    char buffer[1024];
    int terminer = 0;

    printf("\n========== CHAT CONNECTE ==========\n");
    printf("Tapez vos messages puis Entree.\n");
    printf("Commande : stegano envoyer <image.bmp> <message.txt>\n");
    printf("Tapez /quit pour quitter.\n\n");

    while (!terminer) {
        printf("Vous : ");
        fflush(stdout);

        FD_ZERO(&lecteurs);
        FD_SET(STDIN_FILENO, &lecteurs);
        FD_SET(socket_fd, &lecteurs);

        int max_fd = (socket_fd > STDIN_FILENO) ? socket_fd : STDIN_FILENO;

        int resultat = select(max_fd + 1, &lecteurs, NULL, NULL, NULL);
        if (resultat < 0) {
            perror("Erreur select()");
            break;
        }

        /* Réception réseau. */
        if (FD_ISSET(socket_fd, &lecteurs)) {
            printf("\r");
            fflush(stdout);

            if (recevoir_paquet_chat(socket_fd) < 0) {
                printf("\n[!] Partenaire déconnecté ou erreur de réception.\n");
                break;
            }
        }

        /* Lecture clavier et envoi. */
        if (FD_ISSET(STDIN_FILENO, &lecteurs)) {
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                terminer = 1;
                continue;
            }

            buffer[strcspn(buffer, "\r\n")] = '\0';

            if (strcmp(buffer, "/quit") == 0) {
                terminer = 1;
                continue;
            }

            /* Commande : stegano envoyer image.bmp message.txt */
            if (strncmp(buffer, "stegano envoyer ", 16) == 0) {
                char image[256];
                char message[256];
                char extra;

                int nb = sscanf(buffer + 16, "%255s %255s %c",
                                image, message, &extra);

                if (nb != 2) {
                    printf("Usage : stegano envoyer <image.bmp> <message.txt>\n");
                    continue;
                }

                if (executer_send(socket_fd, image, message) != 0)
                    printf("[-] Échec de l'envoi stéganographique.\n");

                continue;
            }

            if (buffer[0] != '\0') {
                if (envoyer_message(socket_fd, buffer, strlen(buffer)) < 0) {
                    perror("Erreur envoi");
                    terminer = 1;
                }
            }
        }
    }

    shutdown(socket_fd, SHUT_RDWR);
    printf("\n========== CHAT TERMINE ==========\n");
}

/* ------------------------------------------------------------------------- */
/* Serveur chat                                                               */
/* ------------------------------------------------------------------------- */

int executer_chat_server(int port)
{
    int socket_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Erreur socket()");
        return 1;
    }

    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((uint16_t)port);

    if (bind(socket_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("Erreur bind()");
        close(socket_fd);
        return 1;
    }

    if (listen(socket_fd, BACKLOG) < 0) {
        perror("Erreur listen()");
        close(socket_fd);
        return 1;
    }

    printf("[+] Serveur en attente sur le port %d...\n", port);

    client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("Erreur accept()");
        close(socket_fd);
        return 1;
    }

    printf("[+] Client connecté : %s\n", inet_ntoa(client_addr.sin_addr));
    executer_chat(client_fd);

    close(client_fd);
    close(socket_fd);
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Client chat                                                                */
/* ------------------------------------------------------------------------- */

int executer_chat_client(const char *ip, int port)
{
    int socket_fd;
    struct sockaddr_in server_addr;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Erreur socket()");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Adresse IP invalide.\n");
        close(socket_fd);
        return 1;
    }

    printf("[+] Connexion au serveur %s:%d...\n", ip, port);

    if (connect(socket_fd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("Erreur connect()");
        close(socket_fd);
        return 1;
    }

    printf("[+] Connexion établie !\n");
    executer_chat(socket_fd);
    close(socket_fd);
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Aide et main                                                               */
/* ------------------------------------------------------------------------- */

void afficher_aide(void)
{
    printf("Usage:\n");
    printf("  ./stegano capacity <photo.bmp>\n");
    printf("  ./stegano encode <photo.bmp> <secret.txt> <photo_steg.bmp>\n");
    printf("  ./stegano verify <photo_steg.bmp>\n");
    printf("  ./stegano compare <photo.bmp> <photo_steg.bmp>\n");
    printf("  ./stegano decode <photo_steg.bmp> <recovered.txt>\n");
    printf("  ./stegano send <adresse> <port> <image.bmp> <message.txt>\n");
    printf("  ./stegano receive <port>\n");
    printf("  ./stegano chat-server <port>\n");
    printf("  ./stegano chat-client <adresse_ip> <port>\n");
    printf("\nDans le chat :\n");
    printf("  stegano envoyer <image.bmp> <message.txt>\n");
    printf("  /quit\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        afficher_aide();
        return 0;
    }

    if (strcmp(argv[1], "capacity") == 0) {
        if (argc != 3 || !fichier_existe(argv[2]))
            return 1;
        app_capacite(argv[2]);
    }
    else if (strcmp(argv[1], "encode") == 0) {
        if (argc != 5 || !fichier_existe(argv[2]) ||
            !fichier_existe(argv[3]))
            return 1;
        app_encoder(argv[2], argv[3], argv[4]);
    }
    else if (strcmp(argv[1], "verify") == 0) {
        if (argc != 3 || !fichier_existe(argv[2]))
            return 1;
        app_verifier(argv[2]);
    }
    else if (strcmp(argv[1], "compare") == 0) {
        if (argc != 4 || !fichier_existe(argv[2]) ||
            !fichier_existe(argv[3]))
            return 1;
        app_comparer(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "decode") == 0) {
        if (argc != 4 || !fichier_existe(argv[2]))
            return 1;
        app_decoder(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "send") == 0) {
        if (argc != 6)
            return 1;
        return executer_send_autonome(argv[2], atoi(argv[3]),
                                      argv[4], argv[5]);
    }
    else if (strcmp(argv[1], "receive") == 0) {
        if (argc != 3)
            return 1;
        return executer_receive(atoi(argv[2]));
    }
    else if (strcmp(argv[1], "chat-server") == 0) {
        if (argc != 3)
            return 1;
        return executer_chat_server(atoi(argv[2]));
    }
    else if (strcmp(argv[1], "chat-client") == 0) {
        if (argc != 4)
            return 1;
        return executer_chat_client(argv[2], atoi(argv[3]));
    }
    else {
        afficher_aide();
        return 1;
    }

    return 0;
}
