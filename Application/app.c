/**
 * @file app.c
 * @author Houssam Moussafir
 * @brief Implémentation des fonctions du module D (Application CLI)
 * @version 0.1
 * @date 2026-07-15
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include "app.h"
#include "../Image/image_bmp.h"
#include "../Encodeur/encodeur.h"
#include "../Decodeur/decodeur.h"

int app_encoder(const char *img_in, const char *msg_in, const char *img_out)
{
    image_t *img = image_charger(img_in);

    if (img == NULL)
    {
        printf("[-] Impossible de charger l'image\n");
        return 0;
    }

    int dispo_bits = image_capacite_bits(img);

    encodeur_t *enc = encodeur_creer(img);

    if (enc == NULL)
    {
        image_detruire(img);
        return 0;
    }

    if (!encodeur_cacher_fichier(enc, msg_in))
    {
        printf("[-] Message trop gros ou fichier invalide\n");
        encodeur_detruire(enc);
        image_detruire(img);
        return 0;
    }

    image_sauvegarder(encodeur_obtenir_image(enc), img_out);

    printf("[+] Message encode avec succes (%d bits dans %d disponibles)\n",
           encodeur_bits_utilises(enc), dispo_bits);
    printf("[+] Image sauvegardee : %s\n", img_out);

    encodeur_detruire(enc);
    image_detruire(img);

    return 1;
}

int app_decoder(const char *img_in, const char *msg_out)
{
    image_t *img = image_charger(img_in);

    if (img == NULL)
    {
        printf("[-] Impossible de charger l'image\n");
        return 0;
    }

    decodeur_t *dec = decodeur_creer(img);

    if (dec == NULL)
    {
        image_detruire(img);
        return 0;
    }

    if (!decodeur_verifier(dec)) {
        printf("[-] Aucun message detecte dans cette image\n");
        decodeur_detruire(dec);
        image_detruire(img);
        return 0;
    }

    int taille_octets = decodeur_taille_message(dec);

    if (!decodeur_extraire_fichier(dec, msg_out))
    {
        printf("[-] Aucun message detecte dans cette image\n");
        decodeur_detruire(dec);
        image_detruire(img);
        return 0;
    }

    printf("[+] Magic number verifie\n");
    printf("[+] Message extrait : %d octets, checksum OK\n", taille_octets);
    printf("[+] Fichier ecrit : %s\n", msg_out);

    decodeur_detruire(dec);
    image_detruire(img);

    return 1;
}

int app_verifier(const char *img_in)
{
    image_t *img = image_charger(img_in);

    if (img == NULL)
    {
        printf("[-] Impossible de charger l'image\n");
        return 0;
    }

    decodeur_t *dec = decodeur_creer(img);

    int resultat = decodeur_verifier(dec);

    if (resultat) {
        printf("[+] Magic number verifie\n");
    } 
    else {
        printf("[-] Aucun message detecte dans cette image\n");
    }

    decodeur_detruire(dec);
    image_detruire(img);

    return resultat;
}

void app_capacite(const char *img_in)
{
    image_t *img = image_charger(img_in);

    if (img == NULL)
    {
        printf("[-] Impossible de charger l'image\n");
        return;
    }

    int bits = image_capacite_bits(img);

    printf("[+] Capacite maximale : %d octets\n", bits / 8);

    image_detruire(img);
}

void app_comparer(const char *img_a, const char *img_b)
{
    image_t *a = image_charger(img_a);
    image_t *b = image_charger(img_b);

    if (a == NULL || b == NULL)
    {
        printf("[-] Impossible de charger les images\n");

        if (a) image_detruire(a);
        if (b) image_detruire(b);

        return;
    }

    int diff = image_comparer(a, b);
    int total = image_capacite_bits(a);

    printf("[+] Nombre de composantes modifiees : %d\n", diff);
    printf("[+] Pourcentage de modification : %.2f %%\n", 100.0 * diff / total);

    image_detruire(a);
    image_detruire(b);
}
