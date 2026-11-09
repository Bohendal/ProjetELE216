/**
 * @file encodeur.c
 * @author 
 * @brief Implémentation du module d'encodage pour la stéganographie.
 *        Ce fichier contient la logique interne pour manipuler les structures
 *        opaques d'encodage et injecter les bits du protocole dans les composantes
 *        de pixels de manière séquentielle.
 * @version 0.1
 * @date 2026-
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../Image/image_bmp.h"
#include "encodeur.h"


/**
 * @brief Structure interne de l'encodeur contenant l'état de la stéganographie.
 * 
 */
struct encodeur {
    image_t *img_out;     // Pointeur vers la copie de l'image de sortie modifiée.
    int pos_courant;      // Index du bit linéaire courant dans l'image.
};


encodeur_t *encodeur_creer(const image_t *img) {
    if (! img) { return NULL; }

    encodeur_t *e = malloc(sizeof(encodeur_t));
    if (! e) { return NULL; }
    
    e->img_out = image_copier(img);
    if (! e->img_out) {
        free(e);
        return NULL;
    }
    e->pos_courant = 0;

    return e;
}

void encodeur_detruire(encodeur_t *e) {
    if (! e) { return; }
    image_detruire(e->img_out);
    free(e);
}

/**
 * @brief Cache un octet dans l'image en insérant du MSB jusqu'au LSB.
 *        Fonction utilitaire interne (static). Elle prend chaque bit de l'octet,
 *        l'injecte dans le LSB du pixel ciblé, puis incrémente pos_courant.
 * 
 * @param e Pointeur vers l'encodeur.
 * @param octet Valeur sur 8 bits à dissimuler.
 */
static void cacher_un_octet(encodeur_t *e, uint8_t octet) {
    for (int b = 7; b >= 0; b--) {
        int bit = (octet >> b) & 1;
        image_set_lsb(e->img_out, e->pos_courant++, bit);
    }
}

int encodeur_cacher(encodeur_t *e, const unsigned char *donnees, int taille) {
    if (! e || ! donnees || taille < 0) { return 0; }

    int bit_requis = (10 + taille) * 8;
    if (bit_requis > image_capacite_bits(e->img_out)) {
        return 0;
    }
    
    e->pos_courant = 0;   // Au cas où on veut réécrire un message sur le même encodeur

    // Magic
    uint16_t magic = 0xCAFE;
    cacher_un_octet(e, (uint8_t)(magic >> 8));
    cacher_un_octet(e, (uint8_t)(magic));

    // Taille
    for (int i = 3; i >= 0; i--) {
        cacher_un_octet(e, (uint8_t)(taille >> (i * 8)));
    }

    // Checksum
    uint32_t checksum = 0;
    for (int i = 0; i < taille; i++) {
        checksum += donnees[i];
    }
    // Checksum
    for (int i = 3; i >= 0; i--) {
        cacher_un_octet(e, (uint8_t)(checksum >> (i * 8)));
    }

    // Payload
    for (int i = 0; i < taille; i++) {
        cacher_un_octet(e, (uint8_t)donnees[i]);
    }
    
    return 1;
}

int encodeur_cacher_fichier(encodeur_t *e, const char *chemin) {
    if (!e || !chemin) { return 0; }

    FILE *f = fopen(chemin, "rb");
    if (! f) { return 0; }

    // Mesure la taille du fichier (nombre d'éléments)
    fseek(f, 0, SEEK_END);
    long taille = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *buffer = malloc(taille);
    if (!buffer) {
        fclose(f);
        return 0;
    }
    
    size_t lecture = fread(buffer, 1, taille, f);
    
    int statut = 0;
    if (lecture == (size_t)taille) {
        statut = encodeur_cacher(e, buffer, (int)taille);
    }
    
    fclose(f);
    free(buffer);
    return statut;
}

const image_t *encodeur_obtenir_image(const encodeur_t *e) {
    if (! e) { return NULL; }
    return e->img_out;
}

int encodeur_bits_utilises(const encodeur_t *e) {
    if (! e) { return 0; }
    return e->pos_courant;
}
