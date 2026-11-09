#include "unity.h"
#include "app.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void setUp(void){}

void tearDown(void)
{
    printf("\n");
    remove("image_encodee.bmp");
    remove("message_decode.txt");
}

/*
 * ============================================================================
 * EXIGENCE 1 : Traitement de chaque commande (via fonctions du module D)
 * ============================================================================
 */

void test_app_encoder_succes(void)
{
    TEST_ASSERT_TRUE(app_encoder("image.bmp", "message.txt", "image_encodee.bmp"));
}

void test_app_verifier_succes(void)
{
    app_encoder("image.bmp", "message.txt", "image_encodee.bmp");
    
    TEST_ASSERT_TRUE(app_verifier("image_encodee.bmp"));
}

void test_app_decoder_succes(void)
{
    app_encoder("image.bmp", "message.txt", "image_encodee.bmp");
    
    TEST_ASSERT_TRUE(app_decoder("image_encodee.bmp", "message_decode.txt"));
}

void test_app_capacite_execution(void)
{
    app_capacite("image.bmp");
    TEST_ASSERT_TRUE(1);
}

void test_app_comparer_execution(void)
{
    app_encoder("image.bmp", "message.txt", "image_encodee.bmp");
    
    app_comparer("image.bmp", "image_encodee.bmp");
    TEST_ASSERT_TRUE(1);
}

/*
 * ============================================================================
 * EXIGENCE 2 : Vérification de la cohérence des données décodées
 * ============================================================================
 */

void test_coherence_contenu_decode(void)
{
    app_encoder("image.bmp", "message.txt", "image_encodee.bmp");
    app_decoder("image_encodee.bmp", "message_decode.txt");

    FILE *original = fopen("message.txt", "rb");
    FILE *decode = fopen("message_decode.txt", "rb");

    TEST_ASSERT_NOT_NULL(original);
    TEST_ASSERT_NOT_NULL(decode);

    int c1;
    int c2;
    do {
        c1 = fgetc(original);
        c2 = fgetc(decode);
        TEST_ASSERT_EQUAL_INT(c1, c2);
    } while (c1 != EOF && c2 != EOF);

    fclose(original);
    fclose(decode);
}

/*
 * ============================================================================
 * EXIGENCE 3 : Fichiers inexistants (Messages d'erreur appropriés par code de retour 0)
 * ============================================================================
 */

void test_erreur_image_inexistante(void)
{
    TEST_ASSERT_FALSE(app_encoder("introuvable.bmp", "message.txt", "sortie.bmp"));
}

void test_erreur_message_inexistant(void)
{
    TEST_ASSERT_FALSE(app_encoder("image.bmp", "introuvable.txt", "sortie.bmp"));
}

void test_erreur_decode_image_normale(void)
{
    TEST_ASSERT_FALSE(app_decoder("image.bmp", "sortie_vide.txt"));
}


int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_app_encoder_succes);
    RUN_TEST(test_app_verifier_succes);
    RUN_TEST(test_app_decoder_succes);
    RUN_TEST(test_app_capacite_execution);
    RUN_TEST(test_app_comparer_execution);
    
    RUN_TEST(test_coherence_contenu_decode);

    RUN_TEST(test_erreur_image_inexistante);
    RUN_TEST(test_erreur_message_inexistant);
    RUN_TEST(test_erreur_decode_image_normale);

    return UNITY_END();
}
