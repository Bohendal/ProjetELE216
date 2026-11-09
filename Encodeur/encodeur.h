/**
 * @file encodeur.h
 * @author 
 * @brief Spécification du module d'encodage pour la stéganographie.
 *        Ce module fournit les fonctions nécessaires pour dissimuler un
 *        message binaire dans les bits de poids faible (LSB) d'une image BMP.
 * @version 0.1
 * @date 2026-
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "image_bmp.h"

/**
 * @brief Structure opaque représentant le moteur d'encodage stéganographique.
 * 
 */
typedef struct encodeur encodeur_t;

/**
 * @brief Crée et initialise l'encodeur.
 *        Effectue une copie profonde de l'image fournie (l'encodeur travaille sur une copie).
 * 
 * @param img Pointeur vers l'image source où la copie profonde sera créé.
 * @return pointeur vers l'encodeur alloué, ou NULL en cas d'échec ou si img est NULL.
 */
encodeur_t *encodeur_creer(const image_t *img);

/**
 * @brief Libère la mémoire associée à l'encodeur.
 *        Cette fonction détruit aussi l'image interne.
 * 
 * @param e Pointeur vers l'encodeur à détruire. Si e est NULL, l'action est ignorée.
 */
void encodeur_detruire(encodeur_t *e);

/**
 * @brief Dissimule un message en mémoire dans les bits LSB de l'image.
 *        Applique le protocole d'en-tête (Magic Number 0xCAFE, Taille du payload, Checksum du message) 
 *        suivi des données du message (Payload) en insérant le bit de poids fort en premier.
 * 
 * @param e Pointeur vers l'encodeur.
 * @param donnees Pointeur vers le tableau d'octets à dissimuler.
 * @param taille Nombre d'octets du message à cacher (payload).
 * @return 1 si l'encodage réussit, sinon 0 si le message est trop gros ou paramètres invalides.
 */
int encodeur_cacher(encodeur_t *e, const unsigned char *donnees, int taille);

/**
 * @brief Lit un fichier en binaire pour dissimuler son contenu dans l'image.
 *        Ouvre le fichier, calcul sa taille, charge ses données et appelle
 *        encodeur_cacher. Gère le cas d'un fichier vide.
 * 
 * @param e Pointeur vers l'encodeur.
 * @param chemin Chaîne de caractères indiquant l'emplacement du fichier à lire.
 * @return 1 si la lecture et l'insertion réussissent, sinon
 *         0 si les paramètres sont invalides, si le fichier n'ouvre pas ou l'allocation du buffer échoue.
 */
int encodeur_cacher_fichier(encodeur_t *e, const char *chemin);

/**
 * @brief Accesseur permettant de récupérer l'image modifiée avec le message caché.
 * 
 * @param e Pointeur constant vers l'encodeur.
 * @return pointeur constant vers la structure de l'image modifiée, sinon NULL si e est NULL.
 */
const image_t *encodeur_obtenir_image(const encodeur_t *e);

/**
 * @brief Retourne le nombre total de bits écrits par l'encodeur.
 * 
 * @param e Pointeur constant vers l'encodeur.
 * @return le nombre de bits utilisés dans l'image ou 0 si e est NULL.
 */
int encodeur_bits_utilises(const encodeur_t *e);

