#include "imagebmp.h"
#include "vecteur_gen.h"
#include "imageppm.h"
#include "pixel.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//struct pour section vert (voir image)
//En-tête BMP (14 octets)
struct bmp_header{
    char magic[2];              /* Signature "BM" */
    int32_t taille;             /* Taille totale du fichier en octets */
    int32_t reserve;            /* Champ réservé (doit être 0) */
    int32_t pixel_start;        /* Offset vers le début des données pixel */
};

//struct pour section bleu
//En-tête DIB (40 octets, format BITMAPINFOHEADER)
struct dib_header{
    int32_t taille_en_tete;     /* Taille de cet en-tête (40) */
    int32_t largeur;            /* Largeur de l'image en pixels */
    int32_t hauteur;            /* Hauteur de l'image en pixels */
    int16_t nb_plans;           /* Nombre de plans de couleur (doit être 1) */
    int16_t bpp;                /* Bits par pixel (32 pour ARGB) */
    int32_t compression;        /* Type de compression (0 = non compressé) */
    int32_t taille_image;       /* Taille des données image en octets */
    int32_t res_horizontale;    /* Résolution horizontale en pixels/mètre */
    int32_t res_verticale;      /* Résolution verticale en pixels/mètre */
    int32_t taille_palette;     /* Nombre de couleurs dans la palette */
    int32_t importantes;        /* Nombre de couleurs importantes */
};

struct image_bmp{
    struct bmp_header en_tete_bmp;  /* En-tête BMP */
    struct dib_header en_tete_dib;  /* En-tête DIB */
    vecteur_t *pixels;              /* Tableau dynamique des pixels */
};

/**
 * Crée une nouvelle image BMP vide.
 * L'image est initialisée avec les en-têtes BMP et DIB standards
 * pour un format 32 bits par pixel (ARGB).
 * @return Pointeur vers l'image créée ou NULL en cas d'échec d'allocation mémoire.
 */
image_bmp_t *image_bmp_creer(){
    image_bmp_t *nouveau = malloc(sizeof(image_bmp_t)); //62 octets total
    if(!nouveau){return NULL;}

    nouveau->pixels = NULL;
    nouveau->en_tete_bmp.magic[0] = 'B';
    nouveau->en_tete_bmp.magic[1] = 'M';
    nouveau->en_tete_bmp.reserve = 0;
    nouveau->en_tete_bmp.pixel_start = 56;

    nouveau->en_tete_dib.taille_en_tete = 40;
    nouveau->en_tete_dib.largeur = 0;
    nouveau->en_tete_dib.hauteur = 0;
    nouveau->en_tete_dib.nb_plans = 1;
    nouveau->en_tete_dib.bpp = 32;
    nouveau->en_tete_dib.compression = 0;
    nouveau->en_tete_dib.taille_image = 0;
    nouveau->en_tete_dib.res_horizontale = 0x2e23;
    nouveau->en_tete_dib.res_verticale = 0x2e23;
    nouveau->en_tete_dib.taille_palette = 0;
    nouveau->en_tete_dib.importantes = 0;

    return nouveau;
}

image_bmp_t *changer_valeur_bmp(image_bmp_t *img,int32_t hauteur,int32_t largeur,int32_t taille_f,vecteur_t *pixels){
    assert(img);
    img->en_tete_bmp.taille = taille_f;
    img->en_tete_dib.hauteur = hauteur;
    img->en_tete_dib.largeur = largeur;
    img->en_tete_dib.taille_image = 0; 
    img->pixels = pixels;
    return img;
}

/**
 * Libère la mémoire allouée pour une image BMP.
 * @param img Pointeur vers l'image à libérer. Si le pointeur est NULL,
 *            la fonction n'a pas d'effet (voir la définition de free()).
 */
void image_bmp_liberer(image_bmp_t *img) {
    if (img == NULL) { return; }
    vecteur_liberer(img->pixels);
    free(img);
}

int32_t image_bmp_largeur(image_bmp_t *img) {
    assert(img != NULL);
    return img->en_tete_dib.largeur;
}

