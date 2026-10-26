#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct image image_t;

/**
 * @brief Charge un fichier BMP et convertie le fichier sous forme d'image_t.
 * 
 * @param chemin pointeur vers le fichier BMP qu'on souhaite charger.
 * @return image_t* pointeur vers l'image convertie, NULL en cas d'échec d'allocation mémoire
 * ou si la convertion d'image BMP à image_t n'a pas fonctionné.
 */
image_t *image_charger(const char *chemin);

/**
 * @brief Crée une image vide (noire) d'une grandeur donnée.
 * 
 * @param largeur de l'image en pixel (int), doit être positive et non-nulle.
 * @param hauteur de l'image en pixel (int), doit être positive et non-nulle.
 * @return image_t* pointeur vers l'image créée, NULL si:
 *      -la hauteur en paramètre est négative ou nulle
 *      -la largeur en paramètre est négative ou nulle
 *      -échec d'allocation mémoire
 */
image_t *image_creer(int largeur, int hauteur);


/**
 * @brief Fait une copie profonde de l'image_t en paramètre. Elle copie la hauteur et la largeur et crée
 * un nouveau vecteur de pixel pour faire la copie profonde.
 * 
 * @param src pointeur vers l'image_t qu'on souhaite copier.
 * @return image_t* pointeur vers l'image copiée, NULL en cas d'échec d'allocation mémoire.
 */
image_t *image_copier(const image_t *src);

/**
 * @brief Sauvegarde une image_t dans un fichier BMP.
 * 
 * @param img image_t qu'on souhaite sauvegarder.
 * @param chemin pointeur vers le fichier BMP qui va contenir l'image_t.
 * 
 *      Retourne NULL en cas d'échec d'allocation mémoire.
 */
void image_sauvegarder(const image_t *img, const char *chemin);

/**
 * @brief Libère la mémoire d'une image_t
 * 
 * @param img pointeur vers l'image_t qu'on souhaite libérer.
 * 
 *      Retourne NULL si l'image_t fournit est NULL.
 */
void image_detruire(image_t *img);

/**
 * @brief Donne l'accès à la largeur de l'image_t
 * 
 * @param img pointeur vers l'image_t
 * @return int la largeur de l'image_t
 */
int image_largeur(const image_t *img);

/**
 * @brief Donne l'accès à la hauteur de l'image_t
 * 
 * @param img pointeur vers l'image_t
 * @return int la hauteur de l'image_t
 */
int image_hauteur(const image_t *img);


/**
 * @brief Retourne la valeur entre 0 et 255 d'une composante donnée du pixel 
 * à la position (x,y). Chaque pixel de l'image a 3 composantes, soit Rouge, Vert et Bleu.
 * Voici quelle valeur représente chaque composante:
 *          -Composante = 0 ----> Rouge
 *          -Composante = 1 ----> Vert
 *          -Composante = 2 ----> Bleu
 * 
 * @param img pointeur vers l'image_t
 * @param x position horizontal du pixel, doit être plus petite que la largeur de l'image
 *          ainsi que plus grande ou égal à 0.
 * @param y position vertical du pixel, doit être plus petite que la hauteur de l'image
 *          ainsi que plus grande ou égal à 0.
 * @param comp valeur la composante qu'on souhaite obtenir, ne peut pas être une autre valeur
 *             que 0, 1 ou 2.
 *             
 * @return unsigned char valeur de la composante voulue (valeur entre 0 et 255)
 */
unsigned char image_obtenir_composante(const image_t *img, int x, int y, int comp);

/**
 * @brief Modifie la veleur d'une composante donnée du pixel à la position (x,y). 
 * Chaque pixel de l'image a 3 composantes, soit Rouge, Vert et Bleu.
 * Voici quelle valeur représente chaque composante:
 *          -Composante = 0 ----> Rouge
 *          -Composante = 1 ----> Vert
 *          -Composante = 2 ----> Bleu
 * 
 * @param img pointeur vers l'image_t
 * @param x position horizontal du pixel, doit être plus petite que la largeur de l'image
 *          ainsi que plus grande ou égal à 0.
 * @param y position vertical du pixel, doit être plus petite que la hauteur de l'image
 *          ainsi que plus grande ou égal à 0.
 * @param comp valeur la composante qu'on souhaite obtenir, ne peut pas être une autre valeur
 *             que 0, 1 ou 2.
 * @param val valeur entre 0 et 255 qu'on souhaite assigner à la composante donnée.
 */
void image_set_composante(image_t *img, int x, int y, int comp, unsigned char val);


/**
 * @brief Permet d'obtenir le LSB (le bit de poids faible) à un index linéaire donné.
 * L'index linéaire représente l'index des composantes de chaque pixel. Donc, par exemple,
 * l'index 1 indique la composante Verte du premier pixel et l'index 3 indique la composante 
 * Rouge du deuxième pixel.
 * 
 * @param img pointeur vers l'image_t.
 * @param index index linéaire de la composante qu'on souhaite obtenir le LSB. L'index doit
 *              être plus grand ou égal à 0 et plus petit que la largeur de l'image * la hauteur 
 *              de l'image * 3.
 * @return int le LSB de la composante à l'index donné, retourne -1 si la composante (donc l'index donné)
 *         n'est pas valide. 
 */
int image_obtenir_lsb(const image_t *img, int index);


/**
 * @brief Permet de modifier le LSB (le bit de poids faible) à un index linéaire donné.
 * L'index linéaire représente l'index des composantes de chaque pixel. Donc, par exemple,
 * l'index 1 indique la composante Verte du premier pixel et l'index 3 indique la composante 
 * Rouge du deuxième pixel. Les 7 bits supérieurs au LSB ne sont pas changés.
 * 
 * @param img pointeur vers l'image_t
 * @param index index linéaire de la composante qu'on souhaite obtenir le LSB. L'index doit
 *              être plus grand ou égal à 0 et plus petit que la largeur de l'image * la hauteur 
 *              de l'image * 3.
 * @param bit valeur, soit 0 ou 1, qu'on souhaite assigner au LSB à l'index donné.
 */
void image_set_lsb(image_t *img, int index, int bit);


/**
 * @brief Retourne le nombre total de bits disponibles de l'image_t, soit
 *        la largeur de l'image * la hauteur de l'image * 3.
 * 
 * @param img pointeur vers l'image_t
 * @return int le nombre de bits disponibles.
 */
int image_capacite_bits(const image_t *img);

/**
 * @brief Retourne le nombre de composantes qui sont différentes à une même position
 *        entre deux images données.
 * 
 * @param a pointeur vers la première image_t qu'on souhaite comparer.
 * @param b pointeur vers la deuxième image_t qu'on souhaite comparer.
 * @return int le nombre de composantes qui sont différentes entre les deux images.
 */
int image_comparer(const image_t *a, const image_t *b);
