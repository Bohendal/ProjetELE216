#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "unity.h"
#include "encodeur.h"
#include "decodeur.h"
#include "image_bmp.h"

static image_t *img = NULL;
static encodeur_t *e = NULL;

void setUp(void) {
    img = image_creer(10, 10);
    e = encodeur_creer(img);

    TEST_ASSERT_NOT_NULL(img);
    TEST_ASSERT_NOT_NULL(e);
}

void tearDown(void) {
    encodeur_detruire(e);
    image_detruire(img);
}

// Encoder un message connu : le magic number est aux bons lsb des premiers pixels
void test_encoder_message_connu(void) {
    unsigned char message[] = "allo";
    int statut = encodeur_cacher(e, message, strlen((char*)message));
    TEST_ASSERT_EQUAL_INT(1, statut);

    const image_t *img_modifiee = encodeur_obtenir_image(e);

    uint8_t octet1 = 0;
    for (int b = 7; b >= 0; b--) {
        int bit = image_obtenir_lsb(img_modifiee, 7 - b);
        octet1 = (bit << b) | octet1;
    }

    uint8_t octet2 = 0;
    for (int b = 7; b >= 0; b--) {
        int bit = image_obtenir_lsb(img_modifiee, 15 - b);
        octet2 = (bit << b) | octet2;
    }

    uint16_t magic_extrait = (octet1 << 8) | octet2;

    TEST_ASSERT_EQUAL_HEX16(0xCAFE, magic_extrait);
}

// Encoder puis décoder avec le module C : le message est retrouvé intégralement
void test_encoder_puis_decoder(void) {
    unsigned char message[] = "Message secret!";
    int taille = strlen((char *)message);

    int statut_e = encodeur_cacher(e, message, taille);
    TEST_ASSERT_EQUAL_INT(1, statut_e);

    const image_t *img_modifiee = encodeur_obtenir_image(e);

    decodeur_t *d = decodeur_creer(img_modifiee); 
    TEST_ASSERT_NOT_NULL(d);

    unsigned char *message_recupere = NULL; 
    int taille_extraite = 0;                

    int statut_d = decodeur_extraire(d, &message_recupere, &taille_extraite);

    TEST_ASSERT_EQUAL_INT(1, statut_d);
    TEST_ASSERT_EQUAL_INT(taille, taille_extraite);
    TEST_ASSERT_NOT_NULL(message_recupere);
    TEST_ASSERT_EQUAL_MEMORY(message, message_recupere, taille);

    free(message_recupere); 
    decodeur_detruire(d); 
}

// Message trop gros pour l’image : retourne 0
void test_message_trop_gros(void) {
    image_t *img1 = image_creer(1, 1);
    TEST_ASSERT_NOT_NULL(img1);
    encodeur_t *e1 = encodeur_creer(img1);
    TEST_ASSERT_NOT_NULL(e1);

    unsigned char message[] = "Qu'est ce qu'il y a de mieux qu'un cours d'ELE216 :) ";
    int statut = encodeur_cacher(e1, message, strlen((char*)message));
    
    TEST_ASSERT_EQUAL_INT(0, statut);

    encodeur_detruire(e1);
    image_detruire(img1);
}

// Message vide : le header est quand même écrit (taille = 0)
void test_message_vide(void) {
    unsigned char message_vide[] = "";
    int statut = encodeur_cacher(e, message_vide, strlen((char*)message_vide));
    
    TEST_ASSERT_EQUAL_INT(1, statut);
    TEST_ASSERT_EQUAL_INT(80, encodeur_bits_utilises(e));
}


// Le nombre de bits utilisés correspond à (taille de l’en-tête + taille du payload) × 8
void test_nombre_bits_utilises(void) {
    unsigned char message[] = "ELE216";
    int taille = strlen((char*)message);
    
    encodeur_cacher(e, message, taille);

    int bits_attendus = (10 + taille) * 8;
    TEST_ASSERT_EQUAL_INT(bits_attendus, encodeur_bits_utilises(e));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_encoder_message_connu);
    RUN_TEST(test_encoder_puis_decoder);
    RUN_TEST(test_message_trop_gros);
    RUN_TEST(test_message_vide);
    RUN_TEST(test_nombre_bits_utilises);

    return UNITY_END();
}