int32_t image_bmp_hauteur(image_bmp_t *img) {
    assert(img != NULL);
    return img->en_tete_dib.hauteur;
}

vecteur_t *image_bmp_pixels(image_bmp_t *img) {
    assert(img != NULL);
    return img->pixels;
}

/**
 * Crée une image BMP à partir d'une image PPM source.
 * Cette fonction réalise une copie superficielle du tableau de pixels
 * de la source. Si la source ou son contenu est détruite, cette référence
 * ne sera plus valide. Pour plus de robustesse, une copie profonde
 * serait nécessaire.
 * 
 * @param source Pointeur non-nul vers l'image PPM source.
 * @return Pointeur vers l'image BMP créée ou NULL en cas d'échec d'allocation mémoire.
 */
image_bmp_t *image_bmp_de_image_ppm(image_ppm_t *source) {
    assert(source != NULL);

    image_bmp_t *image = image_bmp_creer();
    if (image == NULL) { return NULL; }

    image->en_tete_dib.hauteur = image_ppm_hauteur(source);
    image->en_tete_dib.largeur = image_ppm_largeur(source);

    // ATTENTION : copie de surface du pointeur des pixels de la source.
    // Si la source ou son contenu est détruite, cette référence ne sera
    // plus valide.
    // Si plus de robustesse est nécessaire, il faut faire une copie profonde.
    // C'est-à-dire un nouveau tableau qui contient de nouvelles données, pas
    // juste une copie des pointeurs.
    image->pixels = image_ppm_pixels(source);

    // Calcul de la taille du fichier qui contiendrait le BMP en octets.
    image->en_tete_bmp.taille = sizeof(image->en_tete_bmp) +
                                sizeof(image->en_tete_dib) +
                               vecteur_taille(image->pixels) * sizeof(int32_t);

    return image;
}

/**
 * Convertit un pixel RGB en format ARGB 32 bits compacté (packed).
 * Le canal alpha est fixé à 0x00, les composants RGB sont placés
 * dans l'ordre ARGB (bits 31-24: A, 23-16: R, 15-8: G, 7-0: B).
 * 
 * @param source Pixel au format RGB à convertir.
 * @return Valeur 32 bits représentant le pixel au format ARGB.
 */
static int32_t rgb2packed_argb(pixel_t source) {
    return  0x00 << 24  | source.r << 16 | source.g << 8 | source.b;
}

static pixel_t *packed_argb2rgb(int32_t source) {
    pixel_t *pixel = malloc(sizeof(pixel_t));
    pixel->r = (source>>16) & 0xFF;
    pixel->g = (source>>8) & 0xFF;
    pixel->b = source & 0xFF;
    return pixel;
}

/**
 * Sauvegarde une image BMP dans un fichier.
 * Le fichier est créé au format BMP standard avec :
 * - En-tête BMP (14 octets)
 * - En-tête DIB (40 octets) 
 * - Padding d'alignement (2 octets)
 * - Données des pixels au format ARGB 32 bits
 * Les lignes sont stockées de bas en haut selon la convention BMP.
 * 
 * @param img Pointeur non-nul vers l'image BMP.
 * @param destination Nom du fichier de destination.
 */
void image_bmp_sauvegarde_fichier(image_bmp_t* img,const char *destination) {
    assert(img != NULL);
    
    FILE *dest = fopen(destination, "wb");
    if (dest == NULL) { return; }

    int resultat = fwrite(&img->en_tete_bmp.magic, sizeof(char), 2, dest);
    if (resultat != 2) { return; }

    resultat = fwrite(&img->en_tete_bmp.taille, sizeof(int32_t), 3, dest);
    if (resultat != 3) { return; }

    resultat = fwrite(&img->en_tete_dib, sizeof(struct dib_header), 1, dest);
    if (resultat != 1) { return; }

    // padding avant le pixel array pour aligner sur une adresse 32bit
    int16_t pad = 0xAABB;
    resultat = fwrite(&pad, sizeof(int16_t), 1, dest);
    if (resultat != 1) { return; }

    // le format BMP stocke les lignes de bas en haut
    for (int i = img->en_tete_dib.hauteur - 1; i >= 0; i--) {
        for (int j = 0; j < img->en_tete_dib.largeur; j++) {
            // on obtient un pointeur sur un pixel du tableau
            pixel_t *pixel = vecteur_obtenir(img->pixels, i * img->en_tete_dib.largeur + j);
            
            // on doit déréférencer le pointeur pour obtenir le pixel_t
            int32_t packed = rgb2packed_argb(*pixel);

            fwrite(&packed, sizeof(int32_t), 1, dest);
        }
    }
    fclose(dest);
}

