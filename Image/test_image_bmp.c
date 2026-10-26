#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "image_bmp.h"
#include "unity.h"


void setUp(void){
}

void tearDown(void){
}


int fichiers_identiques(const char *chemin1, const char *chemin2) {
    FILE *f1 = fopen(chemin1, "rb");
    FILE *f2 = fopen(chemin2, "rb");
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 0;
    }
    fseek(f1, 0, SEEK_END);
    fseek(f2, 0, SEEK_END);
    long taille1 = ftell(f1);
    long taille2 = ftell(f2);
    rewind(f1);
    rewind(f2);

    if (taille1 != taille2) {
        fclose(f1);
        fclose(f2);
        return 0;
    }

    int identique = 1;
    int c1, c2;
    while ((c1 = fgetc(f1)) != EOF && (c2 = fgetc(f2)) != EOF) {
        if (c1 != c2) { identique = 0; break; }
    }

    fclose(f1);
    fclose(f2);
    return identique;
}
//Charger puis sauvegarder une image : le fichier produit est identique à l’original
void test_charger_sauvegarder_fichier_identique(void) {
    const char *original = "resultat1.bmp";
    const char *copie = "result1_test.bmp";

    image_t *img = image_charger(original);
    TEST_ASSERT_NOT_NULL(img);

    image_sauvegarder(img, copie);
    image_detruire(img);

    TEST_ASSERT_TRUE_MESSAGE(
        fichiers_identiques(original, copie),
        "Le fichier sauvegarde differe de l'original octet par octet"
    );
}

// Modifier une composante puis la relire : valeur correcte
void test_changement_composante(void){
    const char *original = "resultat1.bmp";
    image_t *img = image_charger(original);
    unsigned char avant = image_obtenir_composante(img,1,1,0);
    image_set_composante(img,1,1,0,1);
    unsigned char apres = image_obtenir_composante(img,1,1,0);
    TEST_ASSERT_NOT_NULL(img);
    TEST_ASSERT_NOT_EQUAL(avant,apres);
    image_detruire(img);
}

//set_lsb ne modifie que le bit de poids faible (les 7 bits supérieurs sont préservés)
void test_bits_superieurs_conserves(void){
    const char *original = "resultat1.bmp";
    image_t *img = image_charger(original);
    TEST_ASSERT_NOT_NULL(img);

    int index_lin = 0; // index linéaire utilisé par image_set_lsb
    int index_pix = index_lin / 3;
    int comp = index_lin % 3;
    int hauteur = image_hauteur(img);
    int largeur = image_largeur(img);

    int y = hauteur - 1 - (index_pix / largeur);
    int x = index_pix % largeur;

    unsigned char avant = image_obtenir_composante(img, x, y, comp);
    image_set_lsb(img, index_lin, 1);
    unsigned char apres = image_obtenir_composante(img, x, y, comp);

    TEST_ASSERT_EQUAL_HEX8(avant & 0xFE, apres & 0xFE);
    TEST_ASSERT_EQUAL(1,image_obtenir_lsb(img,index_lin));

    image_detruire(img);
}

//Capacité = largeur × hauteur × 3
void test_bonne_capacite(void){
    const char *original = "resultat1.bmp";
    image_t *img = image_charger(original);
    TEST_ASSERT_NOT_NULL(img);

    int capacite = image_hauteur(img)*image_largeur(img)*3;
    TEST_ASSERT_EQUAL(capacite,image_capacite_bits(img));
    image_detruire(img);
}

//Copie profonde : modifier la copie ne modifie pas l’original
void test_copie_profonde(void){
    const char *original = "resultat1.bmp";
    image_t *img_original = image_charger(original);
    TEST_ASSERT_NOT_NULL(img_original);

    image_t *img_copie = image_copier(img_original);
    TEST_ASSERT_NOT_NULL(img_copie);
    TEST_ASSERT_EQUAL(0,image_comparer(img_original,img_copie));

    image_detruire(img_original);
    image_detruire(img_copie);
}

//Comparer une image avec elle-même : 0 différences
void comparaison_avec_elle_meme(void){
    const char *original = "resultat1.bmp";
    image_t *img_a = image_charger(original);
    TEST_ASSERT_NOT_NULL(img_a);

    image_t *img_b = image_charger(original);
    TEST_ASSERT_NOT_NULL(img_b);
    
    TEST_ASSERT_EQUAL(0,image_comparer(img_a,img_b));

    image_detruire(img_a);
    image_detruire(img_b);
}

//creation d'une image noire
void test_image_vide(void){
    image_t *img = image_creer(1,1);
    TEST_ASSERT_NOT_NULL(img);

    TEST_ASSERT_EQUAL_HEX8(0x00,image_obtenir_composante(img,0,0,0));
    TEST_ASSERT_EQUAL_HEX8(0x00,image_obtenir_composante(img,0,0,1));
    TEST_ASSERT_EQUAL_HEX8(0x00,image_obtenir_composante(img,0,0,2));

    image_detruire(img);
}

int main(void){

    printf("** Tests Manuel **\n");

    printf("** Fin Tests Manuel **\n\n");

    
    UNITY_BEGIN();

    RUN_TEST(test_charger_sauvegarder_fichier_identique);
    RUN_TEST(test_changement_composante);
    RUN_TEST(test_bits_superieurs_conserves);
    RUN_TEST(test_bonne_capacite);
    RUN_TEST(test_copie_profonde);
    RUN_TEST(comparaison_avec_elle_meme);
    RUN_TEST(test_image_vide);

    return UNITY_END();
}
