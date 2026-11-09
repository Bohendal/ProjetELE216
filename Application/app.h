/**
 * @file app.h
 * @author Houssam Moussafir
 * @brief Interface du module D (Application CLI) pour l'outil de stéganographie.
 * @version 0.1
 * @date 2026-07-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef APP_H
#define APP_H

/**
 * @brief Encode un message dans une image BMP.
 *
 * Charge l'image d'entrée, cache le contenu du fichier message
 * dans les bits de poids faible (LSB) de l'image, puis sauvegarde
 * l'image résultante.
 *
 * @param img_in Chemin vers l'image BMP d'entrée.
 * @param msg_in Chemin vers le fichier contenant le message à cacher.
 * @param img_out Chemin de l'image BMP de sortie.
 *
 * @return 1 si l'encodage a réussi, 0 en cas d'erreur.
 */
int app_encoder(const char *img_in, const char *msg_in, const char *img_out);

/**
 * @brief Extrait un message caché d'une image BMP.
 *
 * Charge l'image, extrait le message dissimulé dans les bits de
 * poids faible (LSB) et l'écrit dans un fichier.
 *
 * @param img_in Chemin vers l'image BMP contenant le message.
 * @param msg_out Chemin du fichier où enregistrer le message extrait.
 *
 * @return 1 si l'extraction a réussi, 0 sinon.
 */
int app_decoder(const char *img_in, const char *msg_out);

/**
 * @brief Vérifie si une image contient un message caché.
 *
 * Vérifie la présence du nombre magique (magic number)
 * dans les premiers bits de l'image.
 *
 * @param img_in Chemin vers l'image BMP à vérifier.
 *
 * @return 1 si un message est détecté, 0 sinon.
 */
int app_verifier(const char *img_in);

/**
 * @brief Affiche la capacité de dissimulation d'une image.
 *
 * La capacité est calculée à partir du nombre de composantes
 * de couleur disponibles (R, G, B).
 *
 * @param img_in Chemin vers l'image BMP.
 */
void app_capacite(const char *img_in);

/**
 * @brief Compare deux images BMP.
 *
 * Affiche le nombre de composantes de couleur différentes
 * ainsi que le pourcentage de modifications.
 *
 * @param img_a Chemin vers la première image.
 * @param img_b Chemin vers la deuxième image.
 */
void app_comparer(const char *img_a, const char *img_b);

#endif