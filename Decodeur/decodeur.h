#pragma once

#include "image_bmp.h"

typedef struct decodeur_s decodeur_t;

/**
 * @brief Fonction d'initialisation du décodeur associé à une image
 * 
 * Vérifie que l'image n'est pas NULL, alloue une structure decodeur_t,
 * initialise la position de lecture à 0 et stocke l'image reçue.
 * 
 * @param img Image reçue (ne doit pas être NULL).
 * @return decodeur_t* Pointeur vers le décodeur initialisé, ou NULL si erreur.
 */
decodeur_t *decodeur_creer(const image_t *img);

/**
 * @brief Fonction de destruction du décodeur.
 * 
 * Vérifie que le pointeur reçu n'est pas NULL, libère la structure decodeur_t
 * sans détruire l'image associée (qui appartient à l'appelant).
 *
 * @param d Pointeur vers le décodeur à détruire.
 */
void decodeur_detruire(decodeur_t *d);

/**
 * @brief Fonction de de vérification de la présence du magic number
 * dans les premiers LSB.
 * 
 * Lit les deux premiers octets cachés via une copie locale du décodeur,
 * reconstruit le magic number et le compare à la valeur attendue (0xCAFE).
 * Ne modifie jamais la position du décodeur réel.
 *
 * @param d Décodeur initialisé (ne doit pas être NULL).
 * @return int 1 si le magic est présent, 0 sinon.
 */
int decodeur_verifier(const decodeur_t *d);

/**
 * @brief Fonction d'extraction du message encodé dans l'image du décodeur.
 *
 * Lit le header (magic, taille, checksum), extrait le payload et
 * vérifie son intégrité. En cas de succès, alloue un buffer contenant
 * les données extraites et retourne 1. En cas d'échec, retourne 0.
 *
 * @param d 
 *      Décodeur valide. Ne doit pas être NULL.
 * 
 * @param donnees
 *      Adresse d'un pointeur où sera stocké le buffer alloué.
 *      En cas d'échec, *donnees est mis à NULL.
 * 
 * @param taille
 *      Adresse d'un entier où sera stockée la taille du payload.
 *      Non modifié en cas d'échec.
 * 
 * @return int 
 *      1 si extraction réussie (magic valide, checksum valide).
 *      0 si un paramètre est NULL, si le magic est absent,
 *      si l'allocation échoue ou si le checksum est invalide.
 */
int decodeur_extraire(decodeur_t *d, unsigned char **donnees, int *taille);

/**
 * @brief Extrait le message encodé dans l'image et l'écrit dans un fichier.
 * 
 * Cette fonction utilise decodeur_extraire() pour récupérer le message
 * dissimulé dans l'image associée au décodeur. Si l'extraction réussit,
 * le message est écrit tel quel dans le fichier spécifié par @p chemin.
 *
 * Le fichier est ouvert en mode binaire ("wb") afin de permettre l'écriture
 * de données arbitraires (texte ou contenu non textuel). En cas d'erreur
 * d'extraction ou d'écriture, aucun fichier valide n'est garanti.
 * 
 * @param d Pointeur vers le décodeur initialisé.
 * @param chemin Chemin du fichier dans lequel écrire le message extrait.
 * 
 * @return int 
 *         1 si le message a été extrait et écrit avec succès,
 *         0 en cas d'erreur (décodeur invalide, extraction impossible,
 *         ouverture du fichier échouée, écriture incomplète, etc.).
 */
int decodeur_extraire_fichier(decodeur_t *d, const char *chemin);

/**
 * @brief Retourne la taille du message encodé dans l'image.
 *
 * Cette fonction lit uniquement la taille du message présente dans le header
 * du flux binaire, après avoir vérifié la présence du magic number. Elle ne
 * modifie pas la position interne du décodeur, car la lecture est effectuée
 * sur une copie locale de la structure.
 * 
 * @param d Pointeur vers le décodeur initialisé.
 * @return int 
 *         La taille du message si le header est valide,
 *         0 en cas d'erreur (pointeur invalide, magic absent, etc.).
 */
int decodeur_taille_message(const decodeur_t *d);