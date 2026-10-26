#include "imageppm.h"
#include "imagebmp.h"
#include "vecteur_gen.h"
#include "pixel.h"

#include <stdbit.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

struct image_ppm {
    char      magic[2];     /* Identifiant du format PPM (P3) */
    int32_t   largeur;      /* Largeur de l'image en pixels */
    int32_t   hauteur;      /* Hauteur de l'image en pixels */
    int32_t   valeur_max;   /* Valeur maximale des composants (255) */
    vecteur_t *pixels;       /* Tableau dynamique des pixels */
};

image_ppm_t *image_ppm_creer(){
    image_ppm_t *nouveau = malloc(sizeof(image_ppm_t));   //image_ppm_t = 22 octets
    if(!nouveau){return NULL;}

    nouveau->magic[0] = 'P';
    nouveau->magic[1] = '3';
    nouveau->largeur = 0;
    nouveau->hauteur = 0;
    nouveau->valeur_max = 255;
    nouveau->pixels = vecteur_creer(256);
    if(!nouveau->pixels){
        free(nouveau);
        return NULL;
    }
    return nouveau;
}

void image_ppm_liberer(image_ppm_t *img){
    if(!img){return;}
    vecteur_liberer(img->pixels);
    free(img);
}

/**
 * Obtient la largeur de l'image PPM en pixels.
 * @param img Pointeur non-nul vers l'image.
 * @return La largeur de l'image en pixels.
 */
int32_t image_ppm_largeur(image_ppm_t *img) {
    assert(img != NULL);
    return img->largeur;
}

/**
 * Obtient la hauteur de l'image PPM en pixels.
 * @param img Pointeur non-nul vers l'image.
 * @return La hauteur de l'image en pixels.
 */
int32_t image_ppm_hauteur(image_ppm_t *img) {
    assert(img != NULL);
    return img->hauteur;
}

/**
 * Obtient le tableau dynamique contenant les pixels de l'image.
 * @param img Pointeur non-nul vers l'image.
 * @return Pointeur vers le tableau dynamique des pixels.
 */
vecteur_t *image_ppm_pixels(image_ppm_t *img) {
    assert(img != NULL);
    return img->pixels;
}

// methode usine
// pour le devoir: changer source pour dest et changer fopen pour fwrite
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
//note: fgets change de ligne (\n) automatiquement (le 1er fgets lit la 1ere ligne, le 2eme la 2eme ligne, etc.)
image_ppm_t *image_ppm_du_fichier_ppm(char *nom_fichier){
    image_ppm_t *img = image_ppm_creer();
    if(!img){return NULL;}

    FILE *source = fopen(nom_fichier, "rt");
    if(!source){
        image_ppm_liberer(img);
        return NULL;
    }

    char tampon[128];

    //lecture marqueur PPM
    fgets(tampon, 127, source);
    if (strcmp("P3\n", tampon)!= 0){
        image_ppm_liberer(img);
        return NULL;
    }

    //lecture largeur, hauteur
    fgets(tampon, 127,source);
    char *largeur_s = strtok(tampon, " ");
    if(largeur_s != NULL){
        img->largeur = atoi(largeur_s);
    } else{
        image_ppm_liberer(img);
        return NULL;
    }

    char *hauteur_s = strtok(NULL, " \n");
    if(hauteur_s != NULL){
        img->hauteur = atoi(hauteur_s);
    } else{
        image_ppm_liberer(img);
        return NULL;
    }

    // lecture valeur max composant, seul 255 est accepter
    fgets(tampon, 127, source);
    if (strcmp("255\n", tampon)!= 0){
        image_ppm_liberer(img);
        return NULL;
    }

    // lecture des valeurs de pixels jusqu'à la fin du fichier
    //tant qu'on a des lignes de pixels:
    while(fgets(tampon,127,source) != NULL){
        pixel_t *pixel = malloc(sizeof(pixel_t));
        if(!pixel){
            image_ppm_liberer(img);
            return NULL;
        }
        char *r_s = strtok(tampon, " ");
        if(r_s){pixel->r = atoi(r_s);}

        char *g_s = strtok(NULL, " ");
        if(g_s){pixel->g = atoi(g_s);}

        char *b_s = strtok(NULL, " \n");
        if(b_s){pixel->b = atoi(b_s);}

        vecteur_ajouter(img->pixels, pixel);
    }
    fclose(source);
    return img;
}

/**
 * Vérifie si l'image PPM est valide.
 * Une image est considérée comme valide si elle a des dimensions non-nulles
 * et contient le nombre correct de pixels (largeur × hauteur).
 * @param img Pointeur non-nul vers l'image.
 * @return true si l'image est valide, false sinon.
 */
bool image_ppm_est_valide(image_ppm_t *img){
    assert(img);
    //vecteur_taille(img->pixels)
    return img->hauteur !=0 && img->largeur !=0 && vecteur_taille(img->pixels) == img->hauteur *img->largeur;
}

//fcts devoir 1:
image_ppm_t *image_ppm_de_image_bmp(image_bmp_t *source) {
    assert(source != NULL);

    image_ppm_t *image = image_ppm_creer();
    if (image == NULL) { return NULL; }

    image->hauteur = image_bmp_hauteur(source);
    image->largeur = image_bmp_largeur(source);

    // ATTENTION : copie de surface du pointeur des pixels de la source.
    // Si la source ou son contenu est détruite, cette référence ne sera
    // plus valide.
    // Si plus de robustesse est nécessaire, il faut faire une copie profonde.
    // C'est-à-dire un nouveau tableau qui contient de nouvelles données, pas
    // juste une copie des pointeurs.
    image->pixels = image_bmp_pixels(source);

    return image;
}


void image_ppm_sauvegarde_fichier(image_ppm_t* img, char *destination) {
    assert(img != NULL);
    
    FILE *dest = fopen(destination, "wt");
    if (dest == NULL) { return; }

    fprintf(dest, "%c%c\n", img->magic[0], img->magic[1]);
    fprintf(dest, "%d %d\n", img->largeur, img->hauteur);
    fprintf(dest, "%d\n", img->valeur_max);

    for (int i = 0; i < vecteur_taille(img->pixels); i++) {
        pixel_t *pixel = vecteur_obtenir(img->pixels, i);
        fprintf(dest, "%d %d %d\n", pixel->r, pixel->g, pixel->b);
    }
    fclose(dest);
}