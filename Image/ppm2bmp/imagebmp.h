#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "vecteur_gen.h"

typedef struct image_ppm image_ppm_t;

typedef struct image_bmp image_bmp_t;

/**
 * Crée une nouvelle image BMP vide.
 * L'image est initialisée avec les en-têtes BMP et DIB standards
 * pour un format 32 bits par pixel (ARGB).
 * @return Pointeur vers l'image créée ou NULL en cas d'échec d'allocation mémoire.
 */
image_bmp_t *image_bmp_creer();

/**
 * Libère la mémoire allouée pour une image BMP.
 * @param img Pointeur vers l'image à libérer. Si le pointeur est NULL,
 *            la fonction n'a pas d'effet (voir la définition de free()).
 */
void image_bmp_liberer(image_bmp_t *img);

/**
 * Crée une image BMP à partir d'une image PPM source.
 * Cette fonction réalise une copie superficielle du tableau de pixels
 * de la source. Si la source ou son contenu est détruite, cette référence
 * ne sera plus valide. Pour plus de robustesse, une copie profonde
 * serait nécessaire.
 * 
 * @param source Pointeur non-nul vers l'image PPM source.
 * @return Pointeur vers l'image BMP créée ou NULL en cas d'échec d'allocation mémoire.
 */
image_bmp_t *image_bmp_de_image_ppm(image_ppm_t *source);

/**
 * Sauvegarde une image BMP dans un fichier.
 * Le fichier est créé au format BMP standard avec :
 * - En-tête BMP (14 octets)
 * - En-tête DIB (40 octets) 
 * - Padding d'alignement (2 octets)
 * - Données des pixels au format ARGB 32 bits
 * Les lignes sont stockées de bas en haut selon la convention BMP.
 * 
 * @param img Pointeur non-nul vers l'image BMP.
 * @param destination Nom du fichier de destination.
 */
void image_bmp_sauvegarde_fichier(image_bmp_t* img, const char *destination);

int32_t image_bmp_largeur(image_bmp_t *img);

int32_t image_bmp_hauteur(image_bmp_t *img);

image_bmp_t *image_bmp_du_fichier_bmp(const char *nom_fichier);

vecteur_t *image_bmp_pixels(image_bmp_t *img);

bool image_bmp_est_valide(image_bmp_t *img);

image_bmp_t *changer_valeur_bmp(image_bmp_t *img,int32_t hauteur,int32_t largeur,int32_t taille_f,vecteur_t *pixels);