//fct devoir 1:

image_bmp_t *image_bmp_du_fichier_bmp(const char *nom_fichier){
    image_bmp_t *img = image_bmp_creer();
    if(!img){return NULL;}

    FILE *source = fopen(nom_fichier, "rb");
    if(!source){
        image_bmp_liberer(img);
        return NULL;
    }

    //est ce qu'il faut lire chaque groupe d'octets ou comme ca ca marche?
    //lecture en-tête bmp:
    if (fread(img->en_tete_bmp.magic, sizeof(char), 2, source) != 2) {
        image_bmp_liberer(img);
        return NULL;
    }

    if (memcmp(img->en_tete_bmp.magic, "BM", 2) != 0) {
        image_bmp_liberer(img);
        return NULL;
    }

    if (fread(&img->en_tete_bmp.taille, sizeof(int32_t), 1, source) != 1) {
        image_bmp_liberer(img);
        return NULL;
    }

    if (fread(&img->en_tete_bmp.reserve, sizeof(int32_t), 1, source) != 1) {
        image_bmp_liberer(img);
        return NULL;
    }

    if (fread(&img->en_tete_bmp.pixel_start, sizeof(int32_t), 1, source) != 1) {
        image_bmp_liberer(img);
        return NULL;
    }

    //lecture en-tête dib:
    if (fread(&img->en_tete_dib, sizeof(struct dib_header), 1, source) != 1) {
        image_bmp_liberer(img);
        return NULL;
    }
    int32_t largeur = img->en_tete_dib.largeur;
    int32_t hauteur = img->en_tete_dib.hauteur;
    
    img->pixels = vecteur_creer(largeur*hauteur);

    fseek(source, img->en_tete_bmp.pixel_start, SEEK_SET);

    for(int32_t idx = 0;idx<largeur*hauteur;idx++){
        vecteur_ajouter(img->pixels,NULL);
    }

    //tab temporaire de pixe
    
    // le format BMP stocke les lignes de bas en haut
    for (int32_t i = hauteur - 1; i >= 0; i--) {
        for (int32_t j = 0; j < largeur; j++) {
            int32_t packed;
            if (fread(&packed, sizeof(int32_t), 1, source) != 1) {
                fclose(source);
                image_bmp_liberer(img);
                return NULL;
            }

            pixel_t* pix = packed_argb2rgb(packed);
            if (!pix) {
                image_bmp_liberer(img);
                return NULL;
            }
            /*
            pixel_t *temp = malloc(sizeof(pixel_t));
            if (!temp) {
                image_bmp_liberer(img);
                free(temp);
                return NULL;
            }
            //crochet pour respecter l'ordre d'ajout de bas en haut??
            temp->b = packed & 0xFF;
            temp->g = (packed>>8) & 0xFF;
            temp->r = (packed>>16) & 0xFF;
            vecteur_modifier(img->pixels,i * largeur + j,temp);
            */
            vecteur_modifier(img->pixels,i * largeur + j,pix);
            
        }
    }
    
    fclose(source);
    return img;
}

bool image_bmp_est_valide(image_bmp_t *img){
    assert(img);
    //vecteur_taille(img->pixels)
    return img->en_tete_dib.hauteur !=0 && img->en_tete_dib.largeur !=0 && 
    vecteur_taille(img->pixels) == img->en_tete_dib.hauteur *img->en_tete_dib.largeur;
}

