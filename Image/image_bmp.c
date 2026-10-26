#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "vecteur_gen.h"
#include "image_bmp.h"
#include "imagebmp.h"
#include "pixel.h"

//image_t encapsulant un buffer de pixels, la largeur, la hauteur et le
//nombre de composantes par pixel.
struct image {
    vecteur_t *pixels;
    int largeur;
    int hauteur;
    int composants;     //3 composantes: 0 = R, 1 = G, 2 = B
};

image_t *image_charger(const char *chemin){
    assert(chemin);

    // AJOUT : Vérification de sécurité pour éviter le crash interne de image_bmp_du_fichier_bmp
    FILE *fichier_test = fopen(chemin, "rb");
    if (fichier_test == NULL) {
        return NULL; // Le fichier n'existe pas, on quitte proprement immédiatement
    }
    fclose(fichier_test);

    image_t *img = malloc(sizeof(image_t));
    if(!img){return NULL;}
    //on charge en premier le fichier sous forme d'image BMP
    img->pixels = NULL;
    image_bmp_t *img_bmp = image_bmp_du_fichier_bmp(chemin);
    if(!img_bmp){free(img); return NULL;}

    //on fait la conversion image_bmp -> image_t
    img->pixels = image_bmp_pixels(img_bmp);
    img->hauteur = image_bmp_hauteur(img_bmp);
    img->largeur = image_bmp_largeur(img_bmp);
    img->composants = 3;
    free(img_bmp); 
    return img;
}

image_t *image_creer(int largeur, int hauteur){
    if(largeur<=0 || hauteur<=0){return NULL;}
    image_t *img = malloc(sizeof(image_t));
    if(!img){return NULL;}
    //initialisation du vecteur de pixel
    img->pixels = vecteur_creer(largeur * hauteur); 
    if (!img->pixels) {free(img); return NULL;}
    //on initialise + place les pixels dans le vecteur
    for (int i = hauteur - 1; i >= 0; i--) {
        for (int j = 0; j < largeur; j++) {
            pixel_t *pixel = malloc(sizeof(pixel_t));
            if(!pixel){free(img); return NULL;}
            pixel->r = 0x00;
            pixel->g = 0x00;
            pixel->b = 0x00;
            vecteur_ajouter(img->pixels,pixel);
        }
    }
    img->hauteur = hauteur;
    img->largeur = largeur;
    img->composants = 3;
    return img;
}

image_t *image_copier(const image_t *src){
    assert(src);
    //copie de l'image source
    image_t *img = malloc(sizeof(image_t));
    if(!img){return NULL;}

    img->hauteur = src->hauteur;
    img->largeur = src->largeur;
    img->composants = src->composants;
    //copie profonde: on crée un nouveau vecteur et on le remplie
    //avec les pixels de l'image source (on initilaise aussi des nouveaux pixels)
    img->pixels = vecteur_creer(img->hauteur*img->largeur);
    if(!img->pixels){free(img); return NULL;}

    //parcours du vecteur de pixels:
    for (int i = 0; i < (img->hauteur*img->largeur); i++) {
        pixel_t *pix_src = vecteur_obtenir(src->pixels, i);

        pixel_t *pix_copie = malloc(sizeof(pixel_t));
        if(!pix_copie){free(img); return NULL;}
        
        pix_copie->r = pix_src->r;
        pix_copie->g = pix_src->g;
        pix_copie->b = pix_src->b;
        
        vecteur_ajouter(img->pixels,pix_copie);
        
    }
    return img;
}

void image_sauvegarder(const image_t *img, const char *chemin){
    assert(img);
    assert(chemin);
    image_bmp_t *bmp = image_bmp_creer();
    //taille du vecteur de pixel:
    int32_t taille_pixel_array = vecteur_taille(img->pixels) * sizeof(int32_t);
    //taille fichier = taille en-tete BMP (14) + taille en-tete DIB (40) + buffer (2) + taille vecteur de pixel 
    int32_t taille_fichier = 56 + taille_pixel_array; 
    //on fait la conversion image_t -> image_bmp pour pouvoir utiliser
    //facilement la fct de sauvegarde de fichier
    bmp = changer_valeur_bmp(bmp,img->hauteur,img->largeur,taille_fichier,img->pixels);
    image_bmp_sauvegarde_fichier(bmp,chemin);
    free(bmp);
}

void image_detruire(image_t *img){
    if (img == NULL) { return; }
    if (img->pixels != NULL) {
        vecteur_liberer(img->pixels);
    }
    free(img);
}


int image_largeur(const image_t *img){
    assert(img);
    return img->largeur;
}

int image_hauteur(const image_t *img){
    assert(img);
    return img->hauteur;
}

unsigned char image_obtenir_composante(const image_t *img, int x, int y, int comp){
    assert(y < img->hauteur && x < img->largeur);
    assert(comp >= 0 && comp < 3);
    assert(img);
    //calcul pour obtenir l'index du vecteur de la position (x,y)
    int index = (img->hauteur - 1 -y)* img->largeur + x;
    pixel_t *pixel = vecteur_obtenir(img->pixels,index);
    if(comp == 0){return pixel->r;}
    else if(comp == 1){return pixel->g;}
    else if(comp == 2){return pixel->b;}
    return 0;
}

void image_set_composante(image_t *img, int x, int y, int comp, unsigned char val){
    assert(y < img->hauteur && x < img->largeur);
    assert(comp >= 0 && comp <= 3);
    assert(img);
    int index = (img->hauteur - 1 -y)* img->largeur + x;
    pixel_t *pixel = vecteur_obtenir(img->pixels,index);
    if(comp == 0){pixel->r = val;}
    else if(comp == 1){pixel->g = val;}
    else if(comp == 2){pixel->b = val;}
    vecteur_modifier(img->pixels,index,pixel);
}

int image_obtenir_lsb(const image_t *img, int index){
    assert(index >= 0 && index < img->largeur * img->hauteur * 3);
    assert(img);
    //index_pix = index du vecteur 
    int index_pix = index / 3;
    int composante = index % 3;
    pixel_t *pixel = vecteur_obtenir(img->pixels,index_pix);
    if(composante == 0){return (pixel->r & 1);}
    else if(composante == 1){return (pixel->g & 1);}
    else if(composante == 2){return (pixel->b & 1);}
    return -1;
}

void image_set_lsb(image_t *img, int index, int bit){
    assert(index >= 0 && index < img->largeur * img->hauteur * 3);
    assert(bit >= 0 && bit <=1);
    assert(img);
    int index_pix = index / 3;
    int composante = index % 3;
    pixel_t *pixel = vecteur_obtenir(img->pixels,index_pix);
    if(composante == 0){pixel->r = (pixel->r & 0xFE) | bit;}
    else if(composante == 1){pixel->g = (pixel->g & 0xFE) | bit;}
    else if(composante == 2){pixel->b = (pixel->b & 0xFE) | bit;}
    vecteur_modifier(img->pixels,index_pix,pixel);
}

int image_capacite_bits(const image_t *img){
    assert(img);
    return img->hauteur * img->largeur * 3;
}

int image_comparer(const image_t *a, const image_t *b){
    assert(a);
    assert(b);
    int diff = 0;
    //on parcours chaque coordonnées (x,y):
    for (int y = 0; y < a->hauteur; y++) {
        for (int x = 0; x < a->largeur; x++) {
            for(int c = 0;c < 3; c++){
                if(image_obtenir_composante(a,x,y,c) != image_obtenir_composante(b,x,y,c)){
                    diff+=1;
                }
            }
        }
    }
    return diff;
};
