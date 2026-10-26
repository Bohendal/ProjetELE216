#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "vecteur_gen.h"

// type opaque medelisant une image au foramt PPM P3
typedef struct image_ppm image_ppm_t;

typedef struct image_bmp image_bmp_t;


/**
 * Crée une nouvelle image PPM vide.
 * L'image est initialisée avec le format P3, valeur maximale 255,
 * et un tableau dynamique pour stocker les pixels.
 * @return Pointeur vers l'image créée ou NULL en cas d'échec d'allocation mémoire.
 */
image_ppm_t *image_ppm_creer();

/**
 * Libère la mémoire allouée pour une image PPM.
 * @param img Pointeur vers l'image à libérer. Si le pointeur est NULL,
 *            la fonction n'a pas d'effet (voir la définition de free()).
 */
void image_ppm_liberer(image_ppm_t *img);

/**
 * Obtient la largeur de l'image PPM en pixels.
 * @param img Pointeur non-nul vers l'image.
 * @return La largeur de l'image en pixels.
 */
int32_t image_ppm_largeur(image_ppm_t *img);

/**
 * Obtient la hauteur de l'image PPM en pixels.
 * @param img Pointeur non-nul vers l'image.
 * @return La hauteur de l'image en pixels.
 */
int32_t image_ppm_hauteur(image_ppm_t *img);

/**
 * Obtient le tableau dynamique contenant les pixels de l'image.
 * @param img Pointeur non-nul vers l'image.
 * @return Pointeur vers le tableau dynamique des pixels.
 */
vecteur_t *image_ppm_pixels(image_ppm_t *img);

/**
 * Crée une image PPM à partir d'un fichier PPM format P3.
 * Le fichier doit respecter le format PPM standard :
 * - Ligne 1 : "P3"
 * - Ligne 2 : largeur hauteur
 * - Ligne 3 : valeur maximale (doit être 255)
 * - Lignes suivantes : triplets R G B pour chaque pixel
 * 
 * @param nom_fichier Nom du fichier PPM à charger.
 * @return Pointeur vers l'image créée ou NULL en cas d'échec
 *         (fichier inexistant, format invalide, erreur mémoire).
 */
image_ppm_t *image_ppm_du_fichier_ppm(char *nom_fichier);

/**
 * Vérifie si l'image PPM est valide.
 * Une image est considérée comme valide si elle a des dimensions non-nulles
 * et contient le nombre correct de pixels (largeur × hauteur).
 * @param img Pointeur non-nul vers l'image.
 * @return true si l'image est valide, false sinon.
 */
bool image_ppm_est_valide(image_ppm_t *img);

image_ppm_t *image_ppm_de_image_bmp(image_bmp_t *source);

void image_ppm_sauvegarde_fichier(image_ppm_t* img, char *destination) ;