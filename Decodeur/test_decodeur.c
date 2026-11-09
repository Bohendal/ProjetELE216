/**
 * @file test_decodeur.c
 * @author 
 * @brief 
 * @version 0.1
 * @date 2026-
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <../Image/image_bmp.h>
#include <../Encodeur/encodeur.h>
#include "../unity/unity.h"
#include "decodeur.h"

void setUp(void) {}
void tearDown(void) {}

void test_image_valide() {
    
    /*Création d'une image vide*/
    image_t *img = image_creer(50, 50);

    /*Création d'un encodeur*/
    encodeur_t *e = encodeur_creer(img);
    TEST_ASSERT_NOT_NULL(e);

    /*Message à encoder*/
    const unsigned char msg[] = "HELLO WORLD";
    int taille_msg = sizeof(msg) - 1;

    /*Encodage du message dans l'image*/
    TEST_ASSERT_EQUAL_INT(1, encodeur_cacher(e, msg, taille_msg));

    /*Récupération de l'image encodée*/
    const image_t *img_encodee = encodeur_obtenir_image(e);
    TEST_ASSERT_NOT_NULL(img_encodee);

    /*Création d'un décodeur*/
    decodeur_t *d = decodeur_creer(img_encodee);
    TEST_ASSERT_NOT_NULL(d);

    /*Extraction du message*/
    unsigned char *donnees = NULL;
    int taille = 0;
    TEST_ASSERT_EQUAL_INT(1, decodeur_extraire(d, &donnees, &taille));
    TEST_ASSERT_EQUAL_INT(taille_msg, taille);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg, donnees, taille_msg);

    free(donnees);
    decodeur_detruire(d);
    encodeur_detruire(e);
    image_detruire(img);
}

void test_image_sans_message() {
    
    /*Création d'une image vide*/
    image_t *img = image_creer(50, 50);

    /*Création d'un décodeur*/
    decodeur_t *d = decodeur_creer(img);
    TEST_ASSERT_NOT_NULL(d);

    /*Vérification de l'abscence du magic number*/
    TEST_ASSERT_EQUAL_INT(0, decodeur_verifier(d));

    decodeur_detruire(d);
    image_detruire(img);
}

void test_checksum_corrompu() {

    /*Création d'une image vide*/
    image_t *img = image_creer(50, 50);

    /*Création d'un encodeur*/
    encodeur_t *e = encodeur_creer(img);
    TEST_ASSERT_NOT_NULL(e);

    /*Message à encoder*/
    const unsigned char msg[] = "HELLO";
    int taille_msg = sizeof(msg) - 1;
    TEST_ASSERT_EQUAL_INT(1, encodeur_cacher(e, msg, taille_msg));

    /*Récupération de l'image encodée*/
    const image_t *img_encodee = encodeur_obtenir_image(e);
    TEST_ASSERT_NOT_NULL(img_encodee);

    /*Corruption du checksum : Inversion d'un bit*/
    int bit = image_obtenir_lsb(img_encodee, 48);
    image_set_lsb((image_t*)img_encodee, 48, bit ^ 1);

    /*Création d'un décodeur*/
    decodeur_t *d = decodeur_creer(img_encodee);
    TEST_ASSERT_NOT_NULL(d);

    /*Validation de l'échec de l'extraction*/
    unsigned char *donnees = NULL;
    int taille = 0;
    TEST_ASSERT_EQUAL_INT(0, decodeur_extraire(d, &donnees, &taille));

    decodeur_detruire(d);
    encodeur_detruire(e);
    image_detruire(img);
}

void test_message_taille_zero() {

    /*Création d'une image vide*/
    image_t *img = image_creer(50, 50);

    /*Création d'un encodeur*/
    encodeur_t *e = encodeur_creer(img);
    TEST_ASSERT_NOT_NULL(e);

    /*Message à encoder*/
    const unsigned char msg[] = "";
    int taille_msg = 0;
    TEST_ASSERT_EQUAL_INT(1, encodeur_cacher(e, msg, taille_msg));

    /*Récupération de l'image encodée*/
    const image_t *img_encodee = encodeur_obtenir_image(e);
    TEST_ASSERT_NOT_NULL(img_encodee);

    /*Création d'un décodeur*/
    decodeur_t *d = decodeur_creer(img_encodee);
    TEST_ASSERT_NOT_NULL(d);

    /*Extraction du message vide*/
    unsigned char *donnees = NULL;
    int taille = -1;
    TEST_ASSERT_EQUAL_INT(1, decodeur_extraire(d, &donnees, &taille));

    /*Vérification de la taille*/
    TEST_ASSERT_EQUAL_INT(0, taille);
    TEST_ASSERT_NOT_NULL(donnees);
 
    free(donnees);
    decodeur_detruire(d);
    encodeur_detruire(e);
    image_detruire(img);
}

int main() {

    /*Tests unitaires*/
    UNITY_BEGIN();

    RUN_TEST(test_image_valide);
    RUN_TEST(test_image_sans_message);
    RUN_TEST(test_checksum_corrompu);
    RUN_TEST(test_message_taille_zero);

    return UNITY_END();
}