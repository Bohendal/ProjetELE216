#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

#include "decodeur.h"
#include "../Image/image_bmp.h"

struct decodeur_s {
    const image_t *img;
    int position;
};

/* =========================================================== */
/* FONCTIONS PRIVÉES                                           */
/* =========================================================== */

static int lire_bit(const decodeur_t *d) {
    
    /*Vérification de la validité du pointeur reçu*/
    assert(d != NULL);

    /*Lecture du LSB à l'index d->position*/
    int bit = image_obtenir_lsb(d->img, d->position);

    return bit;
}

static uint8_t lire_octet(decodeur_t *d) {

    /*Vérification de la validité du pointeur reçu*/
    assert(d != NULL);

    /*Variable qui va contenir l'octet reconstruit*/
    uint8_t octet = 0;

    /*Boucle de lecture des 8 bits consécutifs*/
    for (int i = 0; i < 8; i++) {

        /*Lire un bit via lire_bit(d)*/
        int bit = lire_bit(d);

        /*Décalage de 1 vers la gauche de l'octet*/
        octet = (octet << 1) | bit;

        /*Avancement de la position de 1*/
        d->position += 1;
    }

    return octet;
}

static uint32_t lire_uint32(decodeur_t *d) {

    /*Vérification de la validité du pointeur reçu*/
    assert(d != NULL);

    /*Lecture et assemblages des 4 octets*/
    uint8_t byte0 = lire_octet(d);
    uint8_t byte1 = lire_octet(d);
    uint8_t byte2 = lire_octet(d);
    uint8_t byte3 = lire_octet(d);

    uint32_t valeur = ((uint32_t)byte0 << 24) |
                      ((uint32_t)byte1 << 16) |
                      ((uint32_t)byte2 << 8) |
                      (uint32_t)byte3;

    return valeur;
}

/* =========================================================== */
/* FONCTIONS PUBLIQUES                                         */
/* =========================================================== */

decodeur_t *decodeur_creer(const image_t *img) {

    /*Vérification de la valité de l'image reçue*/
    if(img == NULL) {
        return NULL;
    }

    /*Allocation mémoire de la structure decodeur_t*/
    decodeur_t *d = malloc(sizeof(decodeur_t));
    if(d == NULL) {
        return NULL;
    }

    /*Initialisation des champs de decodeur_t*/
    
    /* 
    Important : on ne libère PAS d->img ici.
    Le décodeur ne fait que stocker l'adresse de l'image reçue en paramètre ;
    il n'en est pas propriétaire et ne l'a jamais allouée lui-même.
    Libérer d->img provoquerait un double free ou un comportement indéfini,
    car la mémoire de l'image doit être détruite par le code qui l'a allouée.
    */
    d->img = img;
    d->position = 0;

    return d;
}

void decodeur_detruire(decodeur_t *d) {

    /*Vérification de la valité du pointeur reçu*/
    if(d == NULL) {
        return;
    }

    /*Libération de la mémoire de la structure decodeur_t*/
    free(d);
}

int decodeur_verifier(const decodeur_t *d) {

    /*Vérification de la valité du pointeur reçu*/
    if(d == NULL) {
        return 0;
    }

    /*Création d'une copie locale du décodeur*/
    decodeur_t copie = *d;

    /*Lecture & assemblage des octets*/
    uint8_t high = lire_octet(&copie);
    uint8_t low = lire_octet(&copie);
    
    uint16_t assemblage = (high << 8) | low; /*Short est un type stockant 16 bits*/

    /*Comparaison avec le magic number*/
    if(assemblage == 0xCAFE) {
        return 1;
    }

    return 0;
}

int decodeur_extraire(decodeur_t *d, unsigned char **donnees, int *taille) {
    
    /*Vérification de la validité des pointeurs*/
    if(d == NULL || donnees == NULL || taille == NULL) {
        return 0;
    }

    /*Vérification de la présence du magic number*/
    if(decodeur_verifier(d) == 0) {
        *donnees = NULL;
        return 0;
    }

    /*Consommation du magic number*/
    lire_octet(d);
    lire_octet(d);

    /*Lecture de la taille du message*/
    uint32_t taille_msg = lire_uint32(d);

    /*Taille nulle ou négative*/
    if(taille_msg < 0) {
        *donnees = NULL;
        return 0;
    }

    /*Taille trop grande pour un int*/
    if(taille_msg > INT_MAX) {
        *donnees = NULL;
        return 0;
    }

    /*Vérification que l'image contient assez de bits*/
    int bits_disponibles = image_capacite_bits(d->img);
    int bits_necessaires = 16 + 32 + 32 + (int)taille_msg * 8;

    if(bits_necessaires > bits_disponibles) {
        *donnees = NULL;
        return 0;
    }

    /*Lecture du checksum*/
    uint32_t checksum_lu = lire_uint32(d);

    /*Allocation du buffer de taille "taille_msg"*/
    uint8_t *buffer = malloc(taille_msg);
    if(buffer == NULL) {
        *donnees = NULL;
        return 0;
    }

    /*Lecture du payload*/
    for(uint32_t i = 0 ; i < taille_msg ; i++) {
        buffer[i] = lire_octet(d);
    }

    /*Recalcul du chekcsum du payload*/
    uint32_t checksum_calc = 0;
    for(uint32_t i = 0 ; i < taille_msg ; i++) {
        checksum_calc += buffer[i];
    }

    /*Comparaison entre le checksum calculé et celui lu*/
    if(checksum_calc != checksum_lu) {
        free(buffer);
        *donnees = NULL;
        return 0;
    }

    *donnees = buffer;
    *taille = (int)taille_msg;
    return 1;
}

int decodeur_extraire_fichier(decodeur_t *d, const char *chemin) {
    
    /*Vérification de la validité des pointeurs*/
    if(d == NULL || chemin == NULL) {
        return 0;
    }

    /*Variables pour recevoir les données extraites*/
    unsigned char *donnees = NULL;
    int taille = 0;

    /*Extraction du message via decodeur_extraire*/
    if(decodeur_extraire(d, &donnees, &taille) == 0) {
        return 0;
    }

    /*Ouverture du fichier en mode binaire*/
    FILE *f = fopen(chemin, "wb");
    if(f == NULL) {
        free(donnees);
        return 0;
    }

    /*Écriture des données dans le fichier*/
    size_t ecrits = fwrite(donnees, 1, taille, f);
    if(ecrits != (size_t)taille) {
        fclose(f);
        free(donnees);
        return 0;
    }

    /*Fermeture du fichier et libération du buffer*/
    fclose(f);
    free(donnees);
    
    return 1;
}

int decodeur_taille_message(const decodeur_t *d) {

    /*Vérification de la validité des pointeurs*/
    if(d == NULL) {
        return 0;
    }

    /*Création d'une copie locale du décodeur*/
    decodeur_t copie = *d;

    /*Vérification du magic number via la copie*/
    if(decodeur_verifier(&copie) == 0) {
        return 0;
    }

    /*Consommation du magic number*/
    lire_octet(&copie);
    lire_octet(&copie);

    /*Lecture de la taille du message*/
    uint32_t taille_msg = lire_uint32(&copie);

    return (int)taille_msg;

